#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include "data.hpp"
#include "commands.hpp"

int main()
{
  tarasenko::Data data;
  using cmd_t = void (*)(tarasenko::Data &, std::istream &, std::ostream &);
  std::unordered_map< std::string, cmd_t > cmds;
  cmds["circle"] = tarasenko::circle;
  cmds["set"] = tarasenko::set;
  cmds["show"] = tarasenko::show;
  cmds["showset"] = tarasenko::showSet;
  cmds["frame"] = tarasenko::frame;
  cmds["frameset"] = tarasenko::frameSet;
  cmds["spawn"] = tarasenko::spawn;
  cmds["area-on"] = tarasenko::areaOn;
  cmds["status"] = tarasenko::status;
  cmds["wait"] = tarasenko::wait;

  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      if (!cmds.count(cmd))
      {
        std::cout << "<UNKNOWN COMMAND>\n";
        std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
        continue;
      }
      const cmd_t command = cmds.at(cmd);
      command(data, std::cin, std::cout);
    }
    catch (const std::exception &)
    {
      std::cout << "<INVALID COMMAND>\n";
      auto toignore = std::numeric_limits< std::streamsize >::max();
      std::cin.clear();
      std::cin.ignore(toignore, '\n');
    }
  }
}
