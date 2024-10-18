#include <opl.h>

#include "oe.h"
#include "internal.h"

static opl_window_t s_window;

void init(u16 width, u16 height, const char *title)
{
  init_ext(0, 0, width, height, 0, 0, title);
}

void init_ext(u16 x, u16 y, u16 width, u16 height,
              u16 resx, u16 resy, const char *title)
{
  _log_init();

  _input_init();

  if (!opl_init())
    fatal("failed to initialize opl");
  trace("opl initialized");

  s_window = opl_window_open_ext(width, height, title, x, y,
    OPL_WINDOW_HINT_TITLED_BIT |
    OPL_WINDOW_HINT_CLOSABLE_BIT |
    OPL_WINDOW_HINT_MINIATURIZABLE_BIT
  );
  if (!s_window)
    fatal("failed to create window");
  trace("opened window");

  _gfx_init(s_window, resx, resy);

  info("oe initialized");
}

i32 should_close(void)
{
  _input_update();
  opl_update();
  return opl_window_should_close(s_window);
}

void quit(void)
{
  _gfx_quit();

  opl_window_close(s_window);
  trace("widow closed");

  opl_quit();
  trace("opl terminated");

  info("oe terminated");

  _log_quit();
}

