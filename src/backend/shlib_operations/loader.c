#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include "utils/stack.h"
#include "shlib_operations/operations.h"
#include "logger/logger.h"
#include "guc/guc.h"

Stack_ptr lib_stack;

// if system macros is __USE_MUSC undefined
#ifndef __USE_MISC
enum
  {
    DT_DIR = 4,
# define DT_DIR		DT_DIR
    DT_REG = 8,
# define DT_REG		DT_REG
  };
#endif


/**
 * \brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 * \return str1 + separator + str2
 */
char *create_new_string(char *str1, char *str2, char separator)
{
    size_t len1 = strlen(str1);
    size_t len = len1 + strlen(str2) + 2; // len of first string + len of second string +
                                          // + separator + end of string ('\0')
    char *new_str = (char *) malloc(len);
    memset(new_str, '\0', len);
    new_str = strcpy(new_str, str1);

    if (separator >= 0)
    {
        new_str[len1] = separator;
    }

    new_str = strcat(new_str, str2);

    return new_str;
}

/**
 * \brief Extracts "top" directory from path to source and add ".so" to end
 * \param [in] path_to_source - path from which sample is extracted
 * \param [in] sample - result of the function: directory name + .so
 */
void get_sample(char *path_to_source, char *sample)
{
    int k = 0;
    for (int i = 0; path_to_source[i] != '\0'; ++i, ++k)
    {
        if (path_to_source[i] == '/') {
            k = -1;
            continue;
        }

        sample[k] = path_to_source[i];
    }
    sample[k] = '.';
    sample[++k] = 's';
    sample[++k] = 'o';
    sample[++k] = '\0';
}

bool is_plugin_in_array(char *plug_name, Guc_data plugins)
{
    for (int i = 0; i < plugins.arr_str.count_elements; ++i)
    {
        if (!strcmp(plug_name, plugins.arr_str.data[i]))
            return true;
    }

    return false;
}

/**
 * \brief Find and load all shared libraties
 * \param [in] path_to_source - directory which contains shared libraries (.so files)
 * \param [in] curr_depth - current scan depth relative to the start directory (0 by default; this parameter need for recursion)
 * \param [in] depth - max scan depth relative to the start directory
 * \return nothing
 */
void default_loader(char *path_to_source, int curr_depth, const int depth)
{
    if (path_to_source == NULL)
    {
        elog(ERROR, "Can not load extensions because path to them is invalid!");
        return;
    }
    
    if (curr_depth > depth)
    {
        return;
    }

    if (!is_var_exists_in_config("plugins", C_MAIN))
    {
        elog(WARN, "Variable plugins does not exists in config");
        return;
    }

    Guc_data plugins = get_config_parameter("plugins", C_MAIN);

    char sample[256] = {'\0'};
    // Get sample of sahred library from path. The sample looks like: top dir from path + ".so"
    get_sample(path_to_source, sample);

    DIR *source = opendir(path_to_source);
    if (source == NULL)
    {
        elog(ERROR, "Error during opening the extensions directory (%s): %s",
            path_to_source, strerror(errno));
        
        return;
    }
    struct dirent* entry;

    // Scan current directory
    while ((entry = readdir(source)) != NULL)
    {   
        // if file is regular file and its name matches with sample for current directory
        // we load it to RAM
        if (entry->d_type == DT_REG && !strcmp(entry->d_name, sample) && is_plugin_in_array(sample, plugins))
        {   
            //create full path to sample
            char *full_name = create_new_string(path_to_source, sample, '/');

            // load library and push it to stack with libraries
            void *library = dlopen(full_name, RTLD_LAZY);
            if (library == NULL)
            {
                elog(WARN, "%s\n", dlerror());
            }
            push_to_stack(&lib_stack, library);

            free(full_name);
            break;
        }
        // if file is directory and it is not link to this or previous directory
        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            // create full path to new source directory
            char *full_path = create_new_string(path_to_source, entry->d_name, '/');

            default_loader(full_path, curr_depth + 1, depth);

            free(full_path);
        }
    }

    closedir(source);
}


/**
 * \brief Init loader (create stack for extensions)
 */
void init_loader()
{
    lib_stack = create_stack();
}
