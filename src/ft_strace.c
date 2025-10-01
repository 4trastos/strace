#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

t_syscall_entry *get_syscall_table(int arch)
{
    if (arch == ARCH_64)
        return g_syscall_table_64;
    else
        return g_syscall_table_32;
}

int         ft_strace(t_syscall_info *syscall_info, char **argv, char **envp)
{
    siginfo_t           siginfo;
    t_syscall_entry     *entry;
    pid_t               pid;
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

        waitpid(pid, &status, 0);
        // Configurar opciones de tracing
        if (ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD) == -1)
        {
            ft_printf("Error: ptrace SETOPTIONS ( %s )\n", strerror(errno));
            return (1);
        }
        
        while (1)
        {
            // Pide al hijo que reanude y espere la siguiente parada
            if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1)
            {
                if (errno == ESRCH)
                    break;
                ft_printf("Error: ptrace SYSCALL ( %s )\n", strerror(errno));
                break;
            }

            waitpid(pid, &status, 0);
            // Si el proceso termina o recibe una señal
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                if (WIFSIGNALED(status))
                {
                    char *sig_name = get_signal_name(WTERMSIG(status));
                    ft_printf("\n+++ Killed by signal %s +++\n", sig_name);
                }
                else
                    ft_printf("\n+++ Exited with status %d +++\n", WEXITSTATUS(status));
                break;
            }

            // Si se detiene por un syscall (ENTRY o EXIT)
            if (WIFSTOPPED(status))
            {
                sig = WSTOPSIG(status);
                if (sig == (SIGTRAP | 0x80))
                {
                    if (syscall_state == 0) // ENTRY
                    {
                        // Leer número de syscall y argumentos de los registros
                        reading_entry_regs(pid, syscall_info);
                        // Obtener la entrada de la tabla global (siempre debe existir)
                        entry = &(get_syscall_table(syscall_info->arch))[syscall_info->syscall_numb];
                        print_syscall_entry(pid, syscall_info, entry);
                        syscall_state = 1;   // La siguiente parada es la SALIDA
                    }
                    else // EXIT
                    {
                        // Leer el valor de retorno del syscall
                        reading_exit_regs(pid, syscall_info);
                        print_syscall_exit(syscall_info);
                        syscall_state = 0;
                    }
                }
                // Si se detiene por una señal (no por un syscall)
                else
                {
                    if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == 0)
                    {
                        //ft_printf("--- %s (%s) ---\n", get_signal_name(siginfo.si_signo), "señal");
                        ft_printf("--- %s {si_signo=%d, si_code=%d, si_pid=%d} ---\n", get_signal_name(siginfo.si_signo), siginfo.si_signo, siginfo.si_code, siginfo.si_pid);
                    }
                    else
                        ft_printf("--- %s ---\n", get_signal_name(sig));
                    //ptrace(PTRACE_SYSCALL, pid, NULL, siginfo.si_signo);
                    ptrace(PTRACE_SYSCALL, pid, NULL, sig);
                }
            }
        }
    }
    return (0);
}