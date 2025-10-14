#include "MemoryManager.h"
#include "Globals.h"  // Variables globales con los tipos correctos
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cerrno>

// Implementación del Reaper SIGCHLD, función en segundo plano
void sigchld_handler(int) {
    int saved_errno = errno; // Guardar el error original
    int status;
    pid_t pid;

    // Espera a todos los hijos terminados sin bloquear
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFSTOPPED(status)) {
            continue;
        }
    }

    errno = saved_errno; // Restaurar errno para no alterar otras funciones
}

// Implementación del handler de SIGINT (Ctrl+C)
void sigint_handler(int) {
    cout << "\n[Shell activa - Ctrl+C ignorado]\n";
}

// Implementación de Overrides de new/delete 
void* operator new(size_t size) {
    void* p = malloc(size);
    if (!p) throw bad_alloc();
    g_total_alloc.fetch_add(1, memory_order_relaxed);
    return p;
}

void operator delete(void* ptr) noexcept {
    if (ptr) {
        g_total_free.fetch_add(1, memory_order_relaxed);
        free(ptr);
    }
}

// Implementación del built-in meminfo
void builtin_meminfo() {
    cout << "Asignaciones: " << g_total_alloc.load() << "\n";
    cout << "Liberaciones: " << g_total_free.load() << "\n";
}
