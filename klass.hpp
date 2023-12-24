#pragma once

#include "cpool.hpp"

#include <vector>

namespace CppDuke
{
class KlassFile
{
private:
  std::shared_ptr<ConstantPool::CodeAttribute> _entryPoint;
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> _pool;
  std::vector<ConstantPool::CommonRef> _fields, _methods;
  std::vector<ConstantPool::CommonAttribute> _attributes;

public:
  explicit KlassFile(std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool,
                     std::vector<ConstantPool::CommonRef> fields,
                     std::vector<ConstantPool::CommonRef> methods,
                     std::vector<ConstantPool::CommonAttribute> attributes,
                     int entryPointIndex);

  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> Pool() const;
  std::vector<ConstantPool::CommonRef> Fields() const;
  std::vector<ConstantPool::CommonRef> Methods() const;

  std::shared_ptr<ConstantPool::CodeAttribute> GetEntryPoint() const;
};
}