#include <iostream>

#include "klass.hpp"
#include "parser.hpp"
#include "vm.hpp"

using namespace CppDuke;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Missing classfile argument\n";
    return 1;
  }

  Parser p(argv[1]);
  KlassFile klassName = p.Parse();

  // Typical hack for now.
  // This is not how JVM finds class files.
  VirtualMachine::Interpreter(klassName).Run();

  return 0;
}
