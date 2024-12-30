#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "boss_operations.h"
#include "guc/guc.h"
#include "master.h"
#include "logger/logger.h"

#define LOG_BACKUP_DIR_NAME "log_backup"

// if system macros is __USE_MUSC undefined
#ifndef __USE_MISC
enum
  {
    DT_REG = 8,
# define DT_REG		DT_REG
    DT_DIR = 4,
# define DT_DIR		DT_DIR
  };
#endif

static Hook prev_executor_start_hook = NULL;
static Hook prev_executor_end_hook = NULL;

char *log_path;
char *log_backup_path;

void then_start() {
    if(prev_executor_start_hook) {
        prev_executor_start_hook();
    }
    
    DIR *source = opendir(log_path);
    struct dirent* entry;

    size_t source_len = strlen(log_path) + 1;
    size_t dest_len = strlen(log_backup_path) + 1;

    while ((entry = readdir(source)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;

        char *source_name = (char *) malloc(source_len + strlen(entry->d_name) + 1);
        strcpy(source_name, log_path);
        source_name[source_len - 1] = '/';
        strcpy(&source_name[source_len], entry->d_name);

        char *dest_name = (char *) malloc(dest_len + strlen(entry->d_name) + 1);
        strcpy(dest_name, log_backup_path);
        dest_name[dest_len - 1] = '/';
        strcpy(&dest_name[dest_len], entry->d_name);

        if (access(dest_name, F_OK) == 0)
        {
            if (unlink(dest_name) < 0)
                elog(WARN, "Can not delete previous backup of log file: %s", source_name);
        }

        if (link(source_name, dest_name) < 0)
            elog(WARN, "Can not make backup for file: %s", source_name);

        free(source_name);
        free(dest_name);
    }

    closedir(source);
}

void then_end() {
    if(prev_executor_end_hook) {
        prev_executor_end_hook();
    }
    
    free(log_path);
    free(log_backup_path);
}


void init(void *arg, ...) {
    prev_executor_start_hook = executor_start_hook;
    executor_start_hook = then_start;

    prev_executor_end_hook = executor_end_hook;
    executor_end_hook = then_end;

    char *log_dir = get_config_string_parameter_op("log_dir", C_MAIN);
    assert(log_dir != NULL);

    /*log_path = (char *) malloc(strlen(log_dir) + 7); // "../../...\0"
    assert(log_path != NULL);
    strcpy(log_path, "../../");
    strcpy(&log_path[7], log_dir);*/

    log_path = (char *) malloc(strlen(log_dir) + 1);
    strcpy(log_path, log_dir);

    int last_dash_index = 0;
    for (int i = 0; log_dir[i] != '\0'; ++i) {
        if (log_dir[i] == '/') last_dash_index = i;
    }

    /*log_backup_path = (char *) malloc(last_dash_index + strlen(LOG_BACKUP_DIR_NAME) + 8); // "../../...\0"

    strcpy(log_backup_path, "../../");
    strncpy(&log_backup_path[7], log_dir, last_dash_index + 1);
    strcpy(&log_backup_path[last_dash_index + 8], LOG_BACKUP_DIR_NAME);*/

    log_backup_path = (char *) malloc(last_dash_index + strlen(LOG_BACKUP_DIR_NAME) + 2);
    if (last_dash_index == 0)
        strcpy(log_backup_path, LOG_BACKUP_DIR_NAME);
    else
    {
        strncpy(log_backup_path, log_dir, last_dash_index + 1);
        strcpy(&log_backup_path[last_dash_index + 1], LOG_BACKUP_DIR_NAME);
    }

    if (access(log_backup_path, F_OK) != 0)
    {
        if (mkdir(log_backup_path, S_IRWXU) < 0)
        {
            elog(WARN, "Can not make log dirrectory: %s", strerror(errno));
            return;
        }
    }

    elog(LOG, "Path to log backup: %s", log_backup_path);
}