#include "ShellCore.h"
#include "MemoryManager.h" 
// Se incluye solo lo necesario para el main loop
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <cerrno>

using namespace std;
namespace fs = std::filesystem;

vector<string> tokenize_input(const string& input) {
    vector<string> tokens;
    string token;
    bool in_quotes = false;
    char quote_char = '\0';
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        
        if (in_quotes) {
            if (c == quote_char) {
                in_quotes = false;
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        } else {
            if (c == '"' || c == '\'') {
                in_quotes = true;
                quote_char = c;
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else if (c == '|' || c == '>' || c == '<') {
                // Operadores especiales
                if (!token.empty()) {
		    tokens.push_back(token);
		    token.clear();
	        }

	        if (c == '>' && i + 1 < input.length() && input[i + 1] == '>') {
		    tokens.push_back(">>");
		    i++; // saltar el segundo '>'
	        } else {
		    tokens.push_back(string(1, c));
	        }
            } else if (isspace(c)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
    }
    
    if (!token.empty()) {
        tokens.push_back(token);
    }
    
    return tokens;
}

int main() {
    // 1. Instalación de Signal Handlers (SIGCHLD y SIGINT)
    // SIGCHLD para la recolección asíncrona de procesos en segundo plano
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, nullptr) == -1) {
        perror("Error al configurar el SIGINT");
        return 1;
    }

    // SIGINT (Ctrl+C) para ignorar la señal en la shell principal
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, nullptr) == -1) {
        perror("Error al configurar el SIGINT");
        return 1;
    }

    string input;
    string current_dir = fs::current_path().string();

    cout << "Bienvenido al CMD Simulado en C++\n";
    cout << "Escribe 'ayuda' para ver comandos.\n\n";

    // 2. Bucle principal de la shell
    while (true) {
        char* line = readline((current_dir + " > ").c_str());
        if (!line) { 
            cout << "\n";
            break; // Ctrl+D (EOF)
        }

        input = line;
        free(line); 

        if (!input.empty()) {
            add_history(input.c_str()); 
        }
        if (input.empty()) continue;

        history.push_back(input); 

        // Tokenización
        vector<string> args = tokenize_input(input);
        if (args.empty()) continue;

        string cmd = args[0];

        // 3. Expansión de alias
        if (aliases.count(cmd)) {
            string full = aliases[cmd];
            istringstream iss2(full);
            vector<string> newtokens;
            string w;
            while (iss2 >> w) newtokens.push_back(w);
            for (size_t i = 1; i < args.size(); ++i) newtokens.push_back(args[i]);
            args = move(newtokens);
            cmd = args[0];
        }

        // 4. Decisión de ejecución
        if (cmd == "exit" || cmd == "salir") {
            break;
        } else if (cmd == "ayuda") {
            cout << "Comandos disponibles:\n";
            cout << "  cd, pwd, ls, echo, limpiar, salir\n";
            cout << "  >, <, >>, |, &, alias, historial, paralelo, meminfo\n";
        } else if (cmd == "pwd") {
            builtin_pwd(current_dir);
        } else if (cmd == "cd") {
            builtin_cd(args, current_dir);
        } else if (cmd == "historial") {
            builtin_history();
        } else if (cmd == "alias") {
            builtin_alias(args);
        } else if (cmd == "meminfo") {
            builtin_meminfo();
        } else if (cmd == "paralelo") {
            builtin_parallel(args);
        } else if (cmd == "limpiar") {
            builtin_clear();
        } else if (cmd == "echo") {
            builtin_echo(args);
        }
        // 5. Comandos Externos
        else {
            bool has_pipe = false;
            for (const auto& arg : args) {
                if (arg == "|") {
                    has_pipe = true;
                    break;
                }
            }

            if (has_pipe) {
                execute_pipe_line(args);
            } else {
                execute_external_command(args);
            }
        }
    }

    cout << "Saliendo del CMD...\n";
    return 0;
}
