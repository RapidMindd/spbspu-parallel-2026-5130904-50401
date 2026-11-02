#include "commands.hpp"
#include <chrono>
#include <cctype>
#include <cstdio>
#include <istream>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include "data.hpp"
#include "worker.hpp"

namespace tarasenko
{
  namespace
  {
    void checkWorker(const Worker& worker)
    {
      if (waitpid(worker.pid, nullptr, WNOHANG) != 0)
      {
        throw std::runtime_error("Worker broken");
      }
    }

    template< class T >
    void readValue(std::istream& in, T& value)
    {
      while (in.peek() != '\n' && std::isspace(in.peek()))
      {
        in.get();
      }
      if (in.peek() == '\n' || !(in >> value))
      {
        throw std::invalid_argument("Missing or invalid argument");
      }
    }

    std::string readName(std::istream& in)
    {
      std::string name;
      readValue(in, name);
      const char first = name.front();
      if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z')))
      {
        throw std::invalid_argument("Invalid name");
      }
      return name;
    }

    void checkEnd(std::istream& in)
    {
      while (in.peek() != '\n' && std::isspace(in.peek()))
      {
        in.get();
      }
      if (in.peek() != '\n' && in.peek() != EOF)
      {
        throw std::invalid_argument("Unexpected argument");
      }
    }

    void printStatus(const CalculationInfo& info, std::ostream& out)
    {
      const Calculation& calculation = *info.calculation;
      out << info.process_name << ' ';
      if (calculation.state == CalculationState::done)
      {
        const auto flags = out.flags();
        const auto precision = out.precision();
        out << std::fixed << std::setprecision(3) << calculation.milliseconds << ' ' << calculation.area << '\n';
        out.flags(flags);
        out.precision(precision);
      }
      else
      {
        out << "<IN PROGRESS>\n";
      }
    }
  }

  void circle(Data& data, std::istream& in, std::ostream&)
  {
    const std::string name = readName(in);
    long radius = 0;
    long x = 0;
    long y = 0;
    readValue(in, radius);
    readValue(in, x);
    readValue(in, y);
    checkEnd(in);
    if (radius <= 0 || data.ellipses.count(name))
    {
      throw std::invalid_argument("Invalid circle");
    }
    Ellipse shape{};
    shape.rx = radius;
    shape.ry = radius;
    shape.center.x = x;
    shape.center.y = y;
    data.ellipses.emplace(name, shape);
  }

  void show(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Ellipse& shape = data.ellipses.at(name);
    out << shape.rx << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
  }

  void set(Data& data, std::istream& in, std::ostream&)
  {
    const std::string name = readName(in);
    long count = 0;
    readValue(in, count);
    if (count <= 0 || data.sets.count(name))
    {
      throw std::invalid_argument("Invalid set");
    }
    const size_t set_size = count;
    const std::string memory_name = "/m2_" + std::to_string(data.sets.size());
    Ellipse* const ellipses = allocateSharedCircles(memory_name.c_str(), set_size);
    try
    {
      for (size_t i = 0; i < set_size; ++i)
      {
        const std::string ellipse_name = readName(in);
        ellipses[i] = data.ellipses.at(ellipse_name);
      }
      checkEnd(in);
      data.sets.emplace(name, CircleSet{memory_name, ellipses, set_size});
    }
    catch (...)
    {
      freeSharedCircles(memory_name.c_str(), ellipses, set_size);
      throw;
    }
  }

  void showSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    for (size_t i = 0; i < set.count; ++i)
    {
      const Ellipse& shape = set.ellipses[i];
      out << shape.rx << " (" << shape.center.x << ' ' << shape.center.y << ")\n";
    }
  }

  void frame(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const Ellipse& shape = data.ellipses.at(name);
    const Rectangle rectangle = findFrame(&shape, 1);
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") (" << rectangle.right_top.x << ' '
        << rectangle.right_top.y << ")\n";
  }

  void frameSet(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CircleSet& set = data.sets.at(name);
    const Rectangle rectangle = findFrame(set.ellipses, set.count);
    out << '(' << rectangle.left_bottom.x << ' ' << rectangle.left_bottom.y << ") (" << rectangle.right_top.x << ' '
        << rectangle.right_top.y << ")\n";
  }

  void spawn(Data& data, std::istream& in, std::ostream&)
  {
    std::string name;
    unsigned long seed = 0;
    in >> name >> seed;
    if (data.processes.count(name))
    {
      throw std::invalid_argument("Process already exists");
    }
    const std::string memory_name = "/worker_" + std::to_string(data.processes.size());
    WorkerState* const state
        = static_cast< WorkerState* >(allocateSharedMemory(memory_name.c_str(), sizeof(WorkerState)));
    *state = {0, false};
    const pid_t pid = fork();
    if (pid == -1)
    {
      freeSharedMemory(memory_name.c_str(), state, sizeof(WorkerState));
      throw std::runtime_error("Cannot create process");
    }
    if (pid == 0)
    {
      try
      {
        runWorker(seed, memory_name.c_str(), state);
      }
      catch (const std::exception&)
      {
        _exit(1);
      }
      _exit(0);
    }
    data.processes.emplace(name, Worker{pid, memory_name, state});
  }

  void areaOn(Data& data, std::istream& in, std::ostream&)
  {
    const std::string process_name = readName(in);
    const std::string calculation_name = readName(in);
    const std::string set_name = readName(in);
    long threads = 0;
    long tries = 0;
    readValue(in, threads);
    readValue(in, tries);
    checkEnd(in);
    if (threads < 0 || tries <= 0 || data.calculations.count(calculation_name))
    {
      throw std::invalid_argument("Invalid calculation");
    }
    const Worker& worker = data.processes.at(process_name);
    checkWorker(worker);
    const CircleSet& set = data.sets.at(set_name);
    const std::string memory_name = worker.memory_name + '_' + std::to_string(worker.state->submitted);
    Calculation* const calculation
        = static_cast< Calculation* >(allocateSharedMemory(memory_name.c_str(), sizeof(Calculation)));
    *calculation = {};
    std::strcpy(calculation->set_memory_name, set.memory_name.c_str());
    calculation->count = set.count;
    calculation->threads = threads == 0 ? 1 : threads;
    calculation->tries = tries;
    calculation->state = CalculationState::ready;
    try
    {
      data.calculations.emplace(calculation_name, CalculationInfo{process_name, memory_name, calculation});
    }
    catch (...)
    {
      freeSharedMemory(memory_name.c_str(), calculation, sizeof(Calculation));
      throw;
    }
    ++worker.state->submitted;
  }

  void status(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CalculationInfo& info = data.calculations.at(name);
    if (info.calculation->state != CalculationState::done)
    {
      checkWorker(data.processes.at(info.process_name));
    }
    printStatus(info, out);
  }

  void wait(Data& data, std::istream& in, std::ostream& out)
  {
    std::string name;
    in >> name;
    const CalculationInfo& info = data.calculations.at(name);
    while (info.calculation->state != CalculationState::done)
    {
      checkWorker(data.processes.at(info.process_name));
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printStatus(info, out);
  }
}
