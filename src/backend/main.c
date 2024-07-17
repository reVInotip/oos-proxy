#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <libconfig.h>
#include <string.h>
#include <assert.h>
#include "../include/shlib_operations/lib_stack.h"
#include "../include/shlib_operations/loader.h"
#include "../include/logger/logger.h"

/*
descr: Initialize all extensions
*/
void init_all_exetensions(Lib_stack_ptr lib_stack)
{
    void (*function)() = dlsym(top(lib_stack), "init");
    function();
}

/*
descr: Move this to config
notes: Later this absolute path can be replaced on relative by LD_LIBRARY_PATH variable (when will they study them)
*/
char *base_dir = "/home/grisha/Projects/oos-proxy/src/backend/contrib"; // enter your path

int main(int argc, char *argv[]) {
    //read_config
    init_logger();
    elog(INFO, "Logger inited successfully");

    Lib_stack_ptr lib_stack = create_stack();
    loader(&lib_stack, base_dir);

    if (get_size(lib_stack) > 0)
    {
        init_all_exetensions(lib_stack);
    }
    else
    {
        elog(WARN, "No extensions have been downloaded");
    }

    stop_logger();
}