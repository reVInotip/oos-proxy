/**
 * guc.h
 * GUC - Grand United Configuration.
 * This file contains external declarations pertaining to Grand Unified Configuration.
 * GUC varibles is a mechanism that allows you to limit the use of global variables
 * making them safer. When the program starts, a special function reads data from the
 * configuration file and transfers it to GUC variables. After that you can use configuration
 * varibales in your programm and not be afraid that someone changes their values. Also you can
 * create your own safe global varible using a special method.
 * I implemented it using a hash map.
 */

#pragma once

// The maximum number of bytes that can contain a config value
#define MAX_CONFIG_VALUE_SIZE 300

// The maximum number of bytes that can contain a config key
#define MAX_CONFIG_KEY_SIZE 100

// Default configuration file name
#define CONF_FILE_NAME "oos_proxy.conf"

// The standart description of config varibale
#define STANDART_DESCRIPTION "this is a config variable"

// Max length of config varibale description
#define MAX_DESCRIPTION_LENGTH 256

// Max length for name of configuraion varibale
#define MAX_NAME_LENGTH 50

/**
 * \brief Contains the value of global variable
 * This value can be simultaneously interpreted as both a number and a
 * string of length MAX_CONFIG_VALUE_SIZE.
 */
typedef union guc_data
{
    char str[MAX_CONFIG_VALUE_SIZE];
    long num;
} Guc_data;

/**
 * \brief The context in which the global variable is declared.
 *          Needed to restrict access to it.
 * \details
 *        C_STATIC - variable can not be changed after it was initialize
 *        C_DYNAMIC - variable be changed after it was initialize
 */
typedef enum
{
    C_STATIC,
    C_DYNAMIC
} Guc_context;

/**
 * \brief Type of configuration variable. Now configuration file
 * can contains only long or string variables.
 */
typedef enum
{
    LONG,
    STRING
} Config_vartype;

/**
 * This structure describe Grand United Configuration dlobal varibales.
 */
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
    char *name,
    const char *descr,
    const long boot_value,
    const Guc_context context);
extern void define_custom_string_variable(
    char *name,
    const char *descr,
    const char *boot_value,
    const Guc_context context);
extern Guc_data get_config_parameter(const char *name);
extern void set_config_parameter(const char *name, const Guc_data data);
