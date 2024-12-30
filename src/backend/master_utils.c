#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "utils/stack.h"
#include "shlib_operations/operations.h"
#include "logger/logger.h"
#include "guc/guc.h"
#include "memory/allocator.h"
#include "memory/cache.h"
#include "bg_worker/bg_worker.h"
#include "master.h"
#include "master_utils.h"

/**
 * \brief Parse command line arguments and add it to GUC
 * \param [in] argc - count of elements in argv
 * \param [in] argv - array of command line arguments with flags and values
 * \return 0 if all is OK, 1 if argument is invalid
 */
int parse_command_line(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "c:l:")) != -1)
    {
        switch (c)
        {
            case 'c':
                define_custom_string_variable("conf_path", "Path to configuration file", optarg, C_MAIN | C_DYNAMIC);
                break;
            case 'l':
                define_custom_string_variable("log_dir", "Path to directory with logs", optarg, C_MAIN | C_STATIC);
                break;
            default:
                write_stderr("Unknow option: %c\n", c);
                return 1;
        }
    }

    return 0;
}

/**
 * \brief Graceful shutdown all processes in the programm
 * \param [in] exit_code Code, which will be returned by main process (if it less than zero
 *  the function will ignore it and will not complete main process)
 */
void shutdown(int exit_code)
{
    drop_all_workers();
    drop_cache();
    destroy_guc_table();
    close_all_exetensions();
    stop_logger();

    if (exit_code < 0)
        return;
    
    exit(exit_code);
}
