#undef NDEBUG

#include <assert.h>
#include <stdint.h>

#include "./array.h"
static void test_array() {
  int32_t* numbers = NULL;
  assert(container_array_get_length(numbers) == 0);

  container_array_reserve(numbers, 10);
  for (int32_t i = 50; i < 100; ++i) {
    container_array_insert(numbers, i-50);
    numbers[i-50] = i;
  }
  container_array_reserve(numbers, 10);
  for (int32_t i = 0; i < 50; ++i) {
    container_array_insert(numbers, i);
    numbers[i] = i;
  }
  for (int32_t i = 0; i < 100; ++i) {
    assert(numbers[i] == i);
  }

  for (int32_t i = 99; i >= 0; i-=2) {
    container_array_remove(numbers, i);
  }
  for (int32_t i = 0; i < 100; i+=2) {
    assert(numbers[i/2] == i);
  }
  assert(container_array_get_length(numbers) == 50);

  container_array_resize(numbers, 25);
  assert(container_array_get_length(numbers) == 25);

  int32_t* dup = container_array_duplicate(numbers);
  for (int32_t i = 0; i < 25; ++i) {
    assert(numbers[i] == dup[i]);
  }
  container_array_delete(dup);

  container_array_delete(numbers);
}

int main(void) {
  test_array();
  return 0;
}
