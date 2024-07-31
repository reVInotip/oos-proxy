#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include "../../include/boss_operations/boss_operations.h"
#include "../../include/logger/logger.h"
#include "../../include/utils/stack.h"
#include "../../include/bg_worker/bg_worker.h"

#define STACK_SIZE_FOR_BGWORKER 4096

typedef struct workers_list_elem
{
    BGWorker                 *data;
    struct workers_list_elem *next;
    struct workers_list_elem *prev;
} Workers_list_elem;

typedef Workers_list_elem *Workers_list_ptr;

Workers_list_ptr bg_workers_list = NULL;

// ======== auxiliary structure methods ===========

static Workers_list_elem *insert_to_list(Workers_list_ptr *list, BGWorker *data)
{   
    Workers_list_elem *new = (Workers_list_elem *) malloc(sizeof(Workers_list_elem));
    assert(new != NULL);

    new->data = data;
    new->next = NULL;

    if (*list == NULL)
    {
        new->prev = NULL;
        *list = new;
    }
    else
    {
        Workers_list_elem *last_list_elem = *list;

        while (last_list_elem->next != NULL)
        {
            ++last_list_elem;
        }

        new->prev = last_list_elem;
        last_list_elem->next = new;
    }

    return new;
}

static Workers_list_elem *get_from_list(Workers_list_ptr list, int pid)
{
    while (list != NULL)
    {
        if (list->data->pid == pid)
        {
            return list;
        }

        list = list->next;
    }

    return NULL;
}

static void delete_from_list(Workers_list_elem **del_elem)
{
    if ((*del_elem)->prev != NULL)
    {
        (*del_elem)->prev->next = (*del_elem)->next;
    }

    if ((*del_elem)->next != NULL)
    {
        (*del_elem)->next->prev = (*del_elem)->prev;
    }

    free(*del_elem);

    *del_elem = NULL;
}

// ======== main methods ===========

/**
 * \brief Create new background worker
 * \param [in] extension_handle - pointer to the extension handler.
 * Needed to get a function from it that will be executed in a new process
 * \param [in] bg_worker_data - data required to create a new process (more details in
 * the description of the structure itself)
 */
extern void create_bg_worker(void *extension_handle, const BGWorker_data *bg_worker_data)
{
    void *stack = mmap(NULL, STACK_SIZE_FOR_BGWORKER, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_STACK | MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED)
    {
        elog(WARN, "Can not allocate memory for background worker stack: %s", strerror(errno));
        return;
    }

    int (*function)() = dlsym(extension_handle, bg_worker_data->callback_name);
    if (function == NULL)
    {
        elog(WARN, "Can not get handle for backrgound worker start function: %s", dlerror());
        munmap(stack, STACK_SIZE_FOR_BGWORKER);
        return;
    }

    // TO_DO add background worker struct to shared memory to record its state
    // CLONE_FILES | CLONE_IO flags needed for logger
    int pid = clone(function, (char *) stack + STACK_SIZE_FOR_BGWORKER, CLONE_FILES | CLONE_IO, NULL);
    if (pid < 0)
    {
        elog(WARN, "Can not create new process for background worker: %s", strerror(errno));
        munmap(stack, STACK_SIZE_FOR_BGWORKER);
        return;
    }

    BGWorker *bg_worker = (BGWorker *) malloc(sizeof(BGWorker));

    bg_worker->pid = pid;
    bg_worker->stack = stack;
    bg_worker->name = (char *) malloc(strlen(bg_worker_data->bg_worker_name) + 1);
    assert(bg_worker->name);
    strcpy(bg_worker->name, bg_worker_data->bg_worker_name);

    insert_to_list(&bg_workers_list, bg_worker);

    elog(LOG, "Create new background worker: %s with pid: %d", bg_worker_data->bg_worker_name, pid);
}

/**
 * \brief Destroy all background workers
 * \details "Destroy" means kill process, unmap stack and delete it from list of backgeound workers.
 */
extern void drop_all_workers()
{   
    Workers_list_elem *curr_elem;
    while (bg_workers_list != NULL)
    {   
        kill(bg_workers_list->data->pid, SIGKILL);
        curr_elem = bg_workers_list;
        if (munmap(bg_workers_list->data->stack, STACK_SIZE_FOR_BGWORKER) < 0)
        {
            elog(ERROR, "Destroy stack for background worker: %s with pid: %d failed",
                bg_workers_list->data->name, bg_workers_list->data->pid);
        }

        free(bg_workers_list->data->name);

        bg_workers_list = bg_workers_list->next;
        delete_from_list(&curr_elem);
    }
}