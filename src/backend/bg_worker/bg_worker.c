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
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/user.h>
#include "boss_operations/boss_operations.h"
#include "logger/logger.h"
#include "utils/stack.h"
#include "bg_worker/bg_worker.h"

#define STACK_SIZE_FOR_BGWORKER 4096 * 2

typedef struct bgworker_tracee
{
    int (*function) ();
    void        *stack;
} BGWorker_tracee;


typedef struct workers_list_elem
{
    BGWorker *data;
    struct workers_list_elem *next;
    struct workers_list_elem *prev;
} Workers_list_elem;

typedef Workers_list_elem *Workers_list_ptr;

Workers_list_ptr bg_workers_list = NULL;

// ======== auxiliary structure methods ===========

static Workers_list_elem *insert_to_list(Workers_list_ptr *list, BGWorker *data)
{
    Workers_list_elem *new = (Workers_list_elem *)malloc(sizeof(Workers_list_elem));
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

/*static Workers_list_elem *get_from_list(Workers_list_ptr list, int pid)
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
}*/

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

int tracer(void *tracee)
{
    BGWorker_tracee t;
    t.function = ((BGWorker_tracee *) tracee)->function;
    t.stack = ((BGWorker_tracee *) tracee)->stack;
    int pid = clone(t.function, (char *)t.stack + STACK_SIZE_FOR_BGWORKER, CLONE_FILES | CLONE_IO | SIGCHLD, NULL);
    if (pid < 0)
    {
        elog(WARN, "Can not create new process for background worker: %s", strerror(errno));
        return -1;
    }

    int status = 0;
    struct user_regs_struct state;
    long err = ptrace(PTRACE_ATTACH, pid, NULL, NULL);

    if (err < 0)
    {
        elog(WARN, "Can not attach tracer to background worker: %s", strerror(errno));
        return -1;
    }

    if (waitpid(pid, &status, 0) < 0)
    {
        elog(WARN, "Can not attach tracer to background worker: %s", strerror(errno));
        return -1;
    }

    err = ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
    if (err < 0)
    {
        elog(WARN, "ptrace() error: %s\n", strerror(errno));
        return -1;
    }

    while (!WIFEXITED(status))
    {
        err = ptrace(PTRACE_SYSCALL, pid, 0, 0);
        if (err < 0)
        {
            elog(WARN, "ptrace() error: %s\n", strerror(errno));
            return -1;
        }

        if (waitpid(pid, &status, 0) < 0)
        {
            elog(WARN, "wait() error: %s\n", strerror(errno));
            return -1;
        }

        if (WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80))
        {
            err = ptrace(PTRACE_GETREGS, pid, 0, &state);
            if (err < 0)
            {
                elog(WARN, "ptrace() error: %s\n", strerror(errno));
                return -1;
            }

            printf("SYSCALL %lld at %08llx\n", state.orig_rax, state.rip);

            err = ptrace(PTRACE_SYSCALL, pid, 0, 0);
            if (err < 0)
            {
                elog(WARN, "ptrace() error: %s\n", strerror(errno));
                return -1;
            }

            if (waitpid(pid, &status, 0) < 0)
            {
                elog(WARN, "wait() error: %s\n", strerror(errno));
                return -1;
            }
        }
    }

    while (1)
    {
        // wait after main procces kill as
        sleep(10);
    }

    return 0;
}

/**
 * \brief Create new background worker
 * \param [in] extension_handle - pointer to the extension handler.
 * Needed to get a function from it that will be executed in a new process
 * \param [in] bg_worker_data - data required to create a new process (more details in
 * the description of the structure itself)
 */
void create_bg_worker(void *extension_handle, const BGWorker_data *bg_worker_data)
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
    int pid = clone(function, (char *)stack + STACK_SIZE_FOR_BGWORKER, CLONE_FILES | CLONE_IO | SIGCHLD, NULL);
    if (pid < 0)
    {
        elog(WARN, "Can not create new process for background worker: %s", strerror(errno));
        munmap(stack, STACK_SIZE_FOR_BGWORKER);
        return;
    }

    BGWorker *bg_worker = (BGWorker *)malloc(sizeof(BGWorker));

    bg_worker->pid = pid;
    bg_worker->stack = stack;
    bg_worker->name = (char *)malloc(strlen(bg_worker_data->bg_worker_name) + 1);
    assert(bg_worker->name);
    strcpy(bg_worker->name, bg_worker_data->bg_worker_name);

    insert_to_list(&bg_workers_list, bg_worker);

    elog(LOG, "Create new background worker: %s with pid: %d", bg_worker_data->bg_worker_name, pid);
}

/**
 * \brief Create new background worker and tracer for it
 * \param [in] extension_handle - pointer to the extension handler.
 * Needed to get a function from it that will be executed in a new process
 * \param [in] bg_worker_data - data required to create a new process (more details in
 * the description of the structure itself)
 */
void create_bg_worker_tracer(void *extension_handle, const BGWorker_data *bg_worker_data)
{
    BGWorker_tracee bg_worker_tracee_data;
    bg_worker_tracee_data.function = dlsym(extension_handle, bg_worker_data->callback_name);
    if (bg_worker_tracee_data.function == NULL)
    {
        elog(WARN, "Can not get handle for backrgound worker start function: %s", dlerror());
        return;
    }

    void *tracee_stack = mmap(NULL, STACK_SIZE_FOR_BGWORKER, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_STACK | MAP_ANONYMOUS, -1, 0);
    if (tracee_stack == MAP_FAILED)
    {
        elog(WARN, "Can not allocate memory for background worker stack: %s", strerror(errno));
        return;
    }

    void *observer_stack = mmap(NULL, STACK_SIZE_FOR_BGWORKER, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_STACK | MAP_ANONYMOUS, -1, 0);
    if (observer_stack == MAP_FAILED)
    {
        munmap(tracee_stack, STACK_SIZE_FOR_BGWORKER);
        elog(WARN, "Can not allocate memory for background worker tracer stack: %s", strerror(errno));
        return;
    }

    bg_worker_tracee_data.stack = tracee_stack;
    int tracer_pid = clone(tracer, (char *)observer_stack + STACK_SIZE_FOR_BGWORKER,
                            CLONE_FILES | CLONE_IO | CLONE_VM | SIGCHLD, &bg_worker_tracee_data);
    if (tracer_pid < 0)
    {
        elog(WARN, "Can not create tracer process for background worker: %s", strerror(errno));
        munmap(observer_stack, STACK_SIZE_FOR_BGWORKER);
        return;
    }

    BGWorker *bg_worker_tracer = (BGWorker *)malloc(sizeof(BGWorker));

    bg_worker_tracer->pid = tracer_pid;
    bg_worker_tracer->stack = observer_stack;
    if (asprintf(&bg_worker_tracer->name, "%s tracer", bg_worker_data->bg_worker_name) < 0)
    {
        elog(WARN, "Can not create tracer process for background worker: %s", strerror(errno));
        munmap(observer_stack, STACK_SIZE_FOR_BGWORKER);
        return;
    }

    insert_to_list(&bg_workers_list, bg_worker_tracer);

    elog(LOG, "Create new background worker tracer: %s with pid: %d", bg_worker_tracer->name, tracer_pid);

    BGWorker *bg_worker_tracee = (BGWorker *)malloc(sizeof(BGWorker));

    bg_worker_tracee->pid = -1;
    bg_worker_tracer->stack = tracee_stack;
    bg_worker_tracee->name = (char *)malloc(strlen(bg_worker_data->bg_worker_name) + 1);
    assert(bg_worker_tracee->name);
    strcpy(bg_worker_tracee->name, bg_worker_data->bg_worker_name);

    insert_to_list(&bg_workers_list, bg_worker_tracee);

    elog(LOG, "Create new background worker: %s", bg_worker_tracee->name);
}

/**
 * \brief Destroy all background workers
 * \details "Destroy" means kill process, unmap stack and delete it from list of backgeound workers.
 */
void drop_all_workers()
{
    Workers_list_elem *curr_elem;
    while (bg_workers_list != NULL)
    {
        int status = 0;
        if (bg_workers_list->data->pid != -1)
        {
            if (kill(bg_workers_list->data->pid, SIGTERM) < 0)
            {
                elog(ERROR, "Can not kill background worker: %s with pid: %d - %s",
                    bg_workers_list->data->name, bg_workers_list->data->pid, strerror(errno));
            } else {
                if (waitpid(bg_workers_list->data->pid, &status, 0) < 0 && errno != ECHILD) { // see man 2 wait NOTES
                    elog(ERROR, "Can not get exit status of background worker: %s with pid: %d - %s",
                        bg_workers_list->data->name, bg_workers_list->data->pid, strerror(errno));
                } else {
                    // check how child proccess terminated
                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status);
                        if (exit_code != 0)
                            elog(WARN, "Proccess finished with non zero code: %d", exit_code);
                    } else if (WIFSIGNALED(status)) {
                        int sig_num = WTERMSIG(status);
                        elog(WARN, "Proccess terminated by signal: %d", sig_num);
                    } else {
                        elog(ERROR, "Undefined behaviour after killing proccess!");
                    }
                }
            }
        }

        curr_elem = bg_workers_list;
        if (munmap(bg_workers_list->data->stack, STACK_SIZE_FOR_BGWORKER) < 0)
        {
            elog(ERROR, "Destroy stack for background worker: %s with pid: %d failed - %s",
                 bg_workers_list->data->name, bg_workers_list->data->pid, strerror(errno));
        }

        free(bg_workers_list->data->name);

        bg_workers_list = bg_workers_list->next;
        delete_from_list(&curr_elem);
    }
}