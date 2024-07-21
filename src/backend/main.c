/**
 * The main proccess (the Boss)
 */

#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include <assert.h>
#include "../include/utils/stack.h"
#include "../include/shlib_operations/operations.h"
#include "../include/logger/logger.h"
#include "../include/guc/guc.h"

int main(int argc, char *argv[]) {
    parse_config();
    init_logger();
    elog(INFO, "Logger inited successfully");

    Guc_data base_dir = get_config_parameter("base_dir");

    Stack_ptr lib_stack = create_stack();
    loader(&lib_stack, base_dir.str);

    if (get_stack_size(lib_stack) > 0)
    {
        init_all_exetensions(lib_stack);
    }
    else
    {
        elog(WARN, "No extensions have been downloaded");
    }

    close_all_exetensions(lib_stack);
    stop_logger();

    return 0;
}