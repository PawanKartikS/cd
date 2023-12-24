#pragma once

#include <stack>
#include <any>

#include "klass.hpp"

namespace CppDuke::VirtualMachine
{
typedef enum
{
  NOP = 0x0,
  ACONST_NULL,
  ICONST_M1,
  ICONST_0,
  ICONST_1,
  ICONST_2,
  ICONST_3,
  ICONST_4,
  ICONST_5,
  LCONST_0,
  LCONST_1,
  FCONST_0,
  FCONST_1,
  FCONST_2,
  DCONST_0,
  DCONST_1,
  BIPUSH,
  SIPUSH,
  LDC,
  ILOAD = 0x15,
  ALOAD = 0x19,
  ILOAD_0,
  ILOAD_1,
  ILOAD_2,
  ILOAD_3,
  LLOAD_0,
  LLOAD_1,
  LLOAD_2,
  LLOAD_3,
  FLOAD_0,
  FLOAD_1,
  FLOAD_2,
  FLOAD_3,
  DLOAD_0,
  DLOAD_1,
  DLOAD_2,
  DLOAD_3,
  ALOAD_0,
  ALOAD_1,
  ALOAD_2,
  ALOAD_3,
  IALOAD = 0x2e,
  LALOAD,
  FALOAD,
  DALOAD,
  BALOAD = 0x33,
  CALOAD,
  SALOAD,
  ISTORE = 0x36,
  ASTORE = 0x3a,
  ISTORE_0,
  ISTORE_1,
  ISTORE_2,
  ISTORE_3,
  LSTORE_0,
  LSTORE_1,
  LSTORE_2,
  LSTORE_3,
  FSTORE_0,
  FSTORE_1,
  FSTORE_2,
  FSTORE_3,
  DSTORE_0,
  DSTORE_1,
  DSTORE_2,
  DSTORE_3,
  ASTORE_0,
  ASTORE_1,
  ASTORE_2,
  ASTORE_3,
  IASTORE,
  LASTORE,
  FASTORE,
  DASTORE,
  BASTORE = 0x54,
  CASTORE,
  SASTORE,
  POP = 0x57,
  POP2,
  DUP,
  IADD = 0x60,
  LADD,
  FADD,
  DADD,
  ISUB,
  LSUB,
  FSUB,
  DSUB,
  IMUL,
  LMUL,
  FMUL,
  DMUL,
  IDIV,
  LDIV,
  FDIV,
  DDIV,
  ISHL = 0x78,
  ISHR = 0x7a,
  IINC = 0x84,
  I2C = 0x92,
  IFEQ = 0x99,
  IFNEQ,
  IFLT,
  IFGE,
  IFGT,
  IFLE,
  IF_ICMPEQ = 0x9f,
  IF_ICMPNE,
  IF_ICMPLT,
  IF_ICMPGE,
  IF_ICMPGT,
  IF_ICMPLE,
  GOTO = 0xa7,
  IRETURN = 0xac,
  RETURN = 0xb1,
  PUTSTATIC = 0xb3,
  PUTFIELD = 0xb5,
  INVOKESPECIAL = 0xb7,
  INVOKESTATIC,
  NEW = 0xbb,
  NEWARRAY,
  ARRAYLENGTH = 0xbe
} Opcode;


class Frame
{
private:
  std::stack<std::any> _stack;
  std::vector<std::any> _locals;

public:
  explicit Frame(uint16_t locals);
  [[nodiscard]]
  std::any At(int pos) const;
  void Set(int pos, const std::any val);
  std::any Top() const;
  std::any Pop();
  void Push(const std::any &item);
  std::size_t Size() const;
};

class Interpreter
{
private:
  KlassFile _klassFile;
  std::stack<Frame> _frames;

  bool _Cmp(const uint8_t &kOpcode);
  static std::any _Math(uint8_t opcode, const std::any &v1, const std::any &v2);

  void _ExecOpcode(const std::vector<uint8_t> &kByteCode, int &i);
  void _ExecMethod(const std::vector<uint8_t> &byteCode, uint16_t bufferSize);

public:
  explicit Interpreter(const KlassFile &klassFile);
  void Run();
};
}