#include <time.h>
#include <stdlib.h>

#include <oe.h>

int main(void)
{
  init(1280, 720, "oe application");

  time_t seed = time(NULL);

  texture_t tex = texture_load("assets/textures/character.png");

  camera_t cam = {
    .pos      = { 0.0f, 0.0f },
    .zoom     = 1.0f,
    .rotation = 0.0f
  };

  texture_bind(tex);

  while (!should_close()) {
    // scene update
    int haxis = is_key_down(KEY_D) - is_key_down(KEY_A);
    int vaxis = is_key_down(KEY_S) - is_key_down(KEY_W);
    cam.pos.x += haxis / cam.zoom * 0.01f;
    cam.pos.y += vaxis / cam.zoom * 0.01f;

    cam.zoom -= mouse_wheel() * 0.01f * cam.zoom;
    if (cam.zoom < 0.1e-2)
      cam.zoom = 0.1e-2;
    if (cam.zoom > 0.1e2)
      cam.zoom = 0.1e2;

    // frame draw
    draw_begin(0x010101ff);
      // scene draw
      camera_set(cam);

        srand(seed);
        for (int i = 0; i < 15876; ++i) {
          draw_rect(
            (rect_t){
              -0.9f + (i % 126) * 0.36f + (float)rand() / RAND_MAX,
              -0.8f + (i / 126) * 0.64f + (float)rand() / RAND_MAX,
              0.18f + ((float)rand() / RAND_MAX) * 0.5f,
              0.32f + ((float)rand() / RAND_MAX) * 0.5f
            },
            rand(), 0.0f
          );
        }

      camera_reset();

    // ui draw
    draw_end();
  }

  texture_free(tex);

  quit();
  return 0;
}

