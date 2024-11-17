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

#define USE_VDSO_TIME

//#define SIMPLE_LOGGER

/**
 * \brief Level of log mess#pragma onceages
 * \details
 * INFO - informing about an event from the user side.
 *      Logging of these messages can be disabled in the config;
 * DEBUG - information that may be useful for debugging;
 * LOG - informing about an event from the server side;
 * WARN - warnings about problems in the program.
 *      They can influence its work;
 * ERROR - error message. Reports critical problems in the program;
 * FATAL - after that error programm can not work and will be terminated with exit code 14.
 */
typedef enum e_level {
    DEBUG,
    LOG,
    WARN,
    ERROR,
    FATAL
} E_LEVEL;

#ifdef SIMPLE_LOGGER
#define elog(elevel, format, ...) \
    do \
    { \
        write_log(elevel, format, ## __VA_ARGS__); \
    } while(0)

extern void write_log(E_LEVEL level,
                    const char *format,
                    ...) __attribute__((format(printf, 2, 3)));
#else
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
#endif

extern void write_stderr(const char *format, ...) __attribute__((format(printf, 1, 2)));                        
extern void init_logger();
extern void stop_logger();