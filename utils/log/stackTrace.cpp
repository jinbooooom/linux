#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void printStackTrace() {
  void *callstack[128];
  int frames = backtrace(callstack, 128);
  char **symbols = backtrace_symbols(callstack, frames);

  printf("Stack Trace:\n");
  for (int i = 0; i < frames; ++i) {
    Dl_info info;
    if (dladdr(callstack[i], &info) && info.dli_sname != NULL) {
      printf("%s\n", info.dli_sname);
    } else {
      printf("%s\n", symbols[i]);
    }
  }

  free(symbols);
}

void foo() { printStackTrace(); }

void doo() { foo(); }

int main() {
  doo();
  return 0;
}

/**
编译命令：g++ trace.cpp -ldl -rdynamic 
-ldl 选项用于链接 dl 库，-rdynamic 选项用于保留符号信息

运行结果：
Stack Trace:
_Z15printStackTracev
_Z3foov
_Z3doov
main
__libc_start_main
_start

 */
