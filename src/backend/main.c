/**
 * The main proccess (the Boss)
 */

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
#include "boss_operations/hook.h"

Hook executor_start_hook = NULL;
Hook executor_end_hook = NULL;

extern void hello_from_static_lib(void);
extern void hello_from_dynamic_lib(void);

void test_alloc()
{
    print_alloc_mem();
    int *a = OOS_allocate(4 * sizeof(int));
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;
    printf("%d\n", a[3]);
    print_alloc_mem();

    int *b = OOS_allocate(10 * sizeof(int));
    b[9] = 11;
    printf("%d\n", b[9]);
    print_alloc_mem();

    int *c = OOS_allocate(5 * sizeof(int));
    c[4] = 50;
    printf("%d\n", c[4]);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(b);
    print_alloc_mem();

    int *d = OOS_allocate(15 * sizeof(int));
    d[4] = 50;
    printf("%d\n", d[4]);
    print_alloc_mem();

    int *e = OOS_allocate(8 * sizeof(int));
    e[1] = 50;
    printf("%d\n", e[1]);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(d);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(c);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(a);
    print_alloc_mem();

    printf("===================================================\n");
    OOS_free(e);
    print_alloc_mem();
}

void test_cache()
{   
    char buffer[13];
    cache_write("aaaaaa", "Hello world\n", 13, 100000000);
    cache_read("aaaaaa", buffer, 13);
    printf("%s", buffer);
}

int parse_command_line(int argc, char *argv[], char **conf_path)
{
    int c;
    while ((c = getopt(argc, argv, "c:")) != -1)
    {
        switch (c)
        {
            case 'c':
                strcpy(*conf_path, optarg);
                break;
            default:
                write_stderr("Unknow option: %c\n", c);
                return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char *conf_path = NULL;
    if (parse_command_line(argc, argv, &conf_path) != 0)
        return 1;
    
    hello_from_static_lib();
    hello_from_dynamic_lib();
    parse_config(conf_path);
    init_logger();
    init_cache();
    //test_cache();
    elog(LOG, "Logger inited successfully");

    Guc_data base_dir = get_config_parameter("base_dir", C_MAIN);

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

    sleep(3);

    drop_all_workers();
    drop_cache();
    destroy_guc_table();
    close_all_exetensions(lib_stack);
    stop_logger();

    return 0;
}