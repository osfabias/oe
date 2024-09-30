/**
 * @file internal.h
 * @brief The header of the internal oe functions.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#pragma once

#include <opl.h>

#include "oe.h"

typedef struct vert {
  vec2_t  pos;
  color_t color;
  vec2_t  tex_coord;
} vert_t;

/**
 * @brief Initializes logging system.
 */
extern void _log_init(void);

/**
 * @brief Terminates logging system.
 */
extern void _log_quit(void);

/**
 * @brief Initialized graphics API.
 *
 * @param window An opl window handle of the main window.
 */
extern void _gfx_init(opl_window_t window);

/**
 * @brief Terminate graphics API.
 */
extern void _gfx_quit(void);

