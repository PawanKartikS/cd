#pragma once

#include <vector>
#include <string>
#include <memory>

namespace CppDuke::ConstantPool
{
// These values are taken from Oracle's spec sheet.
typedef enum
{
  UTF_8 = 1,
  CONST_DOUBLE = 6,
  CLASS = 7,
  STRING = 8,
  FIELD_REF = 9,
  METHOD_REF = 10,
  INTERFACE_REF = 11,
  NAME_TYPE_REF = 12,
} EntryType;

class PoolEntry
{
public:
  virtual ~PoolEntry();
};

class CodeAttribute : PoolEntry
{
private:
  uint16_t _stack, _locals;
  std::vector<uint8_t> _code;

public:
  explicit CodeAttribute(uint16_t stack, uint16_t locals, std::vector<uint8_t> code);
  std::vector<uint8_t> ByteCode() const;
  uint16_t BufferSize() const;
};

class CommonAttribute : PoolEntry
{
private:
  uint16_t _nameIndex;
  uint32_t _length;

  std::string _name;
  std::shared_ptr<CodeAttribute> _opt;
public:
  explicit CommonAttribute(uint16_t nameIndex,
                           uint32_t length,
                           std::string name,
                           std::shared_ptr<CodeAttribute> code);
  std::shared_ptr<CodeAttribute> GetCodeAttribute() const;
  std::string Name() const;
};

class CommonRef : PoolEntry
{
private:
  uint16_t _fAccess, _nameIndex, _descIndex, _attributeLength;
  std::vector<CommonAttribute> _chAttributes;
public:
  explicit CommonRef(uint16_t fAccess,
                     uint16_t nameIndex,
                     uint16_t descIndex,
                     uint16_t attributeLength,
                     std::vector<CommonAttribute> chAttributes);
  std::vector<CommonAttribute> GetChildAttributes() const;
  uint16_t DescIndex() const;
};

class GenericEntry : public PoolEntry
{
private:
  uint16_t _v1, _v2;
public:
  explicit GenericEntry(uint16_t v1, uint16_t v2);
  uint16_t Low() const;
};

class Utf8 : public PoolEntry
{
private:
  std::string _S;
public:
  explicit Utf8(const std::string &s);

  std::string Data() const;
};

} // CppDuke::ConstantPool
