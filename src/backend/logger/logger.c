#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include "../../include/logger/logger.h"
#include "../../include/guc/guc.h"

// move it into config
//#define LOG_CAP 10 //8 * 1024
//#define LOG1_FILE_NAME "log/oos_proxy_1.log"
//#define LOG2_FILE_NAME "log/oos_proxy_2.log"
//#define LOG_DIR_NAME "log"
//#define INFO_IN_LOG 1
#define TIME_BUFFER_SIZE 100
#define DATE_BUFFER_SIZE 50
#define OFFSET_BUFFER_SIZE 5

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

typedef struct log_file {
    FILE *file;
    char log_file_name[2][MAX_CONFIG_VALUE_SIZE];
    int curr_log_file;
} Log_file;

Log_file log_file;

inline void swap_log_files()
{   
    fclose(log_file.file);

    log_file.curr_log_file = (log_file.curr_log_file + 1) % 2;

    log_file.file = fopen(log_file.log_file_name[log_file.curr_log_file], "w");

    if (log_file.file == NULL)
    {
        perror(strerror(errno));
        return;
    }
}

char *get_str_elevel(E_LEVEL level)
{
    switch(level)
    {
        case INFO:
            return "INFO";
        case DEBUG:
            return "DEBUG";
        case LOG:
            return "LOG";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
    }

    return NULL;
}

/**
 * \brief Check is log file full
 */
int log_file_full(int file_number)
{   
    if (file_number != 0 && file_number != 1)
    {
        return -1;
    }

    struct stat file_stat;
    stat(log_file.log_file_name[file_number], &file_stat);

    Guc_data log_cap = get_config_parameter("log_capacity");
    if (file_stat.st_size >= log_cap.num)
    {
        return 1;
    }

    return 0;
}

extern void elog(E_LEVEL level, const char *format, ...)
{   
    time_t log_time = time(NULL);
    struct tm *now = localtime(&log_time);
    
    char time[TIME_BUFFER_SIZE] = {'\0'};
    strftime(time, sizeof(time), "%T", now);

    char date[DATE_BUFFER_SIZE] = {'\0'};
    strftime(date, sizeof(time), "%D", now);

    char offset[OFFSET_BUFFER_SIZE] = {'\0'};
    strftime(offset, sizeof(time), "%Z", now);

    int curr_log_file_full = log_file_full(log_file.curr_log_file);

    if (curr_log_file_full > 0)
    {
        swap_log_files();
    }

    if (level == INFO)
    {
        Guc_data info_in_log = get_config_parameter("info_in_log");
        if (!info_in_log.num)
        {
            return;
        }
    }

    fprintf(log_file.file, "%s %s %s: %s %s%c", date, time, offset,
        get_str_elevel(level), format, '\n');
}

extern void init_logger()
{
    if (close(STDERR_FILENO) < 0)
    {
        perror(strerror(errno));
        return;
    }

    bool is_log_dir_exists = false;
    Guc_data log_dir_name = get_config_parameter("log_dir_name");
    Guc_data log1_file_name = get_config_parameter("log1_file_name");
    Guc_data log2_file_name = get_config_parameter("log2_file_name");
    
    DIR *source = opendir(".");
    struct dirent* entry;

    while ((entry = readdir(source)) != NULL)
    {
        if (!strcmp(entry->d_name, log_dir_name.str) && entry->d_type == DT_DIR)
        {
            is_log_dir_exists = true;
            break;
        }
    }

    log_file.curr_log_file = 0;

    strcpy(log_file.log_file_name[0], log1_file_name.str);
    strcpy(log_file.log_file_name[1], log2_file_name.str);

    bool is_log_file_full[2] = {false, false};

    if (is_log_dir_exists)
    {  
        source = opendir(log_dir_name.str);

        is_log_file_full[0] = log_file_full(0) > 0;
        is_log_file_full[1] = log_file_full(1) > 0;
        
        if (is_log_file_full[0] && !is_log_file_full[1])
        {
            log_file.curr_log_file = 1;
        }
    }
    else
    {   
        if (mkdir("log", S_IRWXU) < 0)
        {
            perror(strerror(errno));
            return;
        }

        log_file.file = fopen(log1_file_name.str, "a");
    }

    if (is_log_file_full[log_file.curr_log_file])
    {
        log_file.file = fopen(log_file.log_file_name[log_file.curr_log_file], "w");
    }
    else
    {
        log_file.file = fopen(log_file.log_file_name[log_file.curr_log_file], "a");
    }
}

extern void stop_logger()
{
    fclose(log_file.file);
}