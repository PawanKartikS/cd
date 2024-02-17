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

CppDuke::VirtualMachine::Interpreter::Interpreter(const std::vector<Klass> &klasses,
                                                  const std::string &kMain)
    : _main(kMain),
      _klasses(klasses)
{
}

template<typename _Ty>
_Ty CppDuke::VirtualMachine::Interpreter::_Bitwise(const uint8_t& kOpcode)
{
  _Ty v1 = std::any_cast<_Ty>(_frames.top().Pop());
  std::any v2 = _frames.top().Pop();

  _Ty res{};
  switch (kOpcode)
  {
    case ISHL:
    case LSHL:
      res = v1 << std::any_cast<int>(v2);
      break;

    case ISHR:
    case LSHR:
      res = v1 >> std::any_cast<int>(v2);
      break;

    case IAND:
    case LAND:
      res = v1 & std::any_cast<_Ty>(v2);
      break;

    case IOR:
    case LOR:
      res = v1 | std::any_cast<_Ty>(v2);
      break;

    case IXOR:
    case LXOR:
      res = v1 ^ std::any_cast<_Ty>(v2);
      break;

    default:
      // panic
      break;
  }

  return res;
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

template<typename _Ty>
_Ty CppDuke::VirtualMachine::Interpreter::_Math(const uint8_t &opcode)
{
  _Ty v1 = std::any_cast<_Ty>(_frames.top().Pop());
  _Ty v2 = std::any_cast<_Ty>(_frames.top().Pop());

  switch (opcode)
  {
    case IADD:
    case LADD:
    case FADD:
    case DADD:
      return v1 + v2;

    case ISUB:
    case LSUB:
    case FSUB:
    case DSUB:
      return v2 - v1;

    case IMUL:
    case LMUL:
    case FMUL:
    case DMUL:
      return v1 * v2;

    case IDIV:
    case LDIV:
    case FDIV:
    case DDIV:
      return v2 / v1;

    default:
      throw std::invalid_argument("Cannot perform math op for opcode");
  }
}

void CppDuke::VirtualMachine::Interpreter::_ExecOpcode(const std::vector<uint8_t> &kByteCode,
                                                       int &i,
                                                       std::any &rval)
{
  const Klass &klass = _trace.top();
  const uint8_t kOpcode = kByteCode[i];
  printf("%d: %x\n", i, kOpcode);

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
      frame.Push(kOpcode - ICONST_0);
      break;

    case LCONST_0:
    case LCONST_1:
      frame.Push(kOpcode - LCONST_0);
      break;

    case FCONST_0:
    case FCONST_1:
    case FCONST_2:
      frame.Push(kOpcode - FCONST_0);
      break;

    case DCONST_0:
    case DCONST_1:
      frame.Push(kOpcode - DCONST_0);
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

    case AALOAD:
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

    case DUP2:
    {
      std::any v1 = frame.Pop();
      std::any v2 = frame.Pop();

      frame.Push(v2);
      frame.Push(v1);
      frame.Push(v2);
      frame.Push(v1);
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

    case SWAP:
    {
      std::any v1 = frame.Pop();
      std::any v2 = frame.Pop();

      frame.Push(v1);
      frame.Push(v2);

      break;
    }

    case IADD:
    case ISUB:
    case IMUL:
    case IDIV:
      frame.Push(_Math<int>(kOpcode));
      break;

    case LADD:
    case LSUB:
    case LMUL:
    case LDIV:
      frame.Push(_Math<long>(kOpcode));
      break;

    case FADD:
    case FSUB:
    case FMUL:
    case FDIV:
      frame.Push(_Math<float>(kOpcode));
      break;

    case DADD:
    case DSUB:
    case DMUL:
    case DDIV:
      frame.Push(_Math<double>(kOpcode));
      break;

    case INVOKESTATIC:
    {
      std::shared_ptr<CppDuke::ConstantPool::GenericEntry> methodRef = klass.ResolveLowHigh(
          TWO_BYTE_CONSTRUCT(kByteCode, i));
      int argCount;
      std::shared_ptr<ConstantPool::CodeAttribute> method = klass.Invoke(methodRef->Low(),
                                                                              methodRef->High(),
                                                                              argCount);
      _ExecMethod(method->ByteCode(), method->BufferSize(), argCount);

      i += 2;
      break;
    }

    case ISHL:
    case ISHR:
    case IAND:
    case IOR:
    case IXOR:
      frame.Push(_Bitwise<int>(kOpcode));
      break;

    case LSHL:
    case LSHR:
    case LAND:
    case LOR:
    case LXOR:
      frame.Push(_Bitwise<long>(kOpcode));
      break;

    case IINC:
    {
      int v = std::any_cast<int>(frame.At(kByteCode[i + 1])) + kByteCode[i + 2];
      frame.Set(kByteCode[i + 1], v);

      i += 2;
      return;
    }

    case I2L:
      frame.Push(static_cast<long>(std::any_cast<int>(frame.Pop())));
      break;

    case I2F:
      frame.Push(static_cast<float>(std::any_cast<int>(frame.Pop())));
      break;

    case I2D:
      frame.Push(static_cast<double>(std::any_cast<int>(frame.Pop())));
      break;

    case L2I:
      frame.Push(static_cast<int>(std::any_cast<long>(frame.Pop())));
      break;

    case L2F:
      frame.Push(static_cast<float>(std::any_cast<long>(frame.Pop())));
      break;

    case L2D:
      frame.Push(static_cast<double>(std::any_cast<long>(frame.Pop())));
      break;

    case F2I:
      frame.Push(static_cast<int>(std::any_cast<float>(frame.Pop())));
      break;

    case F2L:
      frame.Push(static_cast<long>(std::any_cast<float>(frame.Pop())));
      break;

    case F2D:
      frame.Push(static_cast<double>(std::any_cast<float>(frame.Pop())));
      break;

    case D2I:
      frame.Push(static_cast<int>(std::any_cast<double>(frame.Pop())));
      break;

    case D2L:
      frame.Push(static_cast<long>(std::any_cast<double>(frame.Pop())));
      break;

    case D2F:
      frame.Push(static_cast<float>(std::any_cast<double>(frame.Pop())));
      break;

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
      frame.Push(klass.Ldc(kByteCode[++i]));
      break;

    case IRETURN:
      rval = std::any_cast<int>(frame.Pop());
      break;

    case LRETURN:
      rval = std::any_cast<long>(frame.Pop());
      break;

    case FRETURN:
      rval = std::any_cast<float>(frame.Pop());
      break;

    case DRETURN:
      rval = std::any_cast<double>(frame.Pop());
      break;

    case ARETURN:
      rval = frame.Pop();
      break;

    case RETURN:
      break;

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

    case IFNULL:
      if (frame.Pop().type() == typeid(nullptr))
      {
        // Offset used here is relative and not absolute.
        i += TWO_BYTE_CONSTRUCT(kByteCode, i) - 1;
      }
      break;

    case IFNONNULL:
      if (frame.Pop().type() != typeid(nullptr))
      {
        // Offset used here is relative and not absolute.
        i += TWO_BYTE_CONSTRUCT(kByteCode, i) - 1;
      }
      break;

    case GOTO_W:
      i += (kByteCode[i + 1] << 24) | (kByteCode[i + 2] << 16) | (kByteCode[i + 3] << 8) | kByteCode[i + 4];
      return;

    case BREAKPOINT:
    case IMPDEP1:
    case IMPDEP2:
      // Reserved. Decide what to do later.
      break;

    default:
      throw std::invalid_argument("Invalid opcode");
  }

  i += 1;
}

void CppDuke::VirtualMachine::Interpreter::_ExecMethod(const std::vector<uint8_t> &byteCode,
                                                       const uint16_t bufferSize,
                                                       const int kParams)
{
  Frame caller{bufferSize};
  if (!_frames.empty() && kParams > 0)
  {
    Frame &callee = _frames.top();
    for (int i = kParams - 1; i >= 0; i--)
    {
      caller.Set(i, callee.Pop());
    }
  }

  _frames.emplace(caller);
  std::cout << "Executing "
            << byteCode.size() - 1
            << " instructions with "
            << kParams << "\n";

  int i = 0;
  std::any rval;
  while (i < byteCode.size())
  {
    _ExecOpcode(byteCode, i, rval);
  }

  _frames.pop();
  if (rval.has_value() && !_frames.empty())
  {
    _frames.top().Push(rval);
  }
}

std::shared_ptr<CppDuke::ConstantPool::CodeAttribute>
CppDuke::VirtualMachine::Interpreter::_LookupEntryPoint(const Klass& klass)
{
  int i = 0, found = 0;
  for (std::shared_ptr<ConstantPool::PoolEntry> e: klass.Pool())
  {
    if (auto casted = std::dynamic_pointer_cast<ConstantPool::Utf8>(e);
      casted
      && casted->Data() == "([Ljava/lang/String;)V")
    {
      found = 1;
      break;
    }

    i++;
  }

  if (found)
  {
    for (const ConstantPool::CommonRef &m: klass.Methods())
    {
      if (m.DescIndex() == i + 1 && m.Flags() == (0x0001 | 0x0008))
      {
        for (const ConstantPool::CommonAttribute &attr: m.GetChildAttributes())
        {
          if (std::shared_ptr<ConstantPool::CodeAttribute> code = attr.GetCodeAttribute())
          {
            return code;
          }
        }
      }
    }
  }

  return nullptr;
}

void CppDuke::VirtualMachine::Interpreter::Run()
{
  auto main = std::find_if(std::begin(_klasses), std::end(_klasses),
                           [this](const Klass &k) -> bool
                           {
                             return k.Name() == _main;
                           });

  if (main == std::end(_klasses))
  {
    fprintf(stderr, "Could not find class %s\n", _main.c_str());
    return;
  }

  auto entryPoint = Interpreter::_LookupEntryPoint(*main);
  if (entryPoint)
  {
    _trace.push(*main);
    _ExecMethod(entryPoint->ByteCode(), entryPoint->BufferSize(), /* Temp */ 0);
    _trace.pop();
  }
  else
  {
    fprintf(stderr, "Could not find entry point in class %s\n", _main.c_str());
  }
}

bool CppDuke::VirtualMachine::Interpreter::CanInline(const ConstantPool::CodeAttribute &method)
{
  return method.ByteCode().size() <= 35;
}
