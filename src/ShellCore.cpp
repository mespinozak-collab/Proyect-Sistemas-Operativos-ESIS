#include "ShellCore.h"

// Implementación de Helper: ejecutar comando con fallback a /bin/<cmd>
void ejecutar_comando_exec(const vector<string>& args_in) {
    if (args_in.empty()) _exit(EXIT_FAILURE);

    // holders y cargs preparan el vector de argumentos C-style (char**) necesario para exec.
    vector<string> holders = args_in;
    
    if (holders[0] == "fecha") {
        holders[0] = "date";
    }
    
    vector<char*> cargs;
    for (auto &s : holders) cargs.push_back(const_cast<char*>(s.c_str()));
    cargs.push_back(nullptr);

    execvp(cargs[0], cargs.data());

    // Si execvp falla, se intenta con /bin/
    if (errno == ENOENT && cargs[0][0] != '/') {
        string path = string("/bin/") + holders[0];
        vector<string> holders2 = holders;
        holders2[0] = path;
        vector<char*> cargs2;
        for (auto &s : holders2) cargs2.push_back(const_cast<char*>(s.c_str()));
        cargs2.push_back(nullptr);
        execv(cargs2[0], cargs2.data());
    }
    
    // Si se llega aqui exec falló definitivamente
    perror("Error en exec");
    _exit(EXIT_FAILURE);
}

// Implementación de run_command para built-in parallel
void* run_command(void* arg) {
    // unique_ptr asegura que el comando (asignado con new) se libera al salir de la función.
    unique_ptr<string> cmd_str(static_cast<string*>(arg));
    istringstream iss(*cmd_str);
    vector<string> tokens;
    string word;
    while (iss >> word) tokens.push_back(word);
    if (tokens.empty()) return nullptr;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error al crear proceso paralelo");
        return nullptr;
    }
    if (pid == 0) {
        ejecutar_comando_exec(tokens);
    } else {
        waitpid(pid, nullptr, 0); // Espera al comando hijo dentro del hilo
    }
    return nullptr;
}

// Implementación de aplicar_redirecciones en el proceso hijo
void aplicar_redirecciones(const vector<string>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") { // Redirección de salida (Truncar/Crear)
            if (i + 1 < args.size()) {
                int fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror(("Abriendo " + args[i + 1]).c_str()); _exit(EXIT_FAILURE); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        } else if (args[i] == ">>") { // Redirección de salida (Append)
            if (i + 1 < args.size()) {
                int fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) { perror(("Abriendo " + args[i + 1]).c_str()); _exit(EXIT_FAILURE); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        } else if (args[i] == "<") { // Redirección de entrada
            if (i + 1 < args.size()) {
                int fd = open(args[i + 1].c_str(), O_RDONLY);
                if (fd < 0) { perror(("Abriendo " + args[i + 1]).c_str()); _exit(EXIT_FAILURE); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
        }
    }
}

// Built-ins

// Implementación de cd
void builtin_cd(vector<string>& args, string& current_dir) {
    string path = (args.size() < 2) ? (getenv("HOME") ? getenv("HOME") : "") : args[1];

    if (path.empty()) {
        cout << "Error: No se pudo obtener HOME\n";
        return;
    }

    // Expande '~' al inicio
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            if (path.size() == 1) path = home;
            else if (path[1] == '/') path = string(home) + path.substr(1);
        }
    }

    error_code ec;
    fs::current_path(path, ec);
    if (ec) cout << "Error: " << ec.message() << "\n";
    else current_dir = fs::current_path().string();
}

// Implementación de pwd
void builtin_pwd(const string& current_dir) {
    cout << current_dir << "\n";
}

// Implementación de ls
void builtin_ls(const vector<string>& args) {
    if (args.size() == 1) {
        // ls simple: lista directorios internamente
        for (const auto& entry : fs::directory_iterator(fs::current_path()))
            cout << entry.path().filename().string() << "\n";
    } else {
        // Si hay argumentos (ej. ls -l), lo ejecutamos como comando externo
        vector<string> mutable_args = args;
        execute_external_command(mutable_args);
    }
}

// Implementación de echo (Maneja redirecciones sin fork)
void builtin_echo(const vector<string>& args) {
    bool redir_out = false, redir_append = false;
    string out_file;

    // 1. Detectar redirecciones
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") {
            redir_out = true;
            if (i + 1 < args.size()) out_file = args[i + 1];
        } else if (args[i] == ">>") {
            redir_append = true;
            if (i + 1 < args.size()) out_file = args[i + 1];
        }
    }

    // 2. Construir mensaje
    string msg;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == ">" || args[i] == ">>" || args[i] == "<") break; 
        msg += args[i];
        if (i + 1 < args.size() && args[i + 1] != ">" && args[i + 1] != ">>" && args[i + 1] != "<") msg += " ";
    }

    // 3. Escribir con redirección temporal con dup/dup2
    if (redir_out || redir_append) {
        int flags = redir_out ? (O_WRONLY | O_CREAT | O_TRUNC) : (O_WRONLY | O_CREAT | O_APPEND);
        int fd = open(out_file.c_str(), flags, 0644);
        if (fd < 0) {
            perror(("Abriendo " + out_file).c_str());
        } else {
            int stdout_backup = dup(STDOUT_FILENO); // Guardar descriptor original
            dup2(fd, STDOUT_FILENO); // Redirigir stdout al archivo
            close(fd);

            cout << msg << "\n";
            fflush(stdout);

            dup2(stdout_backup, STDOUT_FILENO); // Restaurar stdout
            close(stdout_backup);
        }
    } else {
        cout << msg << "\n";
    }
}

// Implementación de clear
void builtin_clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Implementación de history
void builtin_history() {
    for (size_t i = 0; i < history.size(); ++i)
        cout << i + 1 << ": " << history[i] << "\n";
}

// Implementación de alias
void builtin_alias(vector<string>& args) {
    if (args.size() == 1) {
        for (auto &p : aliases) cout << p.first << " = " << p.second << "\n";
    } else if (args.size() >= 4 && args[2] == "=") {
        string rest;
        for (size_t i = 3; i < args.size(); ++i) {
            if (i > 3) rest += " ";
            rest += args[i];
        }
        aliases[args[1]] = rest;
    } else {
        cout << "Uso: alias nombre = comando\n";
    }
}

// Implementación de parallel
void builtin_parallel(vector<string>& args) {
    vector<pthread_t> hilos;
    bool create_failed = false;
    for (size_t i = 1; i < args.size(); ++i) {
        pthread_t tid;
        string* comando = new string(args[i]);
        int rc = pthread_create(&tid, nullptr, run_command, comando);
        if (rc != 0) {
            cerr << "No se pudo crear hilo para: " << args[i] << " (error " << rc << ")\n";
            delete comando; // evitar leak
            create_failed = true;
            break;
        }
        hilos.push_back(tid);
    }
    // Esperar a que todos los hilos terminen, sincronización con pthread_join
    for (auto& h : hilos) pthread_join(h, nullptr);
    if (create_failed) cout << "[Paralelo abortado por error de creación de hilos]\n";
}

// Ejecución de Comandos Externos

// Lógica de Ejecución para pipes simples (cmd1 | cmd2)
void execute_pipe_line(vector<string>& args) {
cout << "DEBUG: Iniciando pipe con: ";
    for (const auto& arg : args) cout << arg << " ";
    cout << endl;
    cout.flush();

    vector<string> leftArgs, rightArgs;
    auto it = find(args.begin(), args.end(), "|");
    if (it == args.end() || it == args.begin() || it == args.end() - 1) {
        cerr << "Error: sintaxis incorrecta en el pipe.\n";
        return;
    }

    leftArgs.assign(args.begin(), it);
    rightArgs.assign(it + 1, args.end());

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid1 == 0) {
        // Hijo izquierdo - escritor
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Forzar unbuffered output
        setvbuf(stdout, NULL, _IONBF, 0);

        vector<char*> argv;
        for (auto& arg : leftArgs) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        
        execvp(argv[0], argv.data());
        perror("execvp izquierda");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, nullptr, 0);
        return;
    }

    if (pid2 == 0) {
        // Hijo derecho - lector
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Forzar unbuffered output
        setvbuf(stdout, NULL, _IONBF, 0);

        vector<char*> argv;
        for (auto& arg : rightArgs) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        
        execvp(argv[0], argv.data());
        perror("execvp derecha");
        exit(EXIT_FAILURE);
    }

    // Proceso padre
    close(pipefd[0]);
    close(pipefd[1]);

    // Espera bloqueante
    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    cout << "DEBUG: Pipe terminado" << endl;
    cout.flush();
}

// Lógica de ejecución para comandos externos sin pipe
void execute_external_command(vector<string>& args) {
    vector<string> args_original = args;

    // 1. Detección de background (&) y redirecciones
    bool segundo_plano = false;
    if (!args.empty() && args.back() == "&") {
        segundo_plano = true;
        args.pop_back();
        if (args.empty()) return;
    }    
    // Eliminación de tokens de redirección del vector args para que exec no falle.
    vector<string> clean_args;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">" || args[i] == ">>" || args[i] == "<") {
            i++; // saltar el nombre del archivo
        } else {
            clean_args.push_back(args[i]);
        }
    }

    // 2. Fork
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Hijo: Aplica las redirecciones ya que el padre no las puede aplicar
        aplicar_redirecciones(args); 

        // Ejecutar
        ejecutar_comando_exec(clean_args);
    } else {
        // PADRE: En espera o continúa
        if (!segundo_plano) waitpid(pid, nullptr, 0);
        else cout << "[Proceso en segundo plano: PID " << pid << "]\n";
    }
}
