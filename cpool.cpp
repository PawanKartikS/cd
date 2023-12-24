#include "cpool.hpp"

CppDuke::ConstantPool::PoolEntry::~PoolEntry()
{};

CppDuke::ConstantPool::GenericEntry::GenericEntry(uint16_t v1,
                                                  uint16_t v2)
    : _v1(v1), // high bytes
      _v2(v2)  // low bytes
{}

CppDuke::ConstantPool::CodeAttribute::CodeAttribute(uint16_t stack,
                                                    uint16_t locals,
                                                    std::vector<uint8_t> code)
    : _stack(stack),   // Max stack length
      _locals(locals), // Max locals
      _code(code)      // Byte code
{
}

std::vector<uint8_t> CppDuke::ConstantPool::CodeAttribute::ByteCode() const
{
  return _code;
}

uint16_t CppDuke::ConstantPool::CodeAttribute::BufferSize() const
{
  return _locals;
}

CppDuke::ConstantPool::CommonAttribute::CommonAttribute(uint16_t nameIndex,
                                                        uint32_t length,
                                                        std::string name,
                                                        std::shared_ptr<CodeAttribute> opt)
    : _nameIndex(nameIndex), // Name index
      _length(length),       // Length of the attribute
      _name(name),           // Name of the attribute
      _opt(opt)              // Optional attribute
{
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::ConstantPool::CommonAttribute::GetCodeAttribute() const
{
  return _opt;
}

std::string CppDuke::ConstantPool::CommonAttribute::Name() const
{
  return _name;
}

CppDuke::ConstantPool::CommonRef::CommonRef(uint16_t fAccess,
                                            uint16_t nameIndex,
                                            uint16_t descIndex,
                                            uint16_t attributeLength,
                                            std::vector<CommonAttribute> chAttributes)
    : _fAccess(fAccess),
      _nameIndex(nameIndex),
      _descIndex(descIndex),
      _attributeLength(attributeLength),
      _chAttributes(chAttributes) // Child attributes
{
}

std::vector<CppDuke::ConstantPool::CommonAttribute>
CppDuke::ConstantPool::CommonRef::GetChildAttributes() const
{
  return _chAttributes;
}

uint16_t CppDuke::ConstantPool::CommonRef::DescIndex() const
{
  return _descIndex;
}

CppDuke::ConstantPool::Utf8::Utf8(const std::string &s) :
    _S(s) // Holds the string value.
{}

std::string CppDuke::ConstantPool::Utf8::Data() const
{
  return _S;
}
