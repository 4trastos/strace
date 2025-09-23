#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int main(int argc, char **argv, char **envp)
{
    t_syscall_info          syscall_info;
    siginfo_t               siginfo;
    //t_signals               signals;
    pid_t                   pid;
    int                     status;
    int                     i = 0;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s must have PROG [ARGS] or -c [ARGS] ❌\n", argv[0]);
        return (1);
    }

    ft_printf("[DEBUG] PID de proceso del binario: ( %d )\n", pid = getpid());
    syscall_info.path = ft_findpath(envp);
    ft_printf("[DEBUG] Path encontada: ( %s )\n", syscall_info.path);
    syscall_info.command_path = ft_split(syscall_info.path, ':');
    syscall_info.binary = get_binary(syscall_info.command_path, argv[1]);
    ft_printf("[DEBUG] Binario encontado: ( %s )\n", syscall_info.binary);
    syscall_info.arch = detect_arch(syscall_info.binary);
    ft_printf("[DEBUG] Architectura detectada: ( %d )\n", syscall_info.arch);
    
    if (syscall_info.arch == -1)
    {
        ft_printf("Error: Unrecognized architecture \n");
        exit (1);
    }

    pid = fork();
    if (pid == -1)
    {
        ft_printf("Error: Falied fork ( %s )\n", strerror(errno));
        exit(1);
    }
    if (pid == 0)
    {
        //ft_printf("[DEBUG] Identificador fork: ( %d )\n", pid);
        //ft_printf("[DEBUG] Dentro del proceso hijo. PIP ( %d )\n", pid = getpid());
        kill(getpid(), SIGSTOP);
        if (execve(syscall_info.binary, &argv[1], envp) == -1)
        {
            ft_printf("Error: execve ( %s )\n", strerror(errno));
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
                    // Syscall event → syscall entry
                    reading_entry_regs(pid, &syscall_info);
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

            // Segundo paso → salida de syscall_exit
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
            
            reading_exit_regs(pid, &syscall_info);
            ft_printf("syscall_name(...) = %ld\n", syscall_info.return_value);
        }
    }

    return (0);
}