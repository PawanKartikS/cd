#include "klass.hpp"

CppDuke::Klass::Klass(
  const std::string &name,
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool,
  std::vector<ConstantPool::CommonRef> fields,
  std::vector<ConstantPool::CommonRef> methods,
  std::vector<ConstantPool::CommonAttribute> attributes,
  int entryPointIndex = -1)
  : _name(name),
    _entryPoint(nullptr),
    _pool(pool),
    _fields(fields),
    _methods(methods),
    _attributes(attributes) // PSVM
{
  // Store a ref to PSVM if exists.
  if (entryPointIndex < 0)
  {
    return;
  }

  for (const ConstantPool::CommonAttribute &attribute: _methods[entryPointIndex].GetChildAttributes())
  {
    _entryPoint = attribute.GetCodeAttribute();
    break;
  }
}

std::vector<std::shared_ptr<CppDuke::ConstantPool::PoolEntry>>
CppDuke::Klass::Pool() const
{
  return _pool;
}

std::vector<CppDuke::ConstantPool::CommonRef>
CppDuke::Klass::Fields() const
{
  return _fields;
}

std::vector<CppDuke::ConstantPool::CommonRef>
CppDuke::Klass::Methods() const
{
  return _methods;
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::Klass::GetEntryPoint() const
{
  return _entryPoint;
}

std::shared_ptr<CppDuke::ConstantPool::GenericEntry> CppDuke::Klass::ResolveLowHigh(const int idx) const
{
  auto nameTpe = std::dynamic_pointer_cast<ConstantPool::GenericEntry>(_pool[idx - 1])->High();
  return std::dynamic_pointer_cast<ConstantPool::GenericEntry>(_pool[nameTpe - 1]);
}

std::string CppDuke::Klass::Ldc(const int idx) const
{
  std::shared_ptr<ConstantPool::GenericEntry> genEntry = std::dynamic_pointer_cast<ConstantPool::GenericEntry>(
      _pool[idx - 1]);
  return std::dynamic_pointer_cast<ConstantPool::Utf8>(_pool[genEntry->Low() - 1])->Data();
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::Klass::Invoke(const uint16_t nameIdx, const uint16_t descIdx, int &argCount)
{
  auto itr = std::find_if(std::begin(_methods),
                          std::end(_methods),
                          [&](const ConstantPool::CommonRef &cref) -> bool
                          {
                            return cref.NameIndex() == nameIdx && cref.DescIndex() == descIdx;
                          });

  argCount = 0;
  if (itr != std::end(_methods))
  {
    for (const CppDuke::ConstantPool::CommonAttribute &attr: itr->GetChildAttributes())
    {
      if (attr.Name() == "Code")
      {
        const std::string kDesc = std::dynamic_pointer_cast<ConstantPool::Utf8>(_pool[descIdx - 1])->Data();

        char *p = const_cast<char *>(kDesc.c_str());
        while (*p != ')')
        {
          switch (*p++)
          {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
              argCount++;
            default:
              // TODO: expand support
              break;
          }
        }

        return attr.GetCodeAttribute();
      }
    }
  }

  throw std::invalid_argument("Could not look up method");
}