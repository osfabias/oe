#include <oe.h>

int main(void) {
  init(1280, 720, "oe application");

  while (!should_close()) {
    // draw_begin();
    // draw_end();
  }

  quit();
  return 0;
}
