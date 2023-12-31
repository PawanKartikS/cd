#include "parser.hpp"

#if defined __APPLE__
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#else
#include <endian.h>
#endif

#include <unordered_map>

using namespace CppDuke;

Parser::Parser(std::string klassFile) :
    _fd(fopen(klassFile.c_str(), "r"))
{}

Parser::~Parser()
{
  fclose(_fd);
}

template<typename T>
T Parser::_Read()
{
  static_assert(
      std::is_same<T, uint8_t>::value
      || std::is_same<T, uint16_t>::value
      || std::is_same<T, uint32_t>::value);
  const unsigned long dwSize = sizeof(T);

  T bits;
  fread(&bits, dwSize, /* nitems = */1, _fd);
  if (std::is_same<T, uint16_t>::value)
  {
    bits = be16toh(bits);
  } else if (std::is_same<T, uint32_t>::value)
  {
    bits = be32toh(bits);
  }

  return bits;
}

void Parser::_ParseHeaders()
{
  (void) _Read<uint32_t>(); // Skip 0xCAFEBABE
  (void) _Read<uint16_t>(); // Skip min version
  (void) _Read<uint16_t>(); // Skip max version
}

std::vector<std::shared_ptr<ConstantPool::PoolEntry>>
Parser::_ParseConstantPool()
{
  uint16_t tableSize = _Read<uint16_t>();
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool;
  pool.resize(tableSize);

  for (int i = 0; i < tableSize - 1; i++)
  {
    uint8_t tag = _Read<uint8_t>();
    if (tag < ConstantPool::UTF_8 || tag > 18)
    {
      throw std::invalid_argument("Invalid tag: " + std::to_string(tag));
    }

    switch (tag)
    {
      case ConstantPool::EntryType::UTF_8:
      {
        uint16_t sLen = _Read<uint16_t>();
        std::string s;
        s.resize(sLen);

        for (int j = 0; j < sLen; j++)
        {
          s[j] = _Read<uint8_t>();
        }

        pool[i] = std::make_shared<ConstantPool::Utf8>(s);
        break;
      }

      case ConstantPool::EntryType::CONST_DOUBLE:
      {
        uint16_t high = _Read<uint16_t>();
        uint16_t low = _Read<uint16_t>();

        // Use high and low to construct the double value.
        pool[i] = std::make_shared<ConstantPool::GenericEntry>(high, low);
        break;
      }

      case ConstantPool::EntryType::CLASS:
      {
        pool[i] = std::make_shared<ConstantPool::GenericEntry>(
            _Read<uint16_t>(), // Name index
            0 // Ignore, zero value
        );
        break;
      }

      case ConstantPool::EntryType::STRING:
      {
        pool[i] = std::make_shared<ConstantPool::GenericEntry>(_Read<uint16_t>(), // Index
                                                               0 // Ignore, zero value
        );
        break;
      }

      case ConstantPool::EntryType::FIELD_REF:
      case ConstantPool::EntryType::METHOD_REF:
      case ConstantPool::EntryType::INTERFACE_REF:
      case ConstantPool::EntryType::NAME_TYPE_REF:
      {
        pool[i] = std::make_shared<ConstantPool::GenericEntry>(
            _Read<uint16_t>(), // Class index
            _Read<uint16_t>() // Name type index
        );
        break;
      }
    } // parsing done
  } // loop

  return pool;
}

void Parser::_ParseMeta()
{
  for (int i = 0; i < 3; i++)
  {
    (void) _Read<uint16_t>(); // Skip f_access, this, and f_super
  }

  uint16_t infLen = _Read<uint16_t>();
  for (int i = 0; i < infLen; i++)
  {
    (void) _Read<uint16_t>();
  }
}

std::vector<ConstantPool::CommonRef> Parser::_ParseKlassFields(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  uint16_t length = _Read<uint16_t>();
  std::vector<ConstantPool::CommonRef> fields;
  for (int i = 0; i < length; i++)
  {
    fields.emplace_back(_ParseCommonFields(pool));
  }

  return fields;
}

std::vector<ConstantPool::CommonRef> Parser::_ParseMethods(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  uint16_t length = _Read<uint16_t>();
  std::vector<ConstantPool::CommonRef> methods;
  for (int i = 0; i < length; i++)
  {
    methods.emplace_back(_ParseCommonFields(pool));
  }

  return methods;
}

std::vector<ConstantPool::CommonAttribute> Parser::_ParseKlassAttributes(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  uint16_t length = _Read<uint16_t>();
  std::vector<ConstantPool::CommonAttribute> attributes;
  for (int i = 0; i < length; i++)
  {
    attributes.emplace_back(_ParseAttribute(pool));
  }

  return attributes;
}

ConstantPool::CommonAttribute Parser::_ParseAttribute(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  uint16_t name = _Read<uint16_t>();
  uint32_t length = _Read<uint32_t>();
  std::string s = std::dynamic_pointer_cast<ConstantPool::Utf8>(pool[name - 1])->Data();

  std::shared_ptr<ConstantPool::CodeAttribute> opt = nullptr;
  if (s == "Code")
  {
    opt = _ParseCodeAttribute(pool);
  } else
  {
    for (int i = 0; i < length; i++)
    {
      (void) _Read<uint8_t>();
    }
  }

  return ConstantPool::CommonAttribute{name, length, s, opt};
}

std::shared_ptr<ConstantPool::CodeAttribute> Parser::_ParseCodeAttribute(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  uint16_t stack = _Read<uint16_t>();
  uint16_t local = _Read<uint16_t>();
  uint32_t bcode = _Read<uint32_t>();

  std::vector<uint8_t> code(bcode);
  for (int i = 0; i < bcode; i++)
  {
    code[i] = _Read<uint8_t>();
  }

  uint16_t empty[_Read<uint16_t>() * 64];
  (void) fread(empty, sizeof(empty), 1, _fd);

  uint16_t ex = _Read<uint16_t>();
  for (int i = 0; i < ex; i++)
  {
    (void) _ParseAttribute(pool);
  }

  return std::make_shared<ConstantPool::CodeAttribute>(stack, local, code);
}

ConstantPool::CommonRef Parser::_ParseCommonFields(
    std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool)
{
  // Skip f_access
  (void) _Read<uint16_t>();

  uint16_t name = _Read<uint16_t>();
  uint16_t desc = _Read<uint16_t>();
  uint16_t length = _Read<uint16_t>();

  std::vector<ConstantPool::CommonAttribute> attributes;
  for (int j = 0; j < length; j++)
  {
    attributes.emplace_back(_ParseAttribute(pool));
  }

  return ConstantPool::CommonRef{0, // Ignore
                                 name,
                                 desc,
                                 length,
                                 attributes};
}

KlassFile Parser::Parse()
{
  _ParseHeaders();
  std::vector<std::shared_ptr<ConstantPool::PoolEntry>> pool = _ParseConstantPool();
  _ParseMeta();
  std::vector<ConstantPool::CommonRef> fields = _ParseKlassFields(pool);
  std::vector<ConstantPool::CommonRef> methods = _ParseMethods(pool);
  std::vector<ConstantPool::CommonAttribute> attributes = _ParseKlassAttributes(pool);

  // PSVM detection.
  // TODO: Improve it.
  int entryPointIndex = -1;
  for (int i = 0; i < pool.size(); i++)
  {
    std::shared_ptr<ConstantPool::Utf8> utf8 = std::dynamic_pointer_cast<ConstantPool::Utf8>(
        pool[i]);
    if (utf8 && utf8->Data() == "([Ljava/lang/String;)V")
    {
      entryPointIndex = i + 1;
    }
  }

  return KlassFile{pool, fields, methods, attributes, entryPointIndex};
}