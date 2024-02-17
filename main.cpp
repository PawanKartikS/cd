#include <iostream>

#include "klass.hpp"
#include "parser.hpp"
#include "vm.hpp"

using namespace CppDuke;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    std::cerr << "Missing arguments\n";
    return 1;
  }

  std::vector<Klass> klasses;
  for (int i = 2; i < argc; i++)
  {
    klasses.emplace_back(Parser{argv[i]}.Parse());
  }

  // Typical hack for now.
  // This is not how JVM finds class files.
  VirtualMachine::Interpreter(klasses, argv[1]).Run();

  return 0;
}
