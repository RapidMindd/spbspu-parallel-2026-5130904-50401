#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <stddef.h>
#include "../common/geometry.hpp"

namespace tarasenko
{
  void* allocateSharedMemory(const char* name, size_t bytes);
  void* openSharedMemory(const char* name, size_t bytes);
  void freeSharedMemory(const char* name, void* memory, size_t bytes);
  Ellipse* allocateSharedCircles(const char* name, size_t count);
  void freeSharedCircles(const char* name, Ellipse* ellipses, size_t count);
}

#endif
