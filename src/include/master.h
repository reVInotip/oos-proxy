/**
 * hook.h
 * This file contains declarations of hooks and .
 */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#pragma once

typedef void (*Hook)(void);

extern Hook executor_start_hook;
extern Hook executor_end_hook;

extern int cache_write_op
(
    const char *key, 
    const char *mesasge,
    const size_t message_length,
    const unsigned TTL
);
extern void define_custom_long_variable_op(char *name, const char *descr, const long boot_value, const int8_t context);
extern void define_custom_string_variable_op(char *name, const char *descr, const char *boot_value, const int8_t context);
extern void print_cache_op(const char *key);
extern void register_background_worker(char *callback_name, char *bg_worker_name, bool need_observer);
extern char *get_config_string_parameter_op(const char *name, const int8_t context);
