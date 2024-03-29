#pragma once

#include "cpool.hpp"

#include <vector>

namespace CppDuke
{
class Klass
{
  std::string _name;
  std::shared_ptr<ConstantPool::CodeAttribute> _entryPoint;
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> _pool;
  std::vector<ConstantPool::CommonRef> _fields, _methods;
  std::vector<ConstantPool::CommonAttribute> _attributes;

public:
  explicit Klass(const std::string &name,
                 std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool,
                 std::vector<ConstantPool::CommonRef> fields,
                 std::vector<ConstantPool::CommonRef> methods,
                 std::vector<ConstantPool::CommonAttribute> attributes);

  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> Pool() const;
  std::vector<ConstantPool::CommonRef> Fields() const;
  std::vector<ConstantPool::CommonRef> Methods() const;
  std::string Name() const;

  std::shared_ptr<ConstantPool::CodeAttribute> GetEntryPoint() const;
  std::shared_ptr<ConstantPool::GenericEntry> ResolveLowHigh(const int idx) const;

  // Eq to VM's opcodes.
  std::string Ldc(const int idx) const;
  std::shared_ptr<ConstantPool::CodeAttribute> Invoke(const uint16_t nameIdx,
                                                      const uint16_t descIdx,
                                                      int &argCount) const;
};
}
