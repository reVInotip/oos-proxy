
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include "../../include/logger/logger.h"
#include "../../include/utils/hash_map.h"
#include "../../include/guc/guc.h"

Hash_map_ptr map;

/**
 * \brief Type of config string (key = value)
 */
typedef struct conf_string
{
    char key[MAX_CONFIG_KEY_SIZE];
    Guc_data *value;
} Conf_string;

/**
 * \brief Analize config raw string: extract from it key and value,
 * check if string is invalid, discards comments or empty strings
 * \param [in] conf_str - Raw config string
 * \param [out] converted_conf_str - key and value extracted from string
 * \param [out] value_is_digit - Is value digit or not?
 * \param [out] string_is_incorrect - Is string incorrect (or comment)?
 * \return nothing
 */
void analize_config_string(const char *conf_str, Conf_string *converted_conf_str, bool *value_is_digit, bool *string_is_incorrect)
{   
    assert(conf_str != NULL);
    assert(converted_conf_str != NULL);

    bool is_equal_sign = false; // Was equals sign (=) founded in string
                                // It needs for validating and separation key and value
    bool is_key_exists = false;
    bool is_value_exists = false;
    *value_is_digit = true;
    *string_is_incorrect = true;
    int k = 0;
    for (size_t i = 0; i < strlen(conf_str); i++)
    { 
        if (conf_str[i] == ' ')
        {
            continue;
        }
        // found comment
        else if (conf_str[i] == '#')
        {
            if (*string_is_incorrect)
            {
                return;
            }
            break;
        }
        // if we found equal sign we should start write value
        else if (conf_str[i] == '=')
        {
            is_equal_sign = true;
            converted_conf_str->key[k + 1] = '\0';
            k = 0;
            continue;
        }

        // if we found equal sign we write value
        if (is_equal_sign)
        {   
            is_value_exists = true;
            if (k >= MAX_CONFIG_VALUE_SIZE || !is_key_exists)
            {
                *string_is_incorrect = true;
                return;
            }
            *string_is_incorrect = false;

            converted_conf_str->value->str[k] = conf_str[i];
            ++k;

            if (conf_str[i] < '0' || conf_str[i] > '9')
            {
                *value_is_digit = false;
            }
        }
        // else we write key
        else
        {   
            if (k >= MAX_CONFIG_KEY_SIZE)
            {
                *string_is_incorrect = true;
                return;
            }
            *string_is_incorrect = false;
            is_key_exists = true;

            converted_conf_str->key[k] = conf_str[i];
            ++k;
        }
    }

    converted_conf_str->value->str[k] = '\0';

    if (!is_value_exists)
    {
        *string_is_incorrect = true;
        return;
    }

    if (*value_is_digit)
    {
        converted_conf_str->value->num = atol(converted_conf_str->value->str);
    }
}

extern void destroy_guc_table()
{
    destroy_map(&map);
}

/**
 * \brief Parse configuration file and create GUC variables from config variables
 */
extern void parse_config()
{
    FILE *config = fopen(CONF_FILE_NAME, "r");
    if (config == NULL)
    {   
        write_stderr("Can`t read config file. Use default GUC values\n");
        return;
    }

    map = create_map();
    char conf_raw_string[MAX_CONFIG_KEY_SIZE + MAX_CONFIG_VALUE_SIZE + 2] = {'\0'};
    Guc_variable *var;
    Conf_string conf_string;
    bool value_is_digit = true;
    bool string_is_incorrect = true;

    // We read the line up to the line break character and read it separately so as not to interfere
    while (fscanf(config, "%[^'\n'] %*['\n']", conf_raw_string) != EOF)
    {   
        var = (Guc_variable *) malloc(sizeof(Guc_variable));

        memset(conf_string.key, 0, MAX_CONFIG_KEY_SIZE);
        conf_string.value = &(var->elem);

        // get key and value from config string
        analize_config_string(conf_raw_string, &conf_string, &value_is_digit, &string_is_incorrect);

        if (string_is_incorrect)
        {
            free(var);
            continue;
        }

        if (value_is_digit)
        {
            var->vartype = LONG;
        }
        else
        {
            var->vartype = STRING;
        }

        // all config varibles is immutable
        var->context = C_STATIC;
        strcpy(var->name, conf_string.key);
        strcpy(var->descripton, STANDART_DESCRIPTION);

        push_to_map(map, conf_string.key, var);
    }

    if (fclose(config) == EOF)
    {
        write_stderr("Can not close configuration file: %s", strerror(errno));
        return;
    }
}

/**
 * \brief Define your own GUC long varible
 * \param [in] name - name of new variable
 * \param [in] descr - description of new variable
 * \param [in] boot_value - initial value of variable
 * \param [in] context - restriction on variable use
 * \return nothing
 */
extern void define_custom_long_variable(
    char *name,
    const char *descr,
    const long boot_value,
    const Guc_context context)
{
    Guc_variable *var = (Guc_variable *) malloc(sizeof(Guc_variable));
    strcpy(var->name, name);
    strcpy(var->descripton, descr);
    var->elem.num = boot_value;
    var->context = context;
    var->vartype = LONG;

    push_to_map(map, name, var);
}

/**
 * \brief Define your own GUC string varible
 * \param [in] name - name of new variable
 * \param [in] descr - description of new variable
 * \param [in] boot_value - initial value of variable
 * \param [in] context - restriction on variable use
 * \return nothing
 */
extern void define_custom_string_variable(
    char *name,
    const char *descr,
    const char *boot_value,
    const Guc_context context)
{
    Guc_variable *var = (Guc_variable *) malloc(sizeof(Guc_variable));
    strcpy(var->name, name);
    strcpy(var->descripton, descr);
    strcpy(var->elem.str, boot_value);
    var->context = context;
    var->vartype = STRING;

    push_to_map(map, name, var);
}

/**
 * \brief Get GUC variable by its name
 */
extern Guc_data get_config_parameter(const char *name)
{
    Guc_variable *var = get_map_element(map, name);
    
    if (var == NULL)
    {
        write_stderr("Config variable %s does not exists", name);
        elog(ERROR, "Config variable %s does not exists", name);
    }
    return var->elem;
}

/**
 * \brief Set new value to GUC variable (only if context allows)
 */
extern void set_config_parameter(const char *name, const Guc_data data)
{
    Guc_variable *var = get_map_element(map, name);
    if (var->context == C_STATIC)
    {
        elog(ERROR, "Try to change static config variable");
        return;
    }

    var->elem.num = data.num;
    strcpy(var->elem.str, data.str);
}
