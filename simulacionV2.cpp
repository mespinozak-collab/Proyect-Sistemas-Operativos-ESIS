#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    const char* GREEN = "\033[1;32m";
    const char* RESET = "\033[0m";
    char* input;

    while (true) {
        std::string prompt = std::string(GREEN) + "myshell> " + RESET;
        input = readline(prompt.c_str());
        if (!input) break; // Ctrl+D

        std::string cmd = input;
        free(input);

        if (cmd.empty()) continue;
        add_history(cmd.c_str());
        if (cmd == "exit") break;

        pid_t pid = fork();
        if (pid == 0) {
            execlp("bash", "bash", "-c", cmd.c_str(), NULL);
            perror("exec failed");
            return 1;
        } else if (pid > 0) {
            waitpid(pid, nullptr, 0);
        } else {
            perror("fork failed");
        }
    }
    std::cout << "\nBye!\n";
    return 0;
}
