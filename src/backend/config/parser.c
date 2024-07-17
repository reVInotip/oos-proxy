#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include "../../include/logger/logger.h"
#include "../../include/config/config.h"
#include "../../include/utils/hash_map.h"

#define CONF_FILE_NAME "oos_proxy.conf"

bool is_number(char *string)
{
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (string[i] < '0' || string[i] > '9')
        {
            return false;
        }
    }
    
    return true;
}

extern void init_config(Hash_map_ptr *map)
{
    FILE *config = fopen(CONF_FILE_NAME, "r");
    if (config == NULL)
    {
        elog(WARN, "Can`t read config file. Use default GUC values");
    }

    char conf_key[MAX_CONFIG_KEY_SIZE] = {'\0'};
    char conf_value_str[MAX_CONFIG_VALUE_SIZE] = {'\0'};
    Config_elem *config_elem;
    while (fscanf(config, "%s=%s", conf_key, conf_value_str) != EOF)
    {
        config_elem = (Config_elem *) malloc(sizeof(Config_elem));

        if (is_number(conf_value_str))
        {
            config_elem->num = atol(conf_value_str);
        }
        else
        {
            strcpy(config_elem->str, conf_value_str);
        }

        push_to_map(map, conf_key, config_elem);
    }
}

extern void destroy_config(Hash_map_ptr *map)
{
    assert(map != NULL);

    while (*map != NULL)
    {
        void *data = pop_from_map(map);
        free(data);
    }
}