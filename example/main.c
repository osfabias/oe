#include <time.h>
#include <stdlib.h>

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
      srand(0);
      for (int i = 0; i < 1000; ++i) {
        draw_rect(
          (rect_t){
            -0.9f + (i % 60) / 35.0f,
            -0.8f + t + i / 600.0f,
            .1f,
            .1f
          },
          rand(), 0.0f
        );
      }
    draw_end();
  }

  quit();
  return 0;
}
