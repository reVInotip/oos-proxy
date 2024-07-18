#pragma once

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