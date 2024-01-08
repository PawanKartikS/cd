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

std::shared_ptr<CppDuke::ConstantPool::GenericEntry> CppDuke::KlassFile::ResolveLowHigh(const int idx) const
{
  auto nameTpe = std::dynamic_pointer_cast<ConstantPool::GenericEntry>(_pool[idx - 1])->High();
  return std::dynamic_pointer_cast<ConstantPool::GenericEntry>(_pool[nameTpe - 1]);
}

std::string CppDuke::KlassFile::Ldc(const int idx) const
{
  std::shared_ptr<ConstantPool::GenericEntry> genEntry = std::dynamic_pointer_cast<ConstantPool::GenericEntry>(
      _pool[idx - 1]);
  return std::dynamic_pointer_cast<ConstantPool::Utf8>(_pool[genEntry->Low() - 1])->Data();
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::KlassFile::Invoke(const uint16_t nameIdx, const uint16_t descIdx, int &argCount)
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