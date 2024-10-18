#include <time.h>
#include <math.h>
#include <stdlib.h>

#include <oe.h>

int main(void)
{
  init_ext(1280, 720, 360, 180, "oe application");

  time_t seed = time(NULL);

  texture_t tex    = texture_load("assets/textures/pic.png");
  texture_t tex2   = texture_load("assets/textures/pic2.png");
  texture_t deftex = texture_load("assets/textures/default.png");

  texture_bind(tex, 0);
  texture_bind(tex2, 1);
  texture_bind(deftex, DEFAULT_TEXTURE_IND);

  camera_t cam = {
    .pos      = { 0.0f, 0.0f },
    .view     = { 360.0f, 180.0f },
    .zoom     = 1.0f,
  };

  vec2_t lst_mouse_pos = mouse_pos();

  while (!should_close()) {
    // scene update
    int haxis = is_key_down(KEY_D) - is_key_down(KEY_A);
    int vaxis = is_key_down(KEY_S) - is_key_down(KEY_W);
    cam.pos.x += haxis / cam.zoom;
    cam.pos.y += vaxis / cam.zoom;

    cam.zoom -= mouse_wheel() * 0.01f * cam.zoom;
    if (cam.zoom < 0.1e-2)
      cam.zoom = 0.1e-2;
    if (cam.zoom > 0.1e2)
      cam.zoom = 0.1e2;

    if (is_btn_down(BTN_RIGHT)) {
      cam.pos.x += (lst_mouse_pos.x - mouse_pos().x) / cam.zoom;
      cam.pos.y += (lst_mouse_pos.y - mouse_pos().y) / cam.zoom;
    }

    // frame draw
    draw_begin(0x010101ff);
      // scene draw
      camera_set(cam);

        srand(seed);
        for (int i = 0; i < 15876; ++i) {
          draw_texture_ext(
            (rect_t){
              -0.9f + (i % 126) * 100.0f + (float)rand() / RAND_MAX + sin(get_time() / 2000.0f + rand()) * 20.0f,
              -0.8f + (i / 126) * 190.0f + (float)rand() / RAND_MAX + cos(get_time() / 4000.0f + rand()) * 10.0f,
              50.0f + (float)rand() / RAND_MAX + sin(get_time() / 2000.0f + rand()) * 700.0f,
              90.0f + (float)rand() / RAND_MAX + cos(get_time() / 4000.0f + rand()) * 500.0f
            },
            (rect_t) { 0.0f, 0.0f, 1.0f, 1.0f },
            rand() % MAX_TEXTURE_COUNT,
            rand(),
            0.0f,
            0.0f
          );
        }

      camera_reset();

    // ui draw
    draw_end();

    lst_mouse_pos = mouse_pos();
  }

  draw_wait();

  texture_free(tex);
  texture_free(tex2);
  texture_free(deftex);

  quit();

  return 0;
}

