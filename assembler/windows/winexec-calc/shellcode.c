#include <stdio.h>

int main() {

  int a = 42;
  printf("a=%d\n", a);

  unsigned char shellcode[] =
   "\x31\xc9\x64\xa1\x30\x00\x00\x00\x8b\x40\x0c\x8b\x70\x14\xad\x96\xad\x8b\x58\x10\x8b\x53\x3c\x01\xda\x8b\x52\x78\x01\xda\x8b\x72\x20\x01\xde\x31\xc9\x41\xad\x01\xd8\x81\x38\x47\x65\x74\x50\x75\xf4\x81\x78\x04\x72\x6f\x63\x41\x75\xeb\x81\x78\x08\x64\x64\x72\x65\x75\xe2\x8b\x72\x24\x01\xde\x66\x8b\x0c\x4e\x49\x8b\x72\x1c\x01\xde\x8b\x14\x8e\x01\xda\x31\xf6\x52\x5e\x31\xff\x53\x5f\x68\x78\x65\x63\x00\x68\x57\x69\x6e\x45\x89\xe1\x51\x53\xff\xd2\x68\x65\x73\x73\x00\x68\x50\x72\x6f\x63\x68\x45\x78\x69\x74\x89\xe1\x51\x57\x89\xc7\xff\xd6\x50\x5e\x6a\x00\x68\x43\x41\x4c\x43\x89\xe1\x6a\x00\x51\xff\xd7\x6a\x00\xff\xd6";

  void (*funcPtr)() = (void(*)())shellcode;

  funcPtr();
  printf("Apparently not reached, a=%d\n", a);

}
