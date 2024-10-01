/**
 * @file oe.h
 * @brief The main header of the oe library.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define OE_NULL_HANDLE NULL

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

// +------------------------------------------------------------------+
// |                        initialization                            |
// +------------------------------------------------------------------+

/**
 * @brief Initializes oe.
 *
 * Initializes oe subsystems, opens window and creates surface for
 * rendering graphics.
 *
 * @param width  The width of the window.
 * @param heitgh The height of the window.
 * @param title  The title of the window.
 */
extern void init(i32 width, i32 height, const char *title);

/**
 * @brief Terminares oe.
 *
 * Destroys surface, closes window and terminates oe subsystems.
 */
extern void quit(void);

/**
 * @brief Updates window and returns whether app should be
 *        terminated, or not.
 *
 * @return Returns 1 if the os requested app termination, otherwise
 *         return 0.
 */
extern i32 should_close(void);

// +------------------------------------------------------------------+
// |                          drawing                                 |
// +------------------------------------------------------------------+

/**
 * @brief 32 bit color.
 */
typedef u32 color_t;

/**
 * @brief Rectangle.
 */
typedef struct rect { f32 x, y, width, height; } rect_t;

/**
 * @brief Texture handle.
 */
typedef struct texture* texture_t;

/**
 * @brief Updates window state and starts frame drawing.
 *
 * @param color Clear color.
 */
extern void draw_begin(color_t color);

/**
 * @brief Ends frame drawing and presents drawn frame.
 */
extern void draw_end(void);

/**
 * @brief Draw colored rectangle.
 */
extern void draw_rect(rect_t rect, color_t color, f32 depth);

/**
 * @brief Loads texture.
 *
 * @param path The path to the texture file.
 *
 * @return Returns an oe texture handle on success, otherwise
 *         returns OE_NULL_HANDLE.
 */
extern texture_t texture_load(const char *path);

/**
 * @brief Frees texture.
 *
 * @param texture The texture handle.
 */
extern void texture_free(texture_t texture);

// +------------------------------------------------------------------+
// |                            math                                  |
// +------------------------------------------------------------------+

/**
 * @brief 2 dimensional vector.
 */
typedef struct vec2 { f32 x, y; } vec2_t;

/**
 * @brief 2 dimensional vector.
 */
typedef struct vec3 { f32 x, y, z; } vec3_t;

// +------------------------------------------------------------------+
// |                         debugging                                |
// +------------------------------------------------------------------+

typedef enum log_level {
  LOG_LEVEL_TRACE,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_FATAL
} log_level_t;

/**
 * @brief Ptrints a log message with the given log level.
 *
 * If logging level is lower than current set log level,
 * message wouldn't be displayed.
 *
 * @param level The logging level.
 * @param msg   A pointer to the c-string to log as a message.
 * @param ...   Variadic arguments.
 */
extern void log_msg(log_level_t level, const char *msg, ...);

/**
 * @def trace
 * @brief Prints trace log message.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */

/**
 * @def debug
 * @brief Prints debug log message.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */
#ifdef OE_DEBUG_BUILD
  #define trace(...) log_msg(LOG_LEVEL_TRACE, __VA_ARGS__)
  #define debug(...) log_msg(LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
  #define trace(...)
  #define debug(...)
#endif

/**
 * @brief Prints info log message.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */
#define info(...) log_msg(LOG_LEVEL_INFO, __VA_ARGS__)

/**
 * @brief Prints warn log message.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */
#define warn(msg, ...) \
  log_msg(LOG_LEVEL_WARN, "%s(): " msg, __func__, ##__VA_ARGS__)

/**
 * @brief Prints error log message.
 *
 * error should be used to print messages about errors, that can
 * be ignored or processed.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */
#define error(msg, ...) \
  log_msg(LOG_LEVEL_ERROR, "%s(): " msg, __func__, ##__VA_ARGS__)

/**
 * @brief Prints fatal log message.
 *
 * fatal should be used to print messages about errors, that can't
 * be ignored or processed.
 *
 * @param msg A message to log.
 * @param ... Variadic arguments.
 */
#define fatal(msg, ...) \
{ \
  log_msg(LOG_LEVEL_FATAL, "%s(): " msg, __func__, ##__VA_ARGS__); \
  exit(1); \
}

/**
 * @def OE_ASSERT
 * @brief Runtime assertion macro
 *
 * @param x A intean expression, that's asserted to be 1.
 * @param msg A message to show on assertion fail.
 * @param ... VA arguments.
 */
#ifdef oe_debug 
#define assert(x, ...) \
if (!(x)) { fatal(__VA_ARGS__); }
#else
#define assert(x, ...)
#endif

