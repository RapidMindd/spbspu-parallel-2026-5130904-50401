#ifndef WORKER_HPP
#define WORKER_HPP

#include <stddef.h>

namespace tarasenko
{
  constexpr size_t memory_name_size = 32;

  enum class CalculationState
  {
    ready,
    running,
    done
  };

  struct Calculation
  {
    char set_memory_name[memory_name_size];
    size_t count;
    long threads;
    long tries;
    CalculationState state;
    double area;
    double milliseconds;
  };

  struct WorkerState
  {
    size_t submitted;
    bool stopping;
  };

  void runWorker(unsigned long seed, const char* memory_name, WorkerState* state);
}

#endif
