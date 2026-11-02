#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>
#include "../common/geometry.hpp"
#include "shared_memory.hpp"
#include "worker.hpp"

namespace tarasenko
{
  struct CircleSet
  {
    std::string memory_name;
    Ellipse* ellipses;
    size_t count;
  };

  struct Worker
  {
    pid_t pid;
    std::string memory_name;
    WorkerState* state;
  };

  struct CalculationInfo
  {
    std::string process_name;
    std::string memory_name;
    Calculation* calculation;
  };

  struct Data
  {
    std::unordered_map< std::string, Ellipse > ellipses;
    std::unordered_map< std::string, CircleSet > sets;
    std::unordered_map< std::string, Worker > processes;
    std::unordered_map< std::string, CalculationInfo > calculations;

    ~Data()
    {
      for (const auto& item : processes)
      {
        item.second.state->stopping = true;
      }
      for (const auto& item : processes)
      {
        waitpid(item.second.pid, nullptr, 0);
      }
      for (const auto& item : calculations)
      {
        const CalculationInfo& info = item.second;
        freeSharedMemory(info.memory_name.c_str(), info.calculation, sizeof(Calculation));
      }
      for (const auto& item : processes)
      {
        const Worker& worker = item.second;
        freeSharedMemory(worker.memory_name.c_str(), worker.state, sizeof(WorkerState));
      }
      for (const auto& item : sets)
      {
        const CircleSet& set = item.second;
        freeSharedCircles(set.memory_name.c_str(), set.ellipses, set.count);
      }
    }
  };
}

#endif
