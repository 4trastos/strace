#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int main(int argc, char **argv)
{
    struct user_regs_struct regs;
    struct iovec            iov;
    siginfo_t               siginfo;
    t_syscall_info          syscall_info;
    t_signals               signals;
    pid_t                   pid;
    int                     status;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s must have PROG [ARGS] or -c [ARGS] ❌\n", argv[0]);
        return (1);
    }
    pid = fork();
    if (pid == -1)
    {
        ft_printf("Error: Falied fork ( %s )\n", strerror(errno));
        exit(1);
    }
    if (pid == 0)
    {
        kill(getpid(), SIGSTOP);
        if (execvp(argv[1], &argv[1]) == -1)
        {
            ft_printf("Error: execvp ( %s )\n", strerror(errno));
            exit(1);
        }
    }
    else
    {
        waitpid(pid, &status, 0);
        // Paso 3: El padre se apodera del proceso hijo.
        if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace SEIZE ( %s )\n", strerror(errno));
            return (1);
        }
        // Paso 4: Se usa PTRACE_SETOPTIONS para habilitar opciones.
        if (ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD) == -1)
        {
            ft_printf("Error: ptrace SETOPTIONS ( %s )\n", strerror(errno));
            return (1);
        }
        while (1)
        {
            if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1)
            {
                if (errno == ESRCH)
                    break;
                ft_printf("Error: ptrace SYSCALL ( %s )\n", strerror(errno));
                break;
            }

            waitpid(pid, &status, 0);
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                ft_printf("+++ Exited with status %d +++\n", WEXITSTATUS(status));
                break;
            }

            if (WIFSTOPPED(status) && WSTOPSIG(status) != (SIGTRAP | 0x80))
            {
                ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo);
                reading_signals(&siginfo, &signals);
                ptrace(PTRACE_SYSCALL, pid, NULL, WSTOPSIG(status)); // reinyecta señal
                continue;
            }

            ptrace(PTRACE_GETREGSET, pid, NULL, &regs);
            reading_regs(&regs, &syscall_info);

            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                ft_printf("+++ Exited with status %d +++\n", WEXITSTATUS(status));
                break;
            }
            
            ptrace(PTRACE_GETREGSET, pid, NULL, &regs);
            reading_return_value(&regs, &syscall_info);
            ft_printf("syscall_name(arg1, arg2, ...) = return_value\n");
        }
    }

    return (0);
}