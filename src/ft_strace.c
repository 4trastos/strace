#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

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

    // Inicializar la lista
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
        TAILQ_INSERT_TAIL(&s_traced_process, main_proc, entries);
        
        while (!TAILQ_EMPTY(&s_traced_process))
        {
            unblock_signals();

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

            // Buscar el proceso en nuestra lista
            t_traced_process *current_proc = NULL;
            TAILQ_FOREACH(current_proc, &s_traced_process, entries)
            {
                if (current_proc->pid == wait_pid)
                    break;
            }

            // Si es un proceso nuevo, añadirlo
            if (!current_proc && WIFSTOPPED(status))
            {
                current_proc = malloc(sizeof(t_traced_process));
                if (!current_proc) continue;
                current_proc->pid = wait_pid;
                current_proc->syscall_state = 0;
                TAILQ_INSERT_TAIL(&s_traced_process, current_proc, entries);
                continue;
            }

            if (!current_proc) continue;

            // Verificar terminación
            if (WIFEXITED(status))
            {
                if (current_proc->pid == pid)
                {
                    ft_printf("exit_group(%d) = ?\n", WEXITSTATUS(status));
                    ft_printf("+++ exited with %d +++\n", WEXITSTATUS(status));
                }
                TAILQ_REMOVE(&s_traced_process, current_proc, entries);
                free(current_proc);
                continue;
            }
            else if (WIFSIGNALED(status))
            {
                char *sig_name = get_signal_name(WTERMSIG(status));
                ft_printf("+++ killed by %s +++\n", sig_name);
                TAILQ_REMOVE(&s_traced_process, current_proc, entries);
                free(current_proc);
                continue;
            }

            if (!WIFSTOPPED(status)) continue;
            
            block_critical_signals();

            sig = WSTOPSIG(status);

            // Manejar eventos ptrace
            if (sig == SIGTRAP)
            {
                int event = (status >> 16) & 0xffff;
                if (event == PTRACE_EVENT_CLONE || event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK)
                    continue;
                /* else if (event == PTRACE_EVENT_EXIT)
                    continue; */
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

                    if (should_skip_syscall(syscall_info->syscall_numb))
                    {
                        current_proc->syscall_state = 1;
                        continue;
                    }

                    table = get_syscall_table(syscall_info->arch);
                    if (!table || syscall_info->syscall_numb < 0 || syscall_info->syscall_numb >= MAX_SYSCALLS_32)
                        ft_printf("syscall_%d(", (int)syscall_info->syscall_numb);
                    else
                    {
                        entry = &table[syscall_info->syscall_numb];
                        if (!entry || !entry->name)
                            ft_printf("syscall_%d(", (int)syscall_info->syscall_numb);
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

                    if (!should_skip_syscall(syscall_info->syscall_numb))
                        print_syscall_exit(syscall_info);
                    current_proc->syscall_state = 0;
                }
            }
            else // Otras señales
            {
                if (sig == SIGTRAP)
                {
                    ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
                    continue;
                }

                if (sig == SIGSEGV)
                {
                    if (ptrace(PTRACE_GETSIGINFO, current_proc->pid, NULL, &siginfo) == 0)
                    {
                        const char *segv_code = "SEGV_MAPERR";
                        if (siginfo.si_code == SEGV_ACCERR)
                            segv_code = "SEGV_ACCERR";
                        
                        ft_printf("--- %s {si_signo=%s, si_code=%s, si_addr=%p} ---\n", 
                                get_signal_name(siginfo.si_signo),
                                get_signal_name(siginfo.si_signo),
                                segv_code,
                                siginfo.si_addr);
                    }
                    ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
                    continue;
                }

                if (ptrace(PTRACE_GETSIGINFO, current_proc->pid, NULL, &siginfo) == 0)
                {
                    ft_printf("--- %s {si_signo=%s, si_code=%d} ---\n", 
                            get_signal_name(siginfo.si_signo),
                            get_signal_name(siginfo.si_signo),
                            siginfo.si_code);
                }
                else
                    ft_printf("--- %s ---\n", get_signal_name(sig));
                ptrace(PTRACE_SYSCALL, current_proc->pid, NULL, sig);
            }
        }
    }
    return (0);
}