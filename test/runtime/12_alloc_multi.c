// clang-format off
// RUN: %run %s 2>&1 | FileCheck %s
// RUN: %run %s "-call-filter" 2>&1 | FileCheck %s --check-prefix CHECK-FILTER
// clang-format on

#include <stdlib.h>

void call() {
  const int n = 10;
  double a[n];
  double b[n];
}

int main(int argc, char** argv) {
  const int n = 42;
  // CHECK: [Trace] TypeART Runtime Trace
  // CHECK-FILTER-NOT: [Trace] TypeART Runtime Trace

  // CHECK: [Trace] Alloc 0x{{.*}} int8 1 42
  char a[n];

  // CHECK: [Trace] Alloc 0x{{.*}} int16 2 42
  short b[n];

  // CHECK: [Trace] Alloc 0x{{.*}} int32 4 42
  int c[n];

  // CHECK: [Trace] Alloc 0x{{.*}} int64 8 42
  long d[n];

  // CHECK: [Trace] Alloc 0x{{.*}} float 4 42
  float e[n];

  // CHECK: [Trace] Alloc 0x{{.*}} double 8 42
  double f[n];

  // CHECK: [Trace] Alloc 0x{{.*}} pointer 8 42
  int* g[n];

  // CHECK: [Trace] Alloc 0x{{.*}} double 8 10
  // CHECK: [Trace] Alloc 0x{{.*}} double 8 10
  call();
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}

  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}
  // CHECK: [Trace] Free 0x{{.*}}

  return 0;
}
