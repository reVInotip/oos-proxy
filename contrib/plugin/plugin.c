/**
 * Example of plugin for lab1
 */
#include <stdio.h>
#include "boss_operations/hook.h"

static Hook prev_executor_start_hook = NULL;
static Hook prev_executor_end_hook = NULL;

void then_start() {
    if(prev_executor_start_hook) {
        prev_executor_start_hook();
    }
    printf("hello from then_start()\n");
}

void then_end() {
    if(prev_executor_end_hook) {
        prev_executor_end_hook();
    }
    printf("hello from then_end()\n");
}

void init(void) {
    prev_executor_start_hook = executor_start_hook;
    executor_start_hook = then_start;

    prev_executor_end_hook = executor_end_hook;
    executor_end_hook = then_end;
    
    printf("init successfully!\n");
}