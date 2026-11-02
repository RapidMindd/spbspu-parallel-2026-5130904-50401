#include "shared_memory.hpp"
#include <stdexcept>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tarasenko
{
  void* allocateSharedMemory(const char* name, size_t bytes)
  {
    const int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
      throw std::runtime_error("Cannot create shared memory");
    }
    if (ftruncate(fd, bytes) == -1)
    {
      close(fd);
      shm_unlink(name);
      throw std::runtime_error("Cannot resize shared memory");
    }
    void* const memory = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (memory == MAP_FAILED)
    {
      shm_unlink(name);
      throw std::runtime_error("Cannot map shared memory");
    }
    return memory;
  }

  void* openSharedMemory(const char* name, size_t bytes)
  {
    const int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
      throw std::runtime_error("Cannot open shared memory");
    }
    void* const memory = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (memory == MAP_FAILED)
    {
      throw std::runtime_error("Cannot map shared memory");
    }
    return memory;
  }

  void freeSharedMemory(const char* name, void* memory, size_t bytes)
  {
    munmap(memory, bytes);
    shm_unlink(name);
  }

  Ellipse* allocateSharedCircles(const char* name, size_t count)
  {
    return static_cast< Ellipse* >(allocateSharedMemory(name, count * sizeof(Ellipse)));
  }

  void freeSharedCircles(const char* name, Ellipse* ellipses, size_t count)
  {
    freeSharedMemory(name, ellipses, count * sizeof(Ellipse));
  }
}
