#include <time.h>

#include <oe.h>

int main(void)
{
  init(1280, 720, "oe application");

  f32 t;
  time_t s = time(NULL);

  while (!should_close()) {
    t += (((time(NULL) - s) / 10.0f) - t) * 0.1f;
    if (t > 0.3f)
      s = time(NULL);

    draw_begin(0x292831ff);
      draw_rect(
        (rect_t){ -0.8f, -0.8f + t, 1.2f + t, 1.7f },
        0xee8695ff, 0.0f
      );

      draw_rect(
        (rect_t){ 0.0f - t, 0.0f + t, 0.8f, 0.2f },
        0x4a7a96ff, 1.0f
      );
    draw_end();
  }

  quit();
  return 0;
}
