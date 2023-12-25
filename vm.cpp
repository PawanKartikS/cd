#include "vm.hpp"

#include <iostream>

#define TWO_BYTE_CONSTRUCT(code, i) ((((signed char)(code[(i) + 1])) << 8) + code[(i) + 2])

CppDuke::VirtualMachine::Frame::Frame(const uint16_t locals)
{
  _locals.resize(locals + 1);
}

std::any CppDuke::VirtualMachine::Frame::At(const int pos) const
{
  return _locals[pos];
}

std::any CppDuke::VirtualMachine::Frame::Top() const
{
  return _stack.top();
}

std::any CppDuke::VirtualMachine::Frame::Pop()
{
  std::any top = Top();
  _stack.pop();

  return top;
}

void CppDuke::VirtualMachine::Frame::Push(const std::any &item)
{
  _stack.emplace(item);
}

void CppDuke::VirtualMachine::Frame::Set(const int pos, const std::any val)
{
  if (!val.has_value())
  {
    throw std::invalid_argument("Value is empty for position " + std::to_string(pos));
  }

  _locals[pos] = val;
}

std::size_t CppDuke::VirtualMachine::Frame::Size() const
{
  return _stack.size();
}

CppDuke::VirtualMachine::Interpreter::Interpreter(const KlassFile &klassFile)
    : _klassFile(klassFile) // The file that contains PSVM
{
  if (_klassFile.GetEntryPoint() == nullptr)
  {
    // Error out?
  }
}

bool CppDuke::VirtualMachine::Interpreter::_Cmp(const uint8_t &kOpcode)
{
  Frame &frame = _frames.top();
  int v1 = std::any_cast<int>(frame.Pop());
  int v2 = 0;

  if (kOpcode >= IF_ICMPEQ && kOpcode <= IF_ICMPLE)
  {
    v2 = std::any_cast<int>(frame.Pop());
  } else if (kOpcode >= IF_ICMPEQ && kOpcode <= IF_ICMPLE)
  {
    std::swap(v1, v2);
  }

  switch (kOpcode)
  {
    case IFEQ:
    case IF_ICMPEQ:
      return v1 == v2;
    case IFNEQ:
    case IF_ICMPNE:
      return v1 != v2;
    case IFLE:
    case IF_ICMPLE:
      return v1 <= v2;
    case IFLT:
    case IF_ICMPLT:
      return v1 < v2;
    case IFGE:
    case IF_ICMPGE:
      return v1 >= v2;
    case IFGT:
    case IF_ICMPGT:
      return v1 > v2;
    default:
      throw std::invalid_argument("Invalid opcode");
  }
}

std::any
CppDuke::VirtualMachine::Interpreter::_Math(const uint8_t opcode, const std::any &v1, const std::any &v2)
{
  switch (opcode)
  {
    case IADD:
      return std::any_cast<int>(v1) + std::any_cast<int>(v2);
    case LADD:
      return std::any_cast<long>(v1) + std::any_cast<long>(v2);
    case FADD:
      return std::any_cast<float>(v1) + std::any_cast<float>(v2);
    case DADD:
      return std::any_cast<double>(v1) + std::any_cast<double>(v2);

    case ISUB:
      return std::any_cast<int>(v1) - std::any_cast<int>(v2);
    case LSUB:
      return std::any_cast<long>(v1) - std::any_cast<long>(v2);
    case FSUB:
      return std::any_cast<float>(v1) - std::any_cast<float>(v2);
    case DSUB:
      return std::any_cast<double>(v1) - std::any_cast<double>(v2);

    case IMUL:
      return std::any_cast<int>(v1) * std::any_cast<int>(v2);
    case LMUL:
      return std::any_cast<long>(v1) * std::any_cast<long>(v2);
    case FMUL:
      return std::any_cast<float>(v1) * std::any_cast<float>(v2);
    case DMUL:
      return std::any_cast<double>(v1) * std::any_cast<double>(v2);

    case IDIV:
      return std::any_cast<int>(v1) / std::any_cast<int>(v2);
    case LDIV:
      return std::any_cast<long>(v1) / std::any_cast<long>(v2);
    case FDIV:
      return std::any_cast<float>(v1) / std::any_cast<float>(v2);
    case DDIV:
      return std::any_cast<double>(v1) / std::any_cast<double>(v2);
    default:
      throw std::invalid_argument("Cannot perform math op for opcode");
  }
}

void CppDuke::VirtualMachine::Interpreter::_ExecOpcode(const std::vector<uint8_t> &kByteCode,
                                                       int &i)
{
  const uint8_t kOpcode = kByteCode[i];
  char hex[32];
  snprintf(hex, sizeof(hex), "%x", kOpcode);
  std::cout << i << ": " << hex << "\n";

  Frame &frame = _frames.top();
  switch (kOpcode)
  {
    case NOP:
      break;

    case ACONST_NULL:
      frame.Push(nullptr);
      break;

    case ICONST_M1:
    case ICONST_0:
    case ICONST_1:
    case ICONST_2:
    case ICONST_3:
    case ICONST_4:
    case ICONST_5:
      frame.Push(static_cast<int>(kOpcode) - 3);
      break;

    case LCONST_0:
    case LCONST_1:
      frame.Push(static_cast<int>(kOpcode) - 9);
      break;

    case FCONST_0:
    case FCONST_1:
    case FCONST_2:
      frame.Push(static_cast<int>(kOpcode) - 11);
      break;

    case DCONST_0:
    case DCONST_1:
      frame.Push(static_cast<int>(kOpcode) - 14);
      break;

    case SIPUSH:
      frame.Push(TWO_BYTE_CONSTRUCT(kByteCode, i));
      i += 2;
      break;

    case BIPUSH:
      frame.Push(static_cast<int>(kByteCode[++i]));
      break;

    case ALOAD:
      frame.Push(frame.At(kByteCode[++i]));
      break;

    case ILOAD:
      frame.Push(std::any_cast<int>(frame.At(kByteCode[++i])));
      break;

    case ILOAD_0:
    case ILOAD_1:
    case ILOAD_2:
    case ILOAD_3:
      frame.Push(std::any_cast<int>(frame.At(kOpcode - ILOAD_0)));
      break;

    case LLOAD_0:
    case LLOAD_1:
    case LLOAD_2:
    case LLOAD_3:
      frame.Push(std::any_cast<long>(frame.At(kOpcode - LLOAD_0)));
      break;

    case FLOAD_0:
    case FLOAD_1:
    case FLOAD_2:
    case FLOAD_3:
      frame.Push(std::any_cast<float>(frame.At(kOpcode - FLOAD_0)));
      break;

    case DLOAD_0:
    case DLOAD_1:
    case DLOAD_2:
    case DLOAD_3:
      frame.Push(std::any_cast<double>(frame.At(kOpcode - DLOAD_0)));
      break;

    case ALOAD_0:
    case ALOAD_1:
    case ALOAD_2:
    case ALOAD_3:
      frame.Push(frame.At(kOpcode - ALOAD_0));
      break;

    case BALOAD:
    case CALOAD:
    case IALOAD:
    case LALOAD:
    case FALOAD:
    case DALOAD:
    {
      // Implement type check here.
      int pos = std::any_cast<int>(frame.Pop());
      std::shared_ptr<std::vector<std::any>> arr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(frame.Pop());
      frame.Push((*arr)[pos]);
      break;
    }

    case ASTORE:
      frame.Set(kByteCode[++i], frame.Pop());
      break;

    case ISTORE:
      frame.Set(kByteCode[++i], std::any_cast<int>(frame.Pop()));
      break;

    case ISTORE_0:
    case ISTORE_1:
    case ISTORE_2:
    case ISTORE_3:
      frame.Set(kOpcode - ISTORE_0, std::any_cast<int>(frame.Pop()));
      break;

    case LSTORE_0:
    case LSTORE_1:
    case LSTORE_2:
    case LSTORE_3:
      frame.Set(kOpcode - LSTORE_0, std::any_cast<long>(frame.Pop()));
      break;

    case FSTORE_0:
    case FSTORE_1:
    case FSTORE_2:
    case FSTORE_3:
      frame.Set(kOpcode - FSTORE_0, std::any_cast<float>(frame.Pop()));
      break;

    case DSTORE_0:
    case DSTORE_1:
    case DSTORE_2:
    case DSTORE_3:
      frame.Set(kOpcode - DSTORE_0, std::any_cast<double>(frame.Pop()));
      break;

    case ASTORE_0:
    case ASTORE_1:
    case ASTORE_2:
    case ASTORE_3:
      frame.Set(kOpcode - ASTORE_0, frame.Pop());
      break;

    case BASTORE:
    case CASTORE:
    case IASTORE:
    case LASTORE:
    case FASTORE:
    case DASTORE:
    {
      std::any e = frame.Pop();
      int pos = std::any_cast<int>(frame.Pop());
      std::any container = frame.Pop();

      std::shared_ptr<std::vector<std::any>> arr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(container);
      (*arr)[pos] = e;

      break;
    }

    case POP:
      frame.Pop();
      break;

    case POP2:
      frame.Pop();
      frame.Pop();
      break;

    case DUP:
    {
      // Duplicate the element on frame of the stack.
      frame.Push(frame.Top());
      break;
    }

    case IFEQ:
    case IFNEQ:
    case IFLT:
    case IFGE:
    case IFGT:
    case IFLE:
    case IF_ICMPEQ:
    case IF_ICMPNE:
    case IF_ICMPLT:
    case IF_ICMPGE:
    case IF_ICMPGT:
    case IF_ICMPLE:
    {
      if (_Cmp(kOpcode))
      {
        i += TWO_BYTE_CONSTRUCT(kByteCode, i) - 1;
      } else
      {
        i += 2;
      }

      return;
    }

    case IADD:
    case ISUB:
    case IMUL:
    case IDIV:
    case LADD:
    case LSUB:
    case LMUL:
    case LDIV:
    case FADD:
    case FSUB:
    case FMUL:
    case FDIV:
    case DADD:
    case DSUB:
    case DMUL:
    case DDIV:
    {
      std::any o1 = frame.Pop();
      std::any o2 = frame.Pop();
      const std::any result = _Math(kOpcode, o2, o1);
      frame.Push(result);
      break;
    }

    case ISHL:
    case ISHR:
    {
      int val = std::any_cast<int>(frame.Pop());
      int amt = std::any_cast<int>(frame.Pop());
      int res = kOpcode == ISHL ? val << amt : val >> amt;

      frame.Push(res);
      break;
    }

    case IINC:
    {
      int v = std::any_cast<int>(frame.At(kByteCode[i + 1])) + kByteCode[i + 2];
      frame.Set(kByteCode[i + 1], v);

      i += 2;
      return;
    }

    case I2C:
      frame.Push(static_cast<char>(std::any_cast<int>(frame.Pop())));
      break;

    case GOTO:
    {
      // Offset used here is relative and not absolute.
      i += TWO_BYTE_CONSTRUCT(kByteCode, i) - 1;
      return;
    }

    case LDC:
      frame.Push(_klassFile.Ldc(kByteCode[++i]));
      break;

    case RETURN:
      // Hack
      i = INT_MAX;
      return;

    case NEWARRAY:
    {
      std::shared_ptr<std::vector<std::any>> arr = std::make_shared<std::vector<std::any>>();
      arr->resize(any_cast<int>(frame.Pop()));
      frame.Push(arr);

      // Skip atype for now.
      i++;
      break;
    }

    case ARRAYLENGTH:
      frame.Push(std::any_cast<std::shared_ptr<std::vector<std::any>>>(frame.Pop())->size());
      break;

    default:
      throw std::invalid_argument("Invalid opcode");
  }

  i += 1;
}

void CppDuke::VirtualMachine::Interpreter::_ExecMethod(const std::vector<uint8_t> &byteCode,
                                                       const uint16_t bufferSize)
{
  _frames.emplace(bufferSize);
  std::cout << "Executing " << byteCode.size() - 1 << " instructions\n";

  int i = 0;
  while (i < byteCode.size())
  {
    _ExecOpcode(byteCode, i);
  }

  _frames.pop();
}

void CppDuke::VirtualMachine::Interpreter::Run()
{
  std::shared_ptr<ConstantPool::CodeAttribute> entryPoint = _klassFile.GetEntryPoint();
  _ExecMethod(entryPoint->ByteCode(), entryPoint->BufferSize());
}
