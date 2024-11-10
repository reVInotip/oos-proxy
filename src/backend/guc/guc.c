/**
 * TODO
 *  Split analize_config_string function with read_config_string function
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "logger/logger.h"
#include "utils/hash_map.h"
#include "utils/stack.h"
#include "guc/guc.h"

static Hash_map_ptr map;
static Stack_ptr alloc_mem_stack;

/**
 * \brief Type of config string (key = value)
 */
typedef struct conf_string
{
    char key[MAX_CONFIG_KEY_SIZE];
    Guc_data *value;
    uint64_t count_values;
} Conf_string;

/**
 * \brief Analize config raw string: extract from it key and value,
 * check if string is invalid, discards comments or empty strings
 * \param [in] conf_str - Raw config string
 * \param [out] converted_conf_str - key and value extracted from string
 * \param [out] string_is_incorrect - Is string incorrect (or comment)?
 * \return Type of config variable
 * \note Memory for string interpretation will be allocated automatically, so after use you should free it
 */
Config_vartype analize_config_string(const char *conf_str, Conf_string *converted_conf_str, bool *string_is_incorrect)
{   
    assert(conf_str != NULL);
    assert(converted_conf_str != NULL);
    char *data = (char *)malloc(strlen(conf_str));
    void *arr_data = NULL;
    bool is_equal_sign = false; // Was equals sign (=) founded in string
                                // It needs for validating and separation key and value
    bool is_key_exists = false;
    bool is_value_exists = false;
    bool value_is_digit = true;
    bool value_is_double = true;
    bool value_is_array = false;
    bool value_is_string = false;

    bool is_close_bracket = false;
    bool is_close_quote = false;
    Config_vartype array_type = UNINIT;
    int k = 0;
    converted_conf_str->count_values = 0;
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
                *string_is_incorrect = true;
                goto ERROR_EXIT;
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
            if (!is_key_exists)
            {
                write_stderr("Error in config string: %s - Key does not exists\n", conf_str);
                goto ERROR_EXIT;
            }
            *string_is_incorrect = false;

            data[k] = conf_str[i];
            ++k;

            if (is_close_bracket)
            {
                write_stderr("Error in config string: %s - Wrong value\n", conf_str);
                *string_is_incorrect = true;
                goto ERROR_EXIT;
            }
            
            if (is_close_quote && !value_is_array)
            {
                write_stderr("Error in config string: %s - Wrong value\n", conf_str);
                *string_is_incorrect = true;
                goto ERROR_EXIT;
            }

            if (conf_str[i] < '0' || conf_str[i] > '9')
            {
                value_is_digit = false;
            }

            if ((conf_str[i] < '0' || conf_str[i] > '9') && conf_str[i] != '.')
            {
                value_is_double = false;
            }

            if (conf_str[i] == '[' && k == 1)
            {
                value_is_array = true;
                value_is_digit = true;
                value_is_double = true;
                k--;
            }

            if (conf_str[i] == '"')
            {
                k--;
                if (k == 0)
                    value_is_string = true;
                else if (value_is_array)
                {
                    is_close_quote = true;
                    continue;
                }
                else
                {
                    is_close_quote = true;
                    break;
                }
            }

            if ((conf_str[i] == ',' || conf_str[i] == ']') && k > 1 && value_is_array)
            {
                ++converted_conf_str->count_values;
                if (value_is_digit)
                {
                    if (array_type != ARR_LONG && array_type != UNINIT)
                    {
                        write_stderr("Error in config string: %s - All elements in array should has equal type\n", conf_str);
                        goto ERROR_EXIT;
                    }

                    arr_data = realloc(arr_data, sizeof(long) * converted_conf_str->count_values);
                    ((long *) arr_data)[converted_conf_str->count_values - 1] = atol(data);
                    k = 0;
                    array_type = ARR_LONG;
                }
                else if (value_is_double)
                {
                    if (array_type != ARR_DOUBLE && array_type != UNINIT)
                    {
                        write_stderr("Error in config string: %s - All elements in array should has equal type\n", conf_str);
                        goto ERROR_EXIT;
                    }
                    
                    arr_data = realloc(arr_data, sizeof(double) * converted_conf_str->count_values);
                    ((double *) arr_data)[converted_conf_str->count_values - 1] = atof(data);
                    k = 0;
                    array_type = ARR_DOUBLE;
                }
                else if (value_is_string)
                {
                    if (array_type != ARR_STRING && array_type != UNINIT)
                    {
                        write_stderr("Error in config string: %s - All elements in array should has equal type\n", conf_str);
                        goto ERROR_EXIT;
                    }
                    
                    arr_data = realloc(arr_data, sizeof(char *) * converted_conf_str->count_values);
                    data[k - 1] = '\0';
                    ((char **) arr_data)[converted_conf_str->count_values - 1] = data;
                    data = (char *) malloc(strlen(conf_str));
                    k = 0;
                    array_type = ARR_STRING;
                    value_is_string = false;
                }
                else
                {
                    write_stderr("Error in config string: %s - All elements in array should has equal type\n", conf_str);
                    goto ERROR_EXIT;
                }

                is_close_bracket = conf_str[i] == ']';
            }
        }
        // else we write key
        else
        {   
            *string_is_incorrect = false;
            is_key_exists = true;

            converted_conf_str->key[k] = conf_str[i];
            ++k;
        }
    }

    data[k] = '\0';

    if (!is_value_exists)
        goto ERROR_EXIT;
    
    if (!is_close_bracket && value_is_array)
    {
        write_stderr("Error in config string: %s - Array should end on close bracket\n", conf_str);
        goto ERROR_EXIT;
    }

    if (!is_close_quote && value_is_string)
    {
        write_stderr("Error in config string: %s - String should end on close quote\n", conf_str);
        goto ERROR_EXIT;
    }

    if (value_is_digit)
    {
        converted_conf_str->value->num = atol(data);
        free(data);
        return LONG;
    }
    else if (value_is_double)
    {
        converted_conf_str->value->numd = atof(data);
        free(data);
        return DOUBLE;
    }
    else if (value_is_array)
    {   
        if (array_type == ARR_LONG)
            converted_conf_str->value->arr_long.data = arr_data;
        else if (array_type == ARR_DOUBLE)
            converted_conf_str->value->arr_double.data = arr_data;
        else
            converted_conf_str->value->arr_str.data = arr_data;
        
        return array_type;
    }
    else if (value_is_string)
    {
        converted_conf_str->value->str = data;
        return STRING;
    }
    else
        write_stderr("Error in config string: %s - Can not determine value type\n", conf_str);

ERROR_EXIT:
    free(data);

    while (converted_conf_str->count_values != 0 && array_type == ARR_STRING)
    {
        --converted_conf_str->count_values;
        free(((char **) arr_data)[converted_conf_str->count_values]);
    }

    if (arr_data) free(arr_data);
    return UNINIT;
}

void destroy_guc_table()
{
    while (alloc_mem_stack)
    {
        void *pointer = pop_from_stack(&alloc_mem_stack);
        free(pointer);
    }
    destroy_map(&map);
}

bool is_eof(FILE *config)
{
    char c = fgetc(config);
    if (c == EOF)
        return true;
    
    fseek(config, -1, SEEK_CUR);
    return false;
}

bool read_config_string(FILE *config, char **conf_raw_string, size_t size)
{
    while (!is_eof(config))
    {
        if (fgets(*conf_raw_string, size, config) == NULL)
            break;

        if ((*conf_raw_string)[strlen(*conf_raw_string) - 1] != '\n')
        {   
            // If read last string (it doesn`t contains '\n') return true for analize it
            // If this not done then looping will occur (because fseek shift our position by number of reading bytes)
            if (is_eof(config))
                return true;
            
            fseek(config, (-1) * strlen(*conf_raw_string), SEEK_CUR);
            *conf_raw_string = (char *)realloc(*conf_raw_string, size * 2);
            size *= 2;
            continue;
        }
        else
            (*conf_raw_string)[strlen(*conf_raw_string) - 1] = '\0';

        return true;
    }

    if (ferror(config))
        write_stderr("Error occured while reading config file: %s", strerror(errno));

    return false;
}

void create_guc_table()
{
    map = create_map();
}

/**
 * \brief Parse configuration file and create GUC variables from config variables
 * \note If variable now exists in GUC table with C_MAIN context
 *  (variables with user context can not be in table then main process call this function) it will be ignored
 */
void parse_config()
{
    FILE *config;
    if (!is_var_exists_in_config("conf_path", C_MAIN | C_STATIC))
    {
        printf("Use default configuration file\n");
        define_custom_string_variable("conf_path", "Path to configuration file", DEFAULT_CONF_FILE_PATH, C_MAIN | C_DYNAMIC);
        config = fopen(DEFAULT_CONF_FILE_PATH, "r");
    }
    else
    {
        Guc_data conf_path = get_config_parameter("conf_path", C_MAIN | C_DYNAMIC);
        config = fopen(conf_path.str, "r");
        if (config == NULL)
        {
            conf_path.str = DEFAULT_CONF_FILE_PATH;
            set_config_parameter("conf_path", conf_path, C_MAIN | C_DYNAMIC);
            write_stderr("Can`t read user config file: %s. Try to read default config file\n", strerror(errno));
            config = fopen(DEFAULT_CONF_FILE_PATH, "r");
        }
    }

    alloc_mem_stack = create_stack();
    
    if (config == NULL)
    {  
        write_stderr("Can`t read config file: %s. Use default GUC values\n", strerror(errno));
        return;
    }

    char *conf_raw_string = (char *)malloc(MAX_CONFIG_KEY_SIZE + CONFIG_VALUE_SIZE + 2);
    Guc_variable var;
    Conf_string conf_string;
    conf_string.value = &(var.elem);
    bool string_is_incorrect = true;

    // We read the line up to the line break character and read it separately so as not to interfere
    while (read_config_string(config, &conf_raw_string, MAX_CONFIG_KEY_SIZE + CONFIG_VALUE_SIZE + 2))
    {
        memset(conf_string.key, 0, MAX_CONFIG_KEY_SIZE);

        // get key and value from config string
        Config_vartype type = analize_config_string(conf_raw_string, &conf_string, &string_is_incorrect);

        if (string_is_incorrect || type == UNINIT || is_var_exists_in_config(conf_string.key, C_MAIN))
            continue;

        if (type == ARR_LONG)
        {
            push_to_stack(&alloc_mem_stack, var.elem.arr_long.data);
            var.elem.arr_long.count_elements = conf_string.count_values;
        }
        else if (type == ARR_DOUBLE)
        {
            push_to_stack(&alloc_mem_stack, var.elem.arr_double.data);
            var.elem.arr_double.count_elements = conf_string.count_values;
        }
        else if (type == ARR_STRING)
        {
            push_to_stack(&alloc_mem_stack, var.elem.arr_str.data);
            uint64_t counter = conf_string.count_values;
            while (counter != 0)
            {
                --counter;
                push_to_stack(&alloc_mem_stack, var.elem.arr_str.data[counter]);
            }
            var.elem.arr_str.count_elements = conf_string.count_values;
        }
        else if (type == STRING)
            push_to_stack(&alloc_mem_stack, var.elem.str);

        var.vartype = type;

        // all config varibles is immutable
        var.context = C_MAIN | C_STATIC;
        strcpy(var.name, conf_string.key);
        strcpy(var.descripton, STANDART_DESCRIPTION);

        push_to_map(map, conf_string.key, &var, sizeof(Guc_variable));
    }

    free(conf_raw_string);

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
void define_custom_long_variable(
    char *name,
    const char *descr,
    const long boot_value,
    const int8_t context)
{
    Guc_variable var;
    strcpy(var.name, name);
    strcpy(var.descripton, descr);
    var.elem.num = boot_value;
    var.context = context;
    var.vartype = LONG;

    push_to_map(map, name, &var, sizeof(Guc_variable));
}

/**
 * \brief Define your own GUC string varible
 * \param [in] name - name of new variable
 * \param [in] descr - description of new variable
 * \param [in] boot_value - initial value of variable
 * \param [in] context - restriction on variable use
 * \return nothing
 */
void define_custom_string_variable(
    char *name,
    const char *descr,
    const char *boot_value,
    const int8_t context)
{
    Guc_variable var;
    strcpy(var.name, name);
    strcpy(var.descripton, descr);
    var.elem.str = (char *) malloc(strlen(boot_value) + 1);
    strcpy(var.elem.str, boot_value);
    var.context = context;
    var.vartype = STRING;

    push_to_map(map, name, &var, sizeof(Guc_variable));
    push_to_stack(&alloc_mem_stack, var.elem.str);
}

/**
 * \brief Get GUC variable by its name
 */
Guc_data get_config_parameter(const char *name, const int8_t context)
{
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);
    Guc_data err_data = {0};
    
    if (var == NULL)
    {
        write_stderr("Config variable %s does not exists\n", name);
        return err_data;
    }

    if (!equals(get_identify(var->context), get_identify(context)))
    {
        elog(ERROR, "Try to get variable with different context");
        return err_data;
    }

    return var->elem;
}

/**
 * \brief Get GUC variable by its name
 */
Config_vartype get_config_parameter_type(const char *name, const int8_t context)
{
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);
    
    if (var == NULL)
    {
        write_stderr("Config variable %s does not exists\n", name);
        return UNINIT;
    }

    if (!equals(get_identify(var->context), get_identify(context)))
    {
        elog(ERROR, "Try to get variable with different context");
        return UNINIT;
    }

    return var->vartype;
}

/**
 * \brief Set new value to GUC variable (only if context allows)
 */
void set_config_parameter(const char *name, const Guc_data data, const int8_t context)
{
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL)
    {
        elog(ERROR, "Config variable %s does not exists", name);
    }

    if (!is_dynamic(var->context))
    {
        elog(ERROR, "Try to change static config variable");
        return;
    }

    if (!equals(get_identify(var->context), get_identify(context)))
    {
        elog(ERROR, "Try to change variable with different context");
        return;
    }

    var->elem.num = data.num;
    strcpy(var->elem.str, data.str);
}

/**
 * \brief Check if variable with name "name" exists in GUC table
 * \return true if variable with name "name" exists in GUC table and it`s context equals with param context,
 *         false if not
 */
bool is_var_exists_in_config(const char *name, const int8_t context)
{
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL || !equals(get_identify(var->context), get_identify(context)))
        return false;

    return true;
}
