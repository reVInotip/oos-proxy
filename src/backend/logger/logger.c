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
#include <stdlib.h>
#include <stdarg.h>
#include "../../include/logger/logger.h"
#include "../../include/guc/guc.h"

// Size of buffer with current time
#define TIME_BUFFER_SIZE 100

// Size of buffer with current date
#define DATE_BUFFER_SIZE 50

// Size of buffer with timezone name
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

// This struct describe log file
typedef struct log_file {
    FILE *file; // Log file that is open now
    char log_file_name[2][MAX_CONFIG_VALUE_SIZE]; // Names of both log files
    int curr_log_file; // Index of current log file name
} Log_file;

Log_file *log_file;

// Close current log file and rewrite new log file
inline void swap_log_files()
{   
    fclose(log_file->file);

    log_file->curr_log_file = (log_file->curr_log_file + 1) % 2;

    log_file->file = fopen(log_file->log_file_name[log_file->curr_log_file], "w");

    if (log_file->file == NULL)
    {
        perror(strerror(errno));
        return;
    }
}

// Get string representation of E_LEVEL
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
 * \return -1 if file_number is wrong. 1 if file size excced capacity of log files
 * 0 if file is not full.
 */
int log_file_full(int file_number)
{   
    if (file_number != 0 && file_number != 1)
    {
        return -1;
    }

    struct stat file_stat;
    stat(log_file->log_file_name[file_number], &file_stat);

    Guc_data log_cap = get_config_parameter("log_capacity");
    if (file_stat.st_size >= log_cap.num)
    {
        return 1;
    }

    return 0;
}

/**
 * \brief Write something into .log file
 * \param [in] level - log level
 * \param [in] filename - name of file where the error occurred
 * \param [in] line_number - line where the error occurred
 * \param [in] format - formated string
 * \return nothing
 */
extern void write_log(E_LEVEL level, const char *filename, int line_number, const char *format, ...)
{   
    time_t log_time = time(NULL);
    struct tm *now = localtime(&log_time);
    
    char time[TIME_BUFFER_SIZE] = {'\0'};
    strftime(time, sizeof(time), "%T", now);

    char date[DATE_BUFFER_SIZE] = {'\0'};
    strftime(date, sizeof(time), "%D", now);

    char offset[OFFSET_BUFFER_SIZE] = {'\0'};
    strftime(offset, sizeof(time), "%Z", now);

    int curr_log_file_full = log_file_full(log_file->curr_log_file);

    if (curr_log_file_full > 0)
    {
        swap_log_files();
    }

    va_list args;
    va_start(args, format);

    if (level == INFO)
    {
        Guc_data info_in_log = get_config_parameter("info_in_log");
        if (!info_in_log.num)
        {
            write_stderr(format, args);
            return;
        }
    }

    fprintf(log_file->file, "%s %s %s | %s: \"", date, time, offset,
        get_str_elevel(level));
    vfprintf(log_file->file, format, args);
    fprintf(log_file->file, "\" in file: %s, line: %d\n", filename, line_number);
    fflush(log_file->file);
    
    va_end(args);
}

/**
 * \brief Write something into stderr (if log file was not initialized or
 *          error level is INFO)
 * \param [in] format - formated string
 * \return nothing
 */
extern void write_stderr(const char *format, ...)
{
    va_list	args;

	va_start(args, format);

	vfprintf(stderr, format, args);
	fflush(stderr);

    va_end(args);
}

/**
 * \brief Initialize logger
 * \return nothing
 */
extern void init_logger()
{
    if (close(STDERR_FILENO) < 0)
    {
        perror(strerror(errno));
        return;
    }

    log_file = (Log_file *) malloc(sizeof(Log_file));

    bool is_log_dir_exists = false;
    Guc_data log_dir_name = get_config_parameter("log_dir_name");
    Guc_data log1_file_name = get_config_parameter("log1_file_name");
    Guc_data log2_file_name = get_config_parameter("log2_file_name");
    
    DIR *source = opendir(".");
    struct dirent* entry;

    // Check if log directory alredy exists
    // Add checking if log files exists
    while ((entry = readdir(source)) != NULL)
    {
        if (!strcmp(entry->d_name, log_dir_name.str) && entry->d_type == DT_DIR)
        {
            is_log_dir_exists = true;
            break;
        }
    }

    log_file->curr_log_file = 0;

    strncpy(log_file->log_file_name[0], log1_file_name.str, sizeof(log1_file_name.str));
    strncpy(log_file->log_file_name[1], log2_file_name.str, sizeof(log2_file_name.str));

    bool is_log_file_full[2] = {false, false};

    if (is_log_dir_exists)
    {  
        source = opendir(log_dir_name.str);

        is_log_file_full[0] = log_file_full(0) > 0;
        is_log_file_full[1] = log_file_full(1) > 0;
        
        // we should check if second log file is not full
        /*if (is_log_file_full[0] && !is_log_file_full[1])
        {
            log_file->curr_log_file = 1;
        }*/
    }
    else
    {   
        if (mkdir("log", S_IRWXU) < 0)
        {
            perror(strerror(errno));
            return;
        }

        log_file->file = fopen(log1_file_name.str, "a");
    }

    if (is_log_file_full[log_file->curr_log_file])
    {
        log_file->file = fopen(log_file->log_file_name[log_file->curr_log_file], "w");
    }
    else
    {
        log_file->file = fopen(log_file->log_file_name[log_file->curr_log_file], "a");
    }
}

/**
 * \brief Stop write logs
 */
extern void stop_logger()
{
    fclose(log_file->file);
    free(log_file);
}