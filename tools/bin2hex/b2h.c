/* bin to hex */
#include <stdio.h>
#include <string.h>

static unsigned long g;


int main(int argc, char **argv) {
  int i, l;
  char *s;

  if(argc<2) {
    printf("usage: %s <binary num>\n", argv[0]);
    return 1;
  }    

  s = argv[1];
  l = strlen(s);
  if(l>32) {
    printf("input string contains more than 32 chars\n");
    return 1;
  }

  g=0;
  for(i=0; i<l; i++) {
    g<<=1;

    switch(s[i]) {
      case '1':
          g |= 1;
	break;
      case '0': break;
      default: printf("invalid char in input string\n"); return 1;	
    }
  }

  printf("0x%08x\n", g);

  return 0;
}
