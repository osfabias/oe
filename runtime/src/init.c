#include <opl.h>

#include "oe.h"
#include "internal.h"

static opl_window_t s_window;

void init(int width, int height, const char *title) {
  _log_init();

  if (!opl_init())
    fatal("failed to initialize opl");
  info("opl initialized");

  s_window = opl_window_open(width, height, title);
  if (!s_window)
    fatal("failed to create window");
  info("opened window");

  _gfx_init();
}

int should_close(void) {
  opl_update();
  return opl_window_should_close(s_window);
}

void quit(void) {
  _gfx_quit();

  opl_window_close(s_window);
  info("widow closed");

  opl_quit();
  info("opl terminated");

  _log_quit();
}

