#pragma once
#include <bits/stdc++.h>
#include <filesystem>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Globals.h"  // Variables globales y handlers

using namespace std;
namespace fs = std::filesystem;

// Funciones auxiliares
void ejecutar_comando_exec(const vector<string>& args_in);
void* run_command(void* arg);
void aplicar_redirecciones(const vector<string>& args);

// Built-ins
void builtin_cd(vector<string>& args, string& current_dir);
void builtin_pwd(const string& current_dir);
void builtin_ls(const vector<string>& args);
void builtin_echo(const vector<string>& args);
void builtin_clear();
void builtin_history();
void builtin_alias(vector<string>& args);
void builtin_parallel(vector<string>& args);
void builtin_meminfo(); // Built-in de memoria

// Ejecución de comandos externos
void execute_pipe_line(vector<string>& args);
void execute_external_command(vector<string>& args);
