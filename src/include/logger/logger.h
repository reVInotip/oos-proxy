#pragma once

/**
 * \brief Level of log messages
 */
typedef enum e_level {
    INFO = 1,
    DEBUG = 2,
    LOG = 3,
    WARN = 4,
    ERROR = 5
} E_LEVEL;

/**
 * \brief Write something into .log file
 * \param [in] level - log level
 * \param [in] format - formated string which will be writed
 * \return nothing
 */
extern void elog(E_LEVEL level, const char *format, ...) __attribute__((format (printf, 2, 3)));

/**
 * \brief Initialize logger
 * \return nothing
 */
extern void init_logger();

/**
 * \brief Stop write logs
 */
extern void stop_logger();