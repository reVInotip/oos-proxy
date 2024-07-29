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
#include "../../include/guc/guc.h"

#pragma once

/**
 * \brief Level of log mess#pragma onceages
 * \details
 * INFO - informing about an event from the user side.
 *      Logging of these messages can be disabled in the config;
 * DEBUG - information that may be useful for debugging;
 * LOG - informing about an event from the server side;
 * WARN - warnings about problems in the program.
 *      They can influence its work;
 * ERROR - error message. Reports critical problems in the program.
 * TO_DO Add CRITICAL level for emergency termination of the program
 */
typedef enum e_level {
    INFO = 1,
    DEBUG = 2,
    LOG = 3,
    WARN = 4,
    ERROR = 5
} E_LEVEL;

#define elog(elevel, format, ...) \
    do \
    { \
        write_log(elevel, __FILE__, __LINE__, format, ## __VA_ARGS__); \
    } while(0)

extern void write_log(E_LEVEL level,
                    const char *filename,
                    int line_number,
                    const char *format,
                    ...) __attribute__((format(printf, 4, 5)));
extern void write_stderr(const char *format, ...) __attribute__((format(printf, 1, 2)));                        
extern void init_logger();
extern void stop_logger();