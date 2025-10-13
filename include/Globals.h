#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <filesystem>
#include <csignal>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

// Variables globales extern, se definen en MemoryManager.cpp
extern atomic<size_t> g_total_alloc;
extern atomic<size_t> g_total_free;
extern vector<string> history;
extern unordered_map<string, string> aliases;

// Handlers de Señales
void sigchld_handler(int); // Para realizar procesos en segundo plano (&)
void sigint_handler(int);  // Ignora Ctrl+C en la shell principal
