#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int main(int argc, char **argv, char **envp)
{
    t_syscall_info          syscall_info;
    siginfo_t               siginfo;
    pid_t                   pid;
    int                     status;
    int                     syscall_state = 0;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s must have PROG [ARGS] ❌\n", argv[0]);
        return (1);
    }

    syscall_info.path = ft_findpath(envp);
    syscall_info.command_path = ft_split(syscall_info.path, ':');
    syscall_info.binary = get_binary(syscall_info.command_path, argv[1]);
    syscall_info.arch = detect_arch(syscall_info.binary);
    
    if (syscall_info.arch == -1 || syscall_info.arch == -2)
    {
        if (syscall_info.arch == -1)
            ft_printf("Error: Unrecognized architecture \n");
        return (1);
    }

    pid = fork();
    if (pid == -1)
    {
        ft_printf("Error: Falied fork ( %s )\n", strerror(errno));
        return (1);
    }

    if (pid == 0)
    {
        if (execve(syscall_info.binary, &argv[1], envp) == -1)
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
                    const char *sig_name = get_signal_name(WTERMSIG(status));
                    ft_printf("+++ Killed by signal %s +++\n", sig_name);
                }
                else
                    ft_printf("+++ Exited with status %d +++\n", WEXITSTATUS(status));
                break;
            }

            // Si se detiene por un syscall
            if (WSTOPSIG(status) == (SIGTRAP | 0x80))
            {
                if (syscall_state == 0) // ENTRY
                {
                    reading_entry_regs(pid, &syscall_info);
                    if (syscall_info.syscall_numb == 59)
                    {
                        const t_syscall_entry   *entry = &g_syscall_table[syscall_info.syscall_numb];
                        print_syscall_entry(pid, &syscall_info, entry);
                    }
                    syscall_state = 1;
                }
                else // EXIT
                {
                    reading_exit_regs(pid, &syscall_info);
                    if (syscall_info.syscall_numb == 59)    // execve: Ya se imprimieron los args, solo se imprime el retorno
                        print_syscall_exit(&syscall_info);
                    else
                    {
                        const t_syscall_entry *entry = &g_syscall_table[syscall_info.syscall_numb];
                        print_syscall_entry(pid, &syscall_info, entry);
                        print_syscall_exit(&syscall_info);
                    }
                    syscall_state = 0;
                }
            }
            // Si se detiene por una señal (no por un syscall)
            else
            {
                if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == -1)
                {
                    ft_printf("Error: GetSigInfo ( %s )\n", strerror(errno));
                    break;
                }
                ft_printf("--- %s (%s) ---\n", get_signal_name(siginfo.si_signo), "señal");
                ptrace(PTRACE_SYSCALL, pid, NULL, siginfo.si_signo);  // reinyecta señal
            }
        }
        free_syscall_info(&syscall_info);
    }

    return (0);
}