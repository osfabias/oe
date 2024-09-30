#include <oe.h>

int main(void) {
  init(1280, 720, "oe application");

  while (!should_close()) { }

  quit();
  return 0;
}
