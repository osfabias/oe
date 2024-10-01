#include <oe.h>

int main(void) {
  init(1280, 720, "oe application");

  while (!should_close()) {
    draw_begin((color_t){3, 3, 3, 255});
    draw_end();
  }

  quit();
  return 0;
}
