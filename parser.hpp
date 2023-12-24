#pragma once

#include <string>
#include "cpool.hpp"
#include "klass.hpp"

namespace CppDuke
{
class Parser
{
private:
  FILE *_fd;

  template<typename T>
  [[maybe_unused]] T _Read();

  void _ParseHeaders();

  [[nodiscard]]
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> _ParseConstantPool();

  void _ParseMeta();

  [[nodiscard]]
  std::vector<ConstantPool::CommonRef>
  _ParseKlassFields(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

  [[nodiscard]]
  std::vector<CppDuke::ConstantPool::CommonRef>
  _ParseMethods(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

  [[nodiscard]]
  std::vector<ConstantPool::CommonAttribute>
  _ParseKlassAttributes(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

  [[nodiscard]]
  ConstantPool::CommonAttribute
  _ParseAttribute(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

  [[nodiscard]]
  std::shared_ptr<ConstantPool::CodeAttribute>
  _ParseCodeAttribute(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

  [[nodiscard]]
  ConstantPool::CommonRef
  _ParseCommonFields(std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool);

public:
  explicit Parser(std::string klassFile);
  CppDuke::KlassFile Parse();

  ~Parser();
};
}