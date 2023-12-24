#include "klass.hpp"

CppDuke::KlassFile::KlassFile(
    std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>> pool,
    std::vector<CppDuke::ConstantPool::CommonRef> fields,
    std::vector<CppDuke::ConstantPool::CommonRef> methods,
    std::vector<CppDuke::ConstantPool::CommonAttribute> attributes,
    int entryPointIndex = -1)
    : _pool(pool),
      _fields(fields),
      _methods(methods),
      _attributes(attributes),
      _entryPoint(nullptr) // PSVM
{
  // Store a ref to PSVM if exists.
  if (entryPointIndex < 0)
  {
    return;
  }

  for (const ConstantPool::CommonRef &method: _methods)
  {
    if (method.DescIndex() == entryPointIndex)
    {
      for (const ConstantPool::CommonAttribute &attribute: method.GetChildAttributes())
      {
        _entryPoint = attribute.GetCodeAttribute();
        break;
      }
    }
  }
}

std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>>
CppDuke::KlassFile::Pool() const
{
  return _pool;
}

std::vector<CppDuke::ConstantPool::CommonRef>
CppDuke::KlassFile::Fields() const
{
  return _fields;
}

std::vector<CppDuke::ConstantPool::CommonRef>
CppDuke::KlassFile::Methods() const
{
  return _methods;
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::KlassFile::GetEntryPoint() const
{
  return _entryPoint;
}

