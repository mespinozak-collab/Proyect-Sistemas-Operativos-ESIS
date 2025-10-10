#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    std::string cmd;

    while (true) {
        std::cout << "myshell> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") break;
        if (cmd.empty()) continue;

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

    return 0;
}
