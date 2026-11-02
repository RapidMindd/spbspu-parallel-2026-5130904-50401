#include "worker.hpp"
#include <chrono>
#include <string>
#include <thread>
#include <sys/mman.h>
#include "../common/area.hpp"
#include "shared_memory.hpp"

namespace tarasenko
{
  void runWorker(unsigned long seed, const char* memory_name, WorkerState* state)
  {
    size_t next = 0;
    while (!state->stopping || next < state->submitted)
    {
      if (next == state->submitted)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      const std::string calculation_name = std::string(memory_name) + '_' + std::to_string(next);
      Calculation* const calculation
          = static_cast< Calculation* >(openSharedMemory(calculation_name.c_str(), sizeof(Calculation)));
      calculation->state = CalculationState::running;
      Ellipse* const ellipses = static_cast< Ellipse* >(
          openSharedMemory(calculation->set_memory_name, calculation->count * sizeof(Ellipse)));
      const auto start = std::chrono::steady_clock::now();
      const auto result = getArea(ellipses, calculation->count, calculation->threads, calculation->tries, seed);
      const auto finish = std::chrono::steady_clock::now();
      calculation->area = result.first;
      calculation->milliseconds = std::chrono::duration< double, std::milli >(finish - start).count();
      munmap(ellipses, calculation->count * sizeof(Ellipse));
      calculation->state = CalculationState::done;
      munmap(calculation, sizeof(Calculation));
      ++next;
    }
  }
}
