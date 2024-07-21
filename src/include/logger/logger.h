/**
 * logger.h
 * This file contains basic methods methods for working with logger.
 * Logs are recorded in a separate folder (its name is set in the config).
 * There are two files in this folder (their names also specified in the config).
 * These files have a size limit (it also set in the config). When one runs out of space, recording begins in
 * another, and so on. When the program starts, recording begins in the first of
 * the files (I mean the first one from the config).
 * The log message consists of time and date then it was writing, the timezone name or abbreviation,
 * level of this message and formated string
 */

#pragma once

/**
 * \brief Level of log messages
 * \details
 * INFO - informing about an event from the user side.
 *      Logging of these messages can be disabled in the config;
 * DEBUG - information that may be useful for debugging;
 * LOG - informing about an event from the server side;
 * WARN - warnings about problems in the program.
 *      They can influence its work;
 * ERROR - error message. Reports critical problems in the program.
 */
typedef enum e_level {
    INFO = 1,
    DEBUG = 2,
    LOG = 3,
    WARN = 4,
    ERROR = 5
} E_LEVEL;

extern void elog(E_LEVEL level, const char *format, ...) __attribute__((format (printf, 2, 3)));
extern void init_logger();
extern void stop_logger();