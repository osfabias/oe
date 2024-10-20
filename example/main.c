#include <time.h>
#include <stdlib.h>

#include <oe.h>

#include "core/room.h"

int main(void)
{
  init_ext(1280, 720, 320, 180, "oe application");

  texture_t tex = texture_load("assets/textures/tilemap.png");
  texture_t deftex = texture_load("assets/textures/default.png");

  texture_bind(tex, 0);
  texture_bind(deftex, DEFAULT_TEXTURE_IND);

  room_init();

  while (!should_close()) {
    room_update(0.016f);

    // frame draw
    draw_begin(0x010101ff);
      room_draw();
    draw_end();
  }

  draw_wait();

  texture_free(tex);
  texture_free(deftex);

  quit();

  return 0;
}

