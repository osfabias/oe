#include <opl.h>

#include "oe.h"
#include "internal.h"

static opl_window_t s_window;

void init(i32 width, i32 height, const char *title)
{
  _log_init();

  if (!opl_init())
    fatal("failed to initialize opl");
  trace("opl initialized");

  s_window = opl_window_open(width, height, title);
  if (!s_window)
    fatal("failed to create window");
  trace("opened window");

  _gfx_init(s_window);

  info("oe initialized");
}

i32 should_close(void)
{
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

