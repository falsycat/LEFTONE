#include "./memory.h"

int main(void) {
  void* ptr = memory_new(0x1);
  ptr = memory_resize(ptr, 0x2);
  memory_delete(ptr);
  return 0;
}
