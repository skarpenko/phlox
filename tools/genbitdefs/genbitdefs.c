#include <stdio.h>

int main() {
  int i;
  unsigned int a = 1;
  
  for(i=0; i<32; i++) {
    printf("#define X86_BIT%d 0x%08x\n", i, a);
    a <<= 1;
  }
  return 0;
}
