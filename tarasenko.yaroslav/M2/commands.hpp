#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iosfwd>
#include "data.hpp"

namespace tarasenko
{
  void circle(Data& data, std::istream& in, std::ostream& out);
  void set(Data& data, std::istream& in, std::ostream& out);
  void show(Data& data, std::istream& in, std::ostream& out);
  void showSet(Data& data, std::istream& in, std::ostream& out);
  void frame(Data& data, std::istream& in, std::ostream& out);
  void frameSet(Data& data, std::istream& in, std::ostream& out);
  void spawn(Data& data, std::istream& in, std::ostream& out);
  void areaOn(Data& data, std::istream& in, std::ostream& out);
  void status(Data& data, std::istream& in, std::ostream& out);
  void wait(Data& data, std::istream& in, std::ostream& out);
}

#endif
