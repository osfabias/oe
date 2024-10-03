/**
 * @file internal.h
 * @brief The header of the internal oe functions.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#pragma once

#include <stdint.h>

#include <opl.h>

#include "oe.h"

typedef struct _vert {
  vec3_t  pos;
  color_t color;
  vec2_t  uv;
} _vert_t;

typedef struct _ubo {
  camera_t cam;
} _ubo_t;

/**
 * @brief Initializes logging system.
 */
extern void _log_init(void);

/**
 * @brief Terminates logging system.
 */
extern void _log_quit(void);

/**
 * @brief Initializes input system.
 */
extern void _input_init(void);

/**
 * @brief Updates input system.
 *
 * This function should be called before opl_update call.
 */
extern void _input_update(void);

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

