/**
 * @file internal.h
 * @brief The header of the internal oe functions.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#pragma once

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
 */
extern void _gfx_init(void);

/**
 * @brief Terminate graphics API.
 */
extern void _gfx_quit(void);

