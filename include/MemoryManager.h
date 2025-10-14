#pragma once

#include "Globals.h"

// Override de operadores new/delete, medir uso de heap
void* operator new(size_t size);
void operator delete(void* ptr) noexcept;

// Built-in de memoria instrumentada
void builtin_meminfo();
