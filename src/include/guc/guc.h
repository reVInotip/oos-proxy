#pragma once

#define MAX_CONFIG_VALUE_SIZE 300
#define MAX_CONFIG_KEY_SIZE 100
#define CONF_FILE_NAME "oos_proxy.conf"
#define STANDART_DESCRIPTION "this is a config variable"
#define MAX_DESCRIPTION_LENGTH 256
#define MAX_NAME_LENGTH 50

/**
 * \brief Contains the value of global variable
 */
typedef union guc_data
{
    char str[MAX_CONFIG_VALUE_SIZE];
    long num;
} Guc_data;

/**
 * \brief C_STATIC - variable can not be changed after it was initialize
 *        C_DYNAMIC - variable be changed after it was initialize
 */
typedef enum
{
    C_STATIC,
    C_DYNAMIC
} Guc_context;

/**
 * \brief Configuration file can contains only long or string variables
 */
typedef enum
{
    LONG,
    STRING
} Config_vartype;

typedef struct guc_variable
{
    Guc_data elem;
    Guc_context context;
    Config_vartype vartype;
    char descripton[MAX_DESCRIPTION_LENGTH];
    char name[MAX_NAME_LENGTH];
} Guc_variable;

extern void destroy_guc_table();
extern void parse_config();
extern void define_custom_long_variable(
    const char *name,
    const char *descr,
    const long boot_value,
    const Guc_context context);
extern void define_custom_string_variable(
    const char *name,
    const char *descr,
    const char *boot_value,
    const Guc_context context);
extern Guc_data get_config_parameter(const char *name);
extern void set_config_parameter(const char *name, const Guc_data data);
