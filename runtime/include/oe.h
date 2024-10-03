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
// |                           input                                  |
// +------------------------------------------------------------------+

/**
 * @brief Keyboard key code.
 */
typedef enum key {
  KEY_BACKSPACE = 0x08,
  KEY_ENTER     = 0x0D,
  KEY_TAB       = 0x09,
  KEY_SHIFT     = 0x10,
  KEY_CONTROL   = 0x11,

  KEY_PAUSE    = 0x13,
  KEY_CAPSLOCK = 0x14,

  KEY_ESCAPE = 0x1B,

  KEY_CONVERT    = 0x1C,
  KEY_NONCONVERT = 0x1D,
  KEY_ACCEPT     = 0x1E,
  KEY_MODECHANGE = 0x1F,

  KEY_SPACE = 0x20,

  KEY_PAGEUP   = 0x21,
  KEY_PAGEDOWN = 0x22,
  KEY_END      = 0x23,
  KEY_HOME     = 0x24,

  KEY_LEFT  = 0x25,
  KEY_UP    = 0x26,
  KEY_RIGHT = 0x27,
  KEY_DOWN  = 0x28,

  KEY_SELECT      = 0x29,
  KEY_PRINT       = 0x2A,
  KEY_EXECUTE     = 0x2B,
  KEY_PRINTSCREEN = 0x2C,
  KEY_INSERT      = 0x2D,
  KEY_DELETE      = 0x2E,
  KEY_HELP        = 0x2F,

  KEY_0 = 0x30,
  KEY_1 = 0x31,
  KEY_2 = 0x32,
  KEY_3 = 0x33,
  KEY_4 = 0x34,
  KEY_5 = 0x35,
  KEY_6 = 0x36,
  KEY_7 = 0x37,
  KEY_8 = 0x38,
  KEY_9 = 0x39,

  KEY_A = 0x41,
  KEY_B = 0x42,
  KEY_C = 0x43,
  KEY_D = 0x44,
  KEY_E = 0x45,
  KEY_F = 0x46,
  KEY_G = 0x47,
  KEY_H = 0x48,
  KEY_I = 0x49,
  KEY_J = 0x4A,
  KEY_K = 0x4B,
  KEY_L = 0x4C,
  KEY_M = 0x4D,
  KEY_N = 0x4E,
  KEY_O = 0x4F,
  KEY_P = 0x50,
  KEY_Q = 0x51,
  KEY_R = 0x52,
  KEY_S = 0x53,
  KEY_T = 0x54,
  KEY_U = 0x55,
  KEY_V = 0x56,
  KEY_W = 0x57,
  KEY_X = 0x58,
  KEY_Y = 0x59,
  KEY_Z = 0x5A,

  KEY_LSUPER = 0x5B,
  KEY_RSUPER = 0x5C,

  KEY_APPS  = 0x5D,
  KEY_SLEEP = 0x5F,

  KEY_NUMPAD0   = 0x60,
  KEY_NUMPAD1   = 0x61,
  KEY_NUMPAD2   = 0x62,
  KEY_NUMPAD3   = 0x63,
  KEY_NUMPAD4   = 0x64,
  KEY_NUMPAD5   = 0x65,
  KEY_NUMPAD6   = 0x66,
  KEY_NUMPAD7   = 0x67,
  KEY_NUMPAD8   = 0x68,
  KEY_NUMPAD9   = 0x69,
  KEY_MULTIPLY  = 0x6A,
  KEY_ADD       = 0x6B,
  KEY_SEPARATOR = 0x6C,
  KEY_SUBTRACT  = 0x6D,
  KEY_DECIMAL   = 0x6E,
  KEY_DIVIDE    = 0x6F,

  KEY_F1  = 0x70,
  KEY_F2  = 0x71,
  KEY_F3  = 0x72,
  KEY_F4  = 0x73,
  KEY_F5  = 0x74,
  KEY_F6  = 0x75,
  KEY_F7  = 0x76,
  KEY_F8  = 0x77,
  KEY_F9  = 0x78,
  KEY_F10 = 0x79,
  KEY_F11 = 0x7A,
  KEY_F12 = 0x7B,
  KEY_F13 = 0x7C,
  KEY_F14 = 0x7D,
  KEY_F15 = 0x7E,
  KEY_F16 = 0x7F,
  KEY_F17 = 0x80,
  KEY_F18 = 0x81,
  KEY_F19 = 0x82,
  KEY_F20 = 0x83,
  KEY_F21 = 0x84,
  KEY_F22 = 0x85,
  KEY_F23 = 0x86,
  KEY_F24 = 0x87,

  KEY_NUMLOCK      = 0x90,
  KEY_SCROLL       = 0x91,
  KEY_NUMPAD_EQUAL = 0x92,

  KEY_LSHIFT   = 0xA0,
  KEY_RSHIFT   = 0xA1,
  KEY_LCONTROL = 0xA2,
  KEY_RCONTROL = 0xA3,
  KEY_LALT     = 0xA4,
  KEY_RALT     = 0xA5,

  KEY_SEMICOLON = 0x3B,

  KEY_APOSTROPHE = 0xDE,
  KEY_EQUAL      = 0xBB,
  KEY_COMMA      = 0xBC,
  KEY_MINUS      = 0xBD,
  KEY_PERIOD     = 0xBE,
  KEY_SLASH      = 0xBF,

  KEY_GRAVE = 0xC0,

  KEY_LBRACKET  = 0xDB,
  KEY_BACKSLASH = 0xDC,
  KEY_RBRACKET  = 0xDD,
} key_t;

/**
 * @brief Mouse button code.
 */
typedef enum btn {
  BTN_LEFT,
  BTN_RIGHT,
  BTN_MIDDLE,
} btn_t;

/**
 * @brief Returns whether the key was just pressed this frame or not.
 */
extern int is_key_pressed(key_t key);

/**
 * @brief Returns whether the key was down this frame or not.
 */
extern int is_key_down(key_t key);

/**
 * @brief Returns whether the key was just released this frame or not.
 */
extern int is_key_released(key_t key);

/**
 * @brief Returns whether the mouse button was just pressed
 *        this frame or not.
 */
extern int is_btn_pressed(btn_t btn);

/**
 * @brief Returns whether the mouse button was
 *        just released this frame or not.
 */
extern int is_btn_released(btn_t btn);

/**
 * @brief Returns whether the mouse button was up
 *        this frame or not.
 */
extern int is_btn_up(btn_t btn);

/**
 * @brief Returns mouse position in the window.
 */
extern vec2_t mouse_pos(void);

/**
 * @brief Returns the mouse wheel scroll this frame.
 */
extern float mouse_wheel(void);

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
typedef struct rect {
  f32 x, y;
  f32 width, height;
} rect_t;

/**
 * @brief Texture handle.
 */
typedef struct texture* texture_t;

/**
 * @brief Camera.
 */
typedef struct camera {
  vec2_t pos;
  f32    zoom;
  f32    rotation;
} camera_t;

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
 * @brief Sets camera.
 */
extern void camera_set(camera_t cam);

/**
 * @brief Resets camera.
 */
extern void camera_reset(void);

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
// |                           utils                                  |
// +------------------------------------------------------------------+

extern double get_time(void);

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

