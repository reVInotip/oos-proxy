#pragma once

#define MAX_CONFIG_VALUE_SIZE 100
#define MAX_CONFIG_KEY_SIZE 100

typedef union config_elem
{
    char str[MAX_CONFIG_VALUE_SIZE];
    long num;
} Config_elem;