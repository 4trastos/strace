#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int main(int argc, char **argv, char **envp)
{
    siginfo_t               siginfo;
    t_syscall_info          syscall_info;
    //t_signals               signals;
    pid_t                   pid;
    int                     status;
    int                     i = 0;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s must have PROG [ARGS] or -c [ARGS] ❌\n", argv[0]);
        return (1);
    }

    if (detect_arch(argv[1]) == -1)
    {
        ft_printf("Error: Unrecognized architecture \n");
        exit (1);
    }

    syscall_info.path = ft_findpath(envp);
    syscall_info.command_path = ft_split(syscall_info.path, ":");
    syscall_info.binary = get_binary(syscall_info.command_path, argv[1]);
    syscall_info.arch = detect_arch(syscall_info.path);
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
        if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace SEIZE ( %s )\n", strerror(errno));
            return (1);
        }
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

            if (WIFSTOPPED(status))
            {
                if (WSTOPSIG(status) == (SIGTRAP | 0x80))
                {
                    // Syscall event
                    reading_regs(pid, &syscall_info);
                    ft_printf("Estos son los registros: ( %d )", syscall_info.arguments[i]);
                    i++;
                }
                else
                {
                    // Signal event
                    if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == -1)
                    {
                        ft_printf("Error: GetSigInfo ( %s )\n", strerror(errno));
                        exit (1);
                    }
                    //reading_signals(&siginfo, &signals);
                    ptrace(PTRACE_SYSCALL, pid, NULL, WSTOPSIG(status));  // reinyecta señal
                    continue;
                }
            }

            // Segundo paso → salida de syscall
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
            
            reading_regs(pid, &syscall_info);
            ft_printf("syscall_name(arg1, arg2, ...) = return_value\n");
        }
    }

    return (0);
}