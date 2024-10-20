/**
 * hook.h
 * This file contains declarations of hooks and .
 */

typedef void (*Hook)(void);

extern Hook executor_start_hook;
extern Hook executor_end_hook;