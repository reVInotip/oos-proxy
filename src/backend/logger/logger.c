#define _GNU_SOURCE

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
#include <assert.h>
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
    FILE           *stream; // Log file that is open now
    char *log_file_name[2]; // Names of both log files
    int      curr_log_file; // Index of current log file name
} Log_file;

Log_file *log_file;

// Close current log file and rewrite new log file
inline void swap_log_files()
{   
    fflush(log_file->stream);
    if (fclose(log_file->stream) == EOF)
    {
        write_stderr("Can not close current log file: %s", strerror(errno));
        return;
    }

    log_file->curr_log_file = (log_file->curr_log_file + 1) % 2;

    log_file->stream = fopen(log_file->log_file_name[log_file->curr_log_file], "w");
    if (log_file->stream == NULL)
    {
        write_stderr("Can not open log file: %s", strerror(errno));
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
 * \brief Check is log file complete
 * \return -1 if file_number is wrong or error occurred. 1 if file size excced capacity of log files
 * 0 if file is not complete.
 */
int log_file_complete(int file_number)
{   
    if (file_number != 0 && file_number != 1)
    {
        return -1;
    }

    struct stat file_stat;
    if (stat(log_file->log_file_name[file_number], &file_stat) == -1)
    {
        if (errno == ENOENT || errno == EFAULT) // file not exists
        {
            return 1;
        }
        write_stderr("Error while getting information about log file: %s", strerror(errno));
        return -1;
    }

    Guc_data log_cap = get_config_parameter("log_capacity", C_MAIN);
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

    int curr_log_file_complete = log_file_complete(log_file->curr_log_file);

    if (curr_log_file_complete > 0)
    {
        swap_log_files();
    }

    va_list args;
    va_start(args, format);

    if (level == INFO)
    {
        Guc_data info_in_log = get_config_parameter("info_in_log", C_MAIN);
        if (!info_in_log.num)
        {
            write_stderr(format, args);
            return;
        }
    }

    char *format_str;
    if (vasprintf(&format_str, format, args) < 0)
    {  
        write_stderr(format, args);
        write_stderr("Something went wrong then writing log file!");
        return;
    }

    fprintf(log_file->stream, "%s %s %s | [%d] %s: \"%s\" in file: %s, line: %d\n", date, time,
        offset, getpid(), get_str_elevel(level), format_str, filename, line_number);
    fflush(log_file->stream);

    free(format_str);
    
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
    Guc_data log_dir_name = get_config_parameter("log_dir_name", C_MAIN);
    Guc_data log_file_names = get_config_parameter("log_file_names", C_MAIN);
    
    if (log_file_names.arr_str.count_elements != 2)
    {
        free(log_file);
        write_stderr("Wrong count of .log files (expected 2, but got %d)", log_file_names.arr_str.count_elements);
    }

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

    log_file->log_file_name[0] = log_file_names.arr_str.data[0];
    log_file->log_file_name[1] = log_file_names.arr_str.data[1];

    bool is_log_file_complete[2] = {false, false};

    closedir(source);

    if (is_log_dir_exists)
    {  
        source = opendir(log_dir_name.str);

        int is_complete1 = log_file_complete(0);
        int is_complete2 = log_file_complete(1);
        if (is_complete1 < 0 || is_complete2 < 0)
        {
            free(log_file);
            write_stderr("Error occurred while checking is log files complete or not: %s",
                        strerror(errno));
            return;
        }

        is_log_file_complete[0] = is_complete1 > 0;
        is_log_file_complete[1] = is_complete2 > 0;

        closedir(source);

        // Choose one of two files for open
        if (is_log_file_complete[(log_file->curr_log_file + 1) % 2])
        {
            // If the second file is complete, open the first one
            if (is_log_file_complete[log_file->curr_log_file])
            {
                // If first file also complete open it in write mode
                log_file->stream = fopen(log_file->log_file_name[log_file->curr_log_file], "w");
                if (log_file->stream == NULL)
                {
                    free(log_file);
                    write_stderr("Can not open log file: %s", strerror(errno));
                    return;
                }
            }
            else
            {
                // Else open it in append mode
                log_file->stream = fopen(log_file->log_file_name[log_file->curr_log_file], "a");
                if (log_file->stream == NULL)
                {
                    free(log_file);
                    write_stderr("Can not open log file: %s", strerror(errno));
                    return;
                }
            }
        }
        else
        {
            // If second file not complete and first file not set current log file to second log file
            if (is_log_file_complete[log_file->curr_log_file])
            {
                log_file->curr_log_file = (log_file->curr_log_file + 1) % 2;
            }

            // And open it
            log_file->stream = fopen(log_file->log_file_name[log_file->curr_log_file], "a");
            if (log_file->stream == NULL)
            {
                free(log_file);
                write_stderr("Can not open log file: %s", strerror(errno));
                return;
            }
        }
    }
    else
    {   
        if (mkdir("log", S_IRWXU) < 0)
        {
            free(log_file);
            write_stderr("Can not make log dirrectory: %s", strerror(errno));
            return;
        }

        log_file->stream = fopen(log_file->log_file_name[log_file->curr_log_file], "a");
        if (log_file->stream == NULL)
        {
            free(log_file);
            write_stderr("Can not open log file: %s", strerror(errno));
            return;
        }
    }
}

/**
 * \brief Stop write logs
 */
extern void stop_logger()
{
    if (fclose(log_file->stream) == EOF)
    {
        write_stderr("Can not close current log file: %s", strerror(errno));
        return;
    }
    free(log_file);
}