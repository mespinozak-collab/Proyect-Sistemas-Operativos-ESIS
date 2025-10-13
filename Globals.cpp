#include "Globals.h"
#include <cstddef>

// Variables para la instrumentación de memoria
std::atomic<size_t> g_total_alloc{0};
std::atomic<size_t> g_total_free{0};

// Variables para la gestión de la historia y alias
std::vector<std::string> history;
std::unordered_map<std::string, std::string> aliases;
