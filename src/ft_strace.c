#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

t_syscall_entry *get_syscall_table(int arch)
{
    if (arch == ARCH_64)
        return (g_syscall_table_64);
    else if (arch == ARCH_32)
        return (g_syscall_table_32);
    else
        return (NULL);
}

int ft_strace(t_syscall_info *syscall_info, char **argv, char **envp)
{
    t_syscall_entry     *table;
    t_syscall_entry     *entry;
    siginfo_t           siginfo;
    pid_t               pid;
    pid_t               wait_pid;
    int                 status;
    int                 sig;

    // Inicializar la lista de procesos
    struct traced_process_head s_traced_process;
    TAILQ_INIT(&s_traced_process);

    pid = fork();
    if (pid == -1)
    {
        ft_printf("Error: Failed fork ( %s )\n", strerror(errno));
        return (1);
    }
    if (pid == 0)
    {
        if (execve(syscall_info->binary, &argv[1], envp) == -1)
        {
            ft_printf("Error: execve ( %s )\n", strerror(errno));
            return (1);
        }
        return (0);
    }
    else
    { 
        block_critical_signals();

        // Configurar tracing
        if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace SEIZE ( %s )\n", strerror(errno));
            return (1);
        }

        if (ptrace(PTRACE_INTERRUPT, pid, NULL, NULL) == -1) {
            ft_printf("Error: ptrace INTERRUPT ( %s )\n", strerror(errno));
            return (1);
        }   

        waitpid(pid, &status, 0);

        int options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT;
        if (ptrace(PTRACE_SETOPTIONS, pid, NULL, options) == -1)
        {
            ft_printf("Error: ptrace SETOPTIONS ( %s )\n", strerror(errno));
            return (1);
        }

        // Añadir proceso principal a la lista
        t_traced_process *main_proc = malloc(sizeof(t_traced_process));
        if (!main_proc)
            return(1);
        main_proc->pid = pid;
        main_proc->syscall_state = 0;
        main_proc->is_thread = 0;  // ← Proceso principal no es thread
        main_proc->arch = syscall_info->arch;
        TAILQ_INSERT_TAIL(&s_traced_process, main_proc, entries);
        
        while (!TAILQ_EMPTY(&s_traced_process))
        {
            // TODOS los procesos usando proc->pid
            t_traced_process *proc;
            TAILQ_FOREACH(proc, &s_traced_process, entries)
            {
                if (ptrace(PTRACE_SYSCALL, proc->pid, NULL, NULL) == -1)
                {
                    if (errno == ESRCH)
                    {
                        // El proceso terminó, será removido en el próximo ciclo
                        continue;
                    }
                }
            }

            wait_pid = waitpid(-1, &status, __WALL);
            if (wait_pid == -1)
            {
                if (errno == ECHILD)
                    break;
                continue;
            }

            // Buscar el proceso en la lista
            t_traced_process *current_proc = NULL;
            TAILQ_FOREACH(current_proc, &s_traced_process, entries)
            {
                if (current_proc->pid == wait_pid)
                    break;
            }

            // Nuevo proceso o thread
            if (!current_proc && WIFSTOPPED(status))
            {
                current_proc = malloc(sizeof(t_traced_process));
                if (!current_proc)
                    continue;
                current_proc->pid = wait_pid;
                current_proc->syscall_state = 0;
                // Verificar si es un thread
                current_proc->is_thread = is_thread_process(wait_pid);
                current_proc->arch = syscall_info->arch;
                TAILQ_INSERT_TAIL(&s_traced_process, current_proc, entries);
                continue;
            }

            if (!current_proc)
                continue;

            // Procesos terminados
            if (WIFEXITED(status))
            {
                if (current_proc->pid == pid)
                {
                    pthread_mutex_lock(&output_mutex);
                    ft_printf("exit_group(%d) = ?\n", WEXITSTATUS(status));
                    ft_printf("+++ exited with %d +++\n", WEXITSTATUS(status));
                    pthread_mutex_unlock(&output_mutex);
                }
                TAILQ_REMOVE(&s_traced_process, current_proc, entries);
                free(current_proc);
                continue;
            }
            else if (WIFSIGNALED(status))
            {
                char *sig_name = get_signal_name(WTERMSIG(status));
                pthread_mutex_lock(&output_mutex);
                ft_printf("+++ killed by %s +++\n", sig_name);
                pthread_mutex_unlock(&output_mutex);
                TAILQ_REMOVE(&s_traced_process, current_proc, entries);
                free(current_proc);
                continue;
            }

            if (!WIFSTOPPED(status))
                continue;
            
            block_critical_signals();

            sig = WSTOPSIG(status);

            // Eventos ptrace
            if (sig == SIGTRAP)
            {
                int event = (status >> 16) & 0xffff;
                if (event == PTRACE_EVENT_CLONE || event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK)
                    continue;
            }
            
            // Syscall stop
            if (sig == (SIGTRAP | 0x80))
            {
                if (current_proc->syscall_state == 0) // ENTRY
                {
                    reading_entry_regs(current_proc->pid, syscall_info);

                    if (syscall_info->syscall_numb == 60 || syscall_info->syscall_numb == 231)
                    {
                        current_proc->syscall_state = 1;
                        continue;
                    }

                    if (should_skip_process_syscall(current_proc->pid, syscall_info->syscall_numb, pid))
                    {
                        current_proc->syscall_state = 1;
                        continue;
                    }

                    table = get_syscall_table(syscall_info->arch);
                    if (!table || syscall_info->syscall_numb < 0 || syscall_info->syscall_numb >= MAX_SYSCALLS_32)
                    {
                        pthread_mutex_lock(&output_mutex);
                        ft_printf("syscall_%d(", (int)syscall_info->syscall_numb);
                        pthread_mutex_unlock(&output_mutex);
                    }
                    else
                    {
                        entry = &table[syscall_info->syscall_numb];
                        if (!entry || !entry->name)
                        {
                            pthread_mutex_lock(&output_mutex);
                            ft_printf("syscall_%d(", (int)syscall_info->syscall_numb);
                            pthread_mutex_unlock(&output_mutex);
                        }
                        else
                            print_syscall_entry(current_proc->pid, syscall_info, entry);
                    }
                    current_proc->syscall_state = 1;
                }
                else // EXIT
                {
                    reading_exit_regs(current_proc->pid, syscall_info);

                    if (syscall_info->syscall_numb == 60 || syscall_info->syscall_numb == 231)
                    {
                        current_proc->syscall_state = 0;
                        continue;
                    }

                    if (!should_skip_process_syscall(current_proc->pid, syscall_info->syscall_numb, pid))
                        print_syscall_exit(syscall_info);
                    current_proc->syscall_state = 0;
                }
            }
            else // señales
            {
                if (sig == SIGTRAP)
                {
                    ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
                    continue;
                }

                if (sig == SIGINT)
                {
                    pthread_mutex_lock(&output_mutex);
                    ft_printf("--- %s {si_signo=%s, si_code=SI_KERNEL} ---\n", get_signal_name(sig), get_signal_name(sig));
                    ft_printf("strace: Process %d detached\n", pid);
                    pthread_mutex_unlock(&output_mutex);
                    t_traced_process *proc;
                    TAILQ_FOREACH(proc, &s_traced_process, entries)
                        kill(proc->pid, SIGKILL);
                    
                    while (!TAILQ_EMPTY(&s_traced_process))
                    {
                        t_traced_process *proc = TAILQ_FIRST(&s_traced_process);
                        TAILQ_REMOVE(&s_traced_process, proc, entries);
                        free(proc);
                    }
                    break;
                }

                if (sig == SIGSEGV)
                {
                    if (ptrace(PTRACE_GETSIGINFO, current_proc->pid, NULL, &siginfo) == 0)
                    {
                        const char *segv_code = "SEGV_MAPERR";
                        if (siginfo.si_code == SEGV_ACCERR)
                            segv_code = "SEGV_ACCERR";

                        pthread_mutex_lock(&output_mutex);
                        ft_printf("--- %s {si_signo=%s, si_code=%s, si_addr=%p} ---\n", 
                                get_signal_name(siginfo.si_signo), get_signal_name(siginfo.si_signo), segv_code, siginfo.si_addr);
                        pthread_mutex_unlock(&output_mutex);
                    }
                    // 1. Reenviar la señal (el kernel la entregará, terminando el proceso)
                    ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
                    continue;
                }

                if (ptrace(PTRACE_GETSIGINFO, current_proc->pid, NULL, &siginfo) == 0)
                {
                    pthread_mutex_lock(&output_mutex);
                    ft_printf("--- %s {si_signo=%s, si_code=%d} ---\n", get_signal_name(siginfo.si_signo),
                            get_signal_name(siginfo.si_signo), siginfo.si_code);
                    pthread_mutex_unlock(&output_mutex);
                }
                else
                {
                    pthread_mutex_lock(&output_mutex);
                    ft_printf("--- %s ---\n", get_signal_name(sig));
                    pthread_mutex_unlock(&output_mutex);
                }
                ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
            }
        }
    }
    return (0);
}