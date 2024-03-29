#pragma once

#include <string>
#include "cpool.hpp"
#include "klass.hpp"

namespace CppDuke
{
class Parser
{
  FILE* _fd;
  uint16_t _this;

  template<typename T>
  [[maybe_unused]] T _Read();

  void _ParseHeaders();

  [[nodiscard]]
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> _ParseConstantPool();

  void _ParseMeta();

  [[nodiscard]]
  std::vector<ConstantPool::CommonRef>
  _ParseKlassFields(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

  [[nodiscard]]
  std::vector<ConstantPool::CommonRef>
  _ParseMethods(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

  [[nodiscard]]
  std::vector<ConstantPool::CommonAttribute>
  _ParseKlassAttributes(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

  [[nodiscard]]
  ConstantPool::CommonAttribute
  _ParseAttribute(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

  [[nodiscard]]
  std::shared_ptr<ConstantPool::CodeAttribute>
  _ParseCodeAttribute(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

  [[nodiscard]]
  ConstantPool::CommonRef
  _ParseCommonFields(const std::vector<std::shared_ptr<ConstantPool::PoolEntry>> &pool);

public:
  explicit Parser(const std::string &klassFile);

  Klass Parse();

  ~Parser();
};
}
