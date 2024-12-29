#include <asmjit/asmjit.h>
#include <stack>
#include <stdio.h>

#include "util.h"

using namespace asmjit;

// Signature of the generated function.
typedef void (*JitFunc)(void);

// tape size and pointer
#define TAPE_SIZE 30000
#define PTR x86::r12

#define DEBUG 0

void prologue(x86::Assembler &as) {
  as.push(x86::rbp);
  as.mov(x86::rbp, x86::rsp);
  as.push(PTR); // preserve callee-saved r12
  as.sub(x86::rsp, TAPE_SIZE);

  // call memset, follow x86 calling convention
  as.lea(x86::rdi, x86::Mem(x86::rsp, 0));
  as.mov(x86::esi, 0);
  as.mov(x86::rdx, TAPE_SIZE);
  as.call(memset);

  as.mov(PTR, x86::rsp);
}

void epilogue(x86::Assembler &as) {
  as.add(x86::rsp, TAPE_SIZE);
  as.pop(PTR);
  as.pop(x86::rbp);
  as.ret();
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    err("Usage: jit <bf program>");

  char *file_contents = read_file(argv[1]);
  if (file_contents == NULL)
    err("Couldn't open file");

  std::stack<std::pair<Label, Label>> pcstack;

  // Runtime designed for JIT - it holds relocated functions and controls their
  // lifetime.
  JitRuntime rt;

  // Holds code and relocation information during code generation.
  CodeHolder code;

#if DEBUG
  FileLogger logger(stdout);
  code.setLogger(&logger);
#endif

  // Code holder must be initialized before it can be used. The simples way to
  // initialize it is to use 'Environment' from JIT runtime, which matches the
  // target architecture, operating system, ABI, and other important properties.
  code.init(rt.environment(), rt.cpuFeatures());

  // Emitters can emit code to CodeHolder - let's create 'x86::Assembler', which
  // can emit either 32-bit (x86) or 64-bit (x86_64) code. The following line
  // also attaches the assembler to CodeHolder, which calls 'code.attach(&a)'
  // implicitly.
  x86::Assembler as(&code);

  prologue(as);

  for (char *p = file_contents; *p; p++) {
    switch (*p) {
    case '>':
      as.inc(PTR);
      break;
    case '<':
      as.dec(PTR);
      break;
    case '+':
      as.inc(x86::byte_ptr(PTR));
      break;
    case '-':
      as.dec(x86::byte_ptr(PTR));
      break;
    case '.':
      as.movzx(x86::edi, x86::byte_ptr(PTR));
      as.mov(x86::rax, putchar);
      as.call(x86::rax);
      break;
    case ',':
      as.mov(x86::rax, getchar);
      as.call(x86::rax);
      as.mov(x86::byte_ptr(PTR), x86::al);
      break;
    case '[': {
      auto label_left = as.newLabel();
      auto label_right = as.newLabel();
      as.cmp(x86::byte_ptr(PTR), 0);
      as.je(label_right);
      as.bind(label_left);
      pcstack.push({std::move(label_left), std::move(label_right)});
      break;
    }
    case ']': {
      auto &labels = pcstack.top();
      as.cmp(x86::byte_ptr(PTR), 0);
      as.jne(labels.first);
      as.bind(labels.second);
      pcstack.pop();
      break;
    }
    }
  }

  epilogue(as);

  // Now add the generated code to JitRuntime via JitRuntime::add(). This
  // function would copy the code from CodeHolder into memory with executable
  // permission and relocate it.
  JitFunc fn;
  Error err = rt.add(&fn, &code);

  if (err) {
    printf("AsmJit failed: %s\n", DebugUtils::errorAsString(err));
    return 1;
  }

  fn();

  return 0;
}
