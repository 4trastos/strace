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

int         ft_strace(t_syscall_info *syscall_info, char **argv, char **envp)
{
    t_syscall_entry     *table;
    t_syscall_entry     *entry;
    siginfo_t           siginfo;
    pid_t               pid;
    pid_t               wait_pid;
    int                 status;
    int                 sig;
    int                 syscall_state = 0;              // 0 = ENTRY, 1 = EXIT

    pid = fork();
    if (pid == -1)
    {
        ft_printf("Error: Falied fork ( %s )\n", strerror(errno));
        return (1);
    }
    if (pid == 0)
    {
        if (execve(syscall_info->binary, &argv[1], envp) == -1)
        {
            ft_printf("Error: execve ( %s )\n", strerror(errno));
            return (1);
        }
    }
    else
    { 
        // Paso 1: "Apoderarse" del proceso hijo.
        if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace SEIZE ( %s )\n", strerror(errno));
            return (1);
        }

        // Paso 2: Interrumpir al hijo para que se detenga. Es una interrupción suave.
        if (ptrace(PTRACE_INTERRUPT, pid, NULL, NULL) == -1) {
            ft_printf("Error: ptrace INTERRUPT ( %s )\n", strerror(errno));
            return (1);
        }   

        waitpid(pid, &status, 0);       // Esperar la primera interrupción

        // Configurar opciones de trazao
        int options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT ;
        if (ptrace(PTRACE_SETOPTIONS, pid, NULL, options) == -1)
        {
            ft_printf("Error: ptrace SETOPTIONS ( %s )\n", strerror(errno));
            return (1);
        }
        
        while (1)
        {
            // static int iteration = 0;
            // ft_printf("\n*******************************\n");
            // ft_printf("[DEBUG] Iteration %d, tracing PID: %d", iteration++, pid);
            // ft_printf("\n*******************************\n");
            unblock_signals();

            // Pide al último PID que se detuvo que reanude. (hijo)
            if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1)
            {
                if (errno == ESRCH)
                    break;
                ft_printf("Error: ptrace SYSCALL ( %s )\n", strerror(errno));
                break;
            }

            //waitpid(pid, &status, 0);
            wait_pid = waitpid(-1, &status, __WALL);
            if (wait_pid == -1)
            {
                if (errno == ECHILD)
                    break;
                ft_printf("Error: waitpid ( %s )\n", strerror(errno));
                break;
            }

            pid = wait_pid;

            // Verificar Si el proceso termina o recibe una señal
            if (WIFEXITED(status))
            {
                ft_printf("exit_group(%d) = ?\n", WEXITSTATUS(status));
                ft_printf("+++ exited with %d +++\n", WEXITSTATUS(status));
                break; // SALIR del bucle principal
            }
            else if (WIFSIGNALED(status))
            {
                char *sig_name = get_signal_name(WTERMSIG(status));
                ft_printf("+++ killed by %s +++\n", sig_name);
                break; // SALIR del bucle principal
            }

            if (!WIFSTOPPED(status))
                continue;
            
            block_critical_signals();

            sig = WSTOPSIG(status);

            // Si se detiene por un syscall (ENTRY o EXIT)
            // 1. Dtección de Eventos PTRACE (CLONE, FORK, EXEC, EXIT)
            if (sig == SIGTRAP)
            {
                int event = (status >> 16) & 0xffff;
                
                if (event == PTRACE_EVENT_CLONE || event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK)
                {
                    // No podemos usar PTRACE_GETEVENTMSG, pero sabemos que hay un nuevo proceso
                    // Continuar el proceso actual, el nuevo se detectará automáticamente
                    continue;
                }
                else if (event == PTRACE_EVENT_EXIT) // El proceso está saliendo, continuar para que termine
                    continue;
            }
            
            // 2. Syscall stop (SIGTRAP | 0x80)
            if (sig == (SIGTRAP | 0x80))
            {
                if (syscall_state == 0)     // ENTRY
                {
                    reading_entry_regs(pid, syscall_info);

                    // **SALTAR syscalls de salida - los manejaremos diferente**
                    if (syscall_info->syscall_numb == 60 || syscall_info->syscall_numb == 231)
                    {
                        syscall_state = 1;
                        continue;
                    }

                    if (should_skip_syscall(syscall_info->syscall_numb))
                    {
                        syscall_state = 1;
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
                            print_syscall_entry(pid, syscall_info, entry);
                    }
                    syscall_state = 1;   // La siguiente parada es la SALIDA
                }
                else // EXIT
                {
                    reading_exit_regs(pid, syscall_info);

                    if (syscall_info->syscall_numb == 60 || syscall_info->syscall_numb == 231)
                    {
                        syscall_state = 0;
                        continue;
                    }

                    if (!should_skip_syscall(syscall_info->syscall_numb))
                        print_syscall_exit(syscall_info);
                    syscall_state = 0;
                }
            }
            else        // Señal regular
            {
                // **FILTRAR SEÑALES NO INTERESANTES COMO SIGTRAP INTERNOS**
                if (sig == SIGTRAP)
                {
                    // Solo continuar sin imprimir para SIGTRAP internos
                    ptrace(PTRACE_SYSCALL, pid, NULL, sig);
                    continue;
                }

                // Para otras señales, imprimir normalmente
                if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == 0)
                {
                    if (siginfo.si_signo == SIGCHLD)
                        ft_printf("--- %s {si_signo=%s, si_code=%d, si_pid=%d} ---\n", 
                            get_signal_name(siginfo.si_signo), 
                            get_signal_name(siginfo.si_signo),
                            siginfo.si_code, siginfo.si_pid);
                    else
                        ft_printf("--- %s {si_signo=%s, si_code=%d} ---\n", 
                                get_signal_name(siginfo.si_signo),
                                get_signal_name(siginfo.si_signo),
                                siginfo.si_code);
                }
                else
                    ft_printf("--- %s ---\n", get_signal_name(sig));
                ptrace(PTRACE_SYSCALL, pid, NULL, sig);
            }
            
        }
    }
    return (0);
}