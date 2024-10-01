/**
 * @file oe.h
 * @brief The main header of the oe library.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#pragma once

#include <stdlib.h> // exit()

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
extern void init(int width, int height, const char *title);

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
extern int should_close(void);

// +------------------------------------------------------------------+
// |                          drawing                                 |
// +------------------------------------------------------------------+

/**
 * @brief 32 bit color.
 */
typedef struct color {
  unsigned char r, g, b, a;
} color_t;

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

// +------------------------------------------------------------------+
// |                            math                                  |
// +------------------------------------------------------------------+

/**
 * @brief 2 dimensional vector.
 */
typedef struct vec2 {
  float x, y;
} vec2_t;

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

