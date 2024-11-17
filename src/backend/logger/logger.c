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
#include "logger/logger.h"
#include "guc/guc.h"
#include "master.h"

#ifndef USE_VDSO_TIME
#include <sys/syscall.h>
#endif

#define DEFAULT_LOG_DIR_NAME "logs"
 
#define DEFAULT_LOG_FILE_NAME "proxy.log"

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
    char  **log_file_names; // Names of log files
    int      curr_log_file; // Index of current log file name
    int    count_log_files;
} Log_file;

Log_file *log_file;
bool is_log_inited = false;

// Close current log file and rewrite new log file
void swap_log_files()
{   
    fflush(log_file->stream);
    if (fclose(log_file->stream) == EOF)
    {
        write_stderr("Can not close current log file: %s", strerror(errno));
        return;
    }

    log_file->curr_log_file = (log_file->curr_log_file + 1) % log_file->count_log_files;

    log_file->stream = fopen(log_file->log_file_names[log_file->curr_log_file], "w");
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
        case FATAL:
            return "FATAL";
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
 * \return -1 if error occurred. 1 if file size excced capacity of log files
 * 0 if file is not complete.
 */
static int log_file_complete(int file_number)
{   
    if (!is_var_exists_in_config("log_file_size_limit", C_MAIN))
        return 0;

    struct stat file_stat;
    if (stat(log_file->log_file_names[file_number], &file_stat) == -1)
    {
        if (errno == ENOENT || errno == EFAULT) // file not exists
            return 0;

        write_stderr("Error while getting information about log file: %s", strerror(errno));
        return -1;
    }

    Guc_data log_cap = get_config_parameter("log_file_size_limit", C_MAIN);
    if (file_stat.st_size >= log_cap.num * 1024 * 1024) // in Mb
        return 1;

    return 0;
}

#ifdef SIMPLE_LOGGER
/**
 * \brief Write something into .log file
 * \param [in] level - log level
 * \param [in] format - formated string
 * \return nothing
 */
extern void write_log(E_LEVEL level, const char *format, ...)
{   
    va_list args;
    va_start(args, format);

    if (!is_log_inited)
    {
        write_stderr("%s: ", get_str_elevel(level));
        write_stderr(format, args);
        write_stderr("\n");

        va_end(args);
        return;
    }

    char *format_str;
    if (vasprintf(&format_str, format, args) < 0)
    {  
        write_stderr("%s: ", get_str_elevel(level));
        write_stderr(format, args);
        write_stderr("Something went wrong then writing log file!");
        write_stderr("\n");
        return;
    }

    va_end(args);

    char *log_str;
    if (asprintf(&log_str, "%s: \"%s\"\n", get_str_elevel(level), format_str) < 0)
    {  
        write_stderr("%s: ", get_str_elevel(level));
        write_stderr(format, args);
        write_stderr("Something went wrong then writing log file!\n");
        return;
    }

    fprintf(log_file->stream, "%s", log_str);
    fflush(log_file->stream);

    free(log_str);
    free(format_str);

    if (log_file_complete(log_file->curr_log_file) > 0)
        swap_log_files();
    
    if (level == FATAL)
        shutdown(14); // TODO change it to gracefull shutdown
}
#else
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
    va_list args;
    va_start(args, format);

    if (!is_log_inited)
    {
        write_stderr("%s in file %s, line %d: ", get_str_elevel(level), filename, line_number);
        write_stderr(format, args);
        write_stderr("\n");

        va_end(args);
        return;
    }

#ifdef USE_VDSO_TIME
    time_t log_time = time(NULL);
#else
    time_t log_time = syscall(SYS_time, NULL);
#endif
    if (log_time < 0)
    {
        write_stderr("%s in file %s, line %d: ", get_str_elevel(level), filename, line_number);
        write_stderr(format, args);
        write_stderr("\nCan not get time: %s\n", strerror(errno));

        va_end(args);
        return;
    }

    struct tm *now = localtime(&log_time);
    
    char time[TIME_BUFFER_SIZE] = {'\0'};
    strftime(time, sizeof(time), "%T", now);

    char date[DATE_BUFFER_SIZE] = {'\0'};
    strftime(date, sizeof(time), "%D", now);

    char offset[OFFSET_BUFFER_SIZE] = {'\0'};
    strftime(offset, sizeof(time), "%Z", now);

    char *format_str;
    if (vasprintf(&format_str, format, args) < 0)
    {  
        write_stderr(format, args);
        write_stderr("Something went wrong then writing log file!");
        write_stderr("\n");
        return;
    }

    va_end(args);

    char *log_str;
    if
    (
        asprintf(&log_str, "%s %s %s | [%d] %s: \"%s\" in file: %s, line: %d\n", date,
        time, offset, getpid(), get_str_elevel(level), format_str, filename, line_number) < 0
    )
    {  
        write_stderr(format, args);
        write_stderr("Something went wrong then writing log file!\n");
        return;
    }

    fprintf(log_file->stream, "%s", log_str);
    fflush(log_file->stream);

    free(log_str);
    free(format_str);

    if (log_file_complete(log_file->curr_log_file) > 0)
        swap_log_files();
    
    if (level == FATAL)
        shutdown(14); // TODO change it to gracefull shutdown
}
#endif

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
void init_logger()
{
    /*if (close(STDERR_FILENO) < 0)
    {
        perror(strerror(errno));
        return;
    }*/

    if (!is_var_exists_in_config("log_dir", C_MAIN))
        define_custom_string_variable("log_dir", "Directory with logs", DEFAULT_LOG_DIR_NAME, C_MAIN | C_STATIC);

    if (!is_var_exists_in_config("log_files_count", C_MAIN))
        define_custom_long_variable("log_files_count", "Count of log files", 1, C_MAIN | C_STATIC);

    // Find directory with logs
    log_file = (Log_file *) malloc(sizeof(Log_file));
    bool is_log_dir_exists = false;

    Guc_data log_dir_name = get_config_parameter("log_dir", C_MAIN);

    DIR *source = opendir(".");
    struct dirent* entry;

    // Check if log directory alredy exists
    while ((entry = readdir(source)) != NULL)
    {
        if (!strcmp(entry->d_name, log_dir_name.str) && entry->d_type == DT_DIR)
        {
            is_log_dir_exists = true;
            break;
        }
    }

    closedir(source);

    // Create log file names
    Guc_data log_files_count = get_config_parameter("log_files_count", C_MAIN);
    if (log_files_count.num <= 0)
    {
        free(log_file);
        write_stderr("Wrong count of .log files: %ld", log_files_count.num);
        return;
    }

    log_file->count_log_files = log_files_count.num;

    log_file->log_file_names = (char **) malloc(log_files_count.num * sizeof(char *));
    assert(log_file->log_file_names != NULL);
    
    size_t log_dir_name_len = strlen(log_dir_name.str);
    size_t base_size = log_dir_name_len + strlen(DEFAULT_LOG_FILE_NAME) + 2;
    log_file->log_file_names[0] = (char *) malloc(base_size);
    assert(log_file->log_file_names[0] != NULL);

    log_file->log_file_names[0] = strcpy(log_file->log_file_names[0], log_dir_name.str);
    log_file->log_file_names[0][log_dir_name_len] = '/';
    strcpy(&(log_file->log_file_names[0][log_dir_name_len + 1]), DEFAULT_LOG_FILE_NAME);
    ++base_size;

    // i should be less than 128 (SIGNED_CHAR_MAX)
    for (long i = 1, j = 1; i < log_files_count.num; ++i)
    {
        log_file->log_file_names[i] = (char *) malloc(base_size);
        assert(log_file->log_file_names[i] != NULL);

        log_file->log_file_names[i] = strcpy(log_file->log_file_names[i], log_dir_name.str);
        log_file->log_file_names[i][log_dir_name_len] = '/';
        strcpy(&(log_file->log_file_names[i][log_dir_name_len + 1]), DEFAULT_LOG_FILE_NAME);
        log_file->log_file_names[i][base_size - 2] = '0' + (char) i;

        if (i % (10 * j) == 0)
        {
            ++base_size;
            ++j;
        }
    }

    long index_of_first_uncompleted_file = -1;
    for (long i = 0; i < log_files_count.num; ++i)
    {
        if (!log_file_complete(i))
        {
            index_of_first_uncompleted_file = i;
            break;
        }
    }

    if (is_log_dir_exists)
    {  
        if (index_of_first_uncompleted_file != -1)
        {
            log_file->stream = fopen(log_file->log_file_names[index_of_first_uncompleted_file], "a");
            if (log_file->stream == NULL)
            {
                write_stderr("Can not open log file: %s", strerror(errno));
                goto ERROR;
            }

            log_file->curr_log_file = index_of_first_uncompleted_file;
        }
        else
        {
            log_file->stream = fopen(log_file->log_file_names[0], "w");
            if (log_file->stream == NULL)
            {
                write_stderr("Can not open log file: %s", strerror(errno));
                goto ERROR;
            }

            log_file->curr_log_file = 0;
        }
    }
    else
    {   
        if (mkdir(log_dir_name.str, S_IRWXU) < 0)
        {
            write_stderr("Can not make log dirrectory: %s", strerror(errno));
            goto ERROR;
        }

        log_file->stream = fopen(log_file->log_file_names[0], "a");
        if (log_file->stream == NULL)
        {
            write_stderr("Can not open log file: %s", strerror(errno));
            goto ERROR;
        }

        log_file->curr_log_file = 0;
    }

    is_log_inited = true;
    return;

ERROR:
    for (long i = 0; i < log_files_count.num; ++i)
    {
        free(log_file->log_file_names[i]);
    }

    free(log_file->log_file_names);
    free(log_file);

    return;
}

/**
 * \brief Stop write logs
 */
extern void stop_logger()
{
    if (!is_log_inited)
        return;
    
    if (fclose(log_file->stream) == EOF)
    {
        write_stderr("Can not close current log file: %s", strerror(errno));
        return;
    }

    for (long i = 0; i < log_file->count_log_files; ++i)
    {
        free(log_file->log_file_names[i]);
    }

    free(log_file->log_file_names);
    free(log_file);
}