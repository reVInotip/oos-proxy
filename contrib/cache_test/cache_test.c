#include "boss_operations/boss_operations.h"
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

extern void init(void *arg, ...)
{
    Boss_op_func *func = (Boss_op_func *) arg;

    FILE *proc = fopen("/proc/self/maps", "r");
    if (proc == NULL)
    {
        printf("Can`t open proc\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    int size = fread(buffer, sizeof(char), BUFFER_SIZE, proc);
    if (size == EOF)
    {
        printf("Can`t read proc\n");
        return;
    }

    func->cache_write_op("bbbbb", buffer, BUFFER_SIZE, 10);
    func->print_cache_op("bbbbb");
    
    fclose(proc);
}
