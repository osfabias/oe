/**
 * @file internal.h
 * @brief The implementation of the internal oe functions.
 *
 * @date 30.09.2024
 * @author Ilya Buravov
 */
#include <stdio.h>
#include <stdarg.h>

#include <opl.h>
#include <arch.h>

#include "oe.h"
#include "internal.h"

static arch_logger_t s_logger;

void _log_init(void) {
  const arch_logger_create_info_t arch_logger_create_info = {
    .path_fmt = "./logs/",
    .filename_fmt = "#d.#M.#y_#h:#m:#s.txt",
    .msg_fmts = {
      "\033[36m ~ \033[37;3m#t\033[0m\n",
      "\033[34m ℹ\033[0m #t\n",
      "\033[32m ✓\033[0m #t\n",
      "\n\033[37m[#h:#m:#s]\033[0m \033[43;30m WARNING \033[0m #t\n\n",
      "\n\033[37m[#h:#m:#s]\033[0m \033[41;30m  ERROR  \033[0m #t\n\n",
      "\n\033[37m[#h:#m:#s]\033[0m \033[41;38m  FATAL  \033[0m #t\n\n",
    },
    .file_msg_fmts = {
      "[#h:#m:#s] TRACE | #t\n",
      "[#h:#m:#s] DEBUG | #t\n",
      "[#h:#m:#s] INFO  | #t\n",
      "[#h:#m:#s] WARN  | #t\n",
      "[#h:#m:#s] ERROR | #t\n",
      "[#h:#m:#s] FATAL | #t\n",
    },
    .level = ARCH_LOG_LEVEL_TRACE,
  };

  s_logger = arch_logger_create(&arch_logger_create_info);
  if (!s_logger)
    fatal("failed to create an Archivio logger instance");

  info("logger initialized");
}

void _log_quit(void) {
  arch_logger_destroy(s_logger);
  info("log terminated"); 
}

void log_msg(log_level_t level, const char *msg, ...) {
  assert(msg, "passed msg is a null pointer");

  va_list valist;
  va_start(valist, msg);

  if (
    !s_logger ||
    !arch_logvl(s_logger, (arch_log_level_t)level, msg, valist)
  )
    goto SIMPLE_LOG;

  va_end(valist);

  if (level == LOG_LEVEL_FATAL)
    arch_logger_destroy(s_logger);

  return;

SIMPLE_LOG:
  ; // THIS SHOULD BE HERE!
  // ignore error codes
  vprintf(msg, valist);
  puts("");
  va_end(valist);
}

