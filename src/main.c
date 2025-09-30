#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int  init_syscall_info(t_syscall_info *syscall_info, char **argv, char **envp)
{
    syscall_info->path = ft_findpath(envp);
    syscall_info->command_path = ft_split(syscall_info->path, ':');
    syscall_info->binary = get_binary(syscall_info->command_path, argv[1]);
    syscall_info->arch = detect_arch(syscall_info->binary);
    
    if (syscall_info->arch == -1 || syscall_info->arch == -2)
    {
        if (syscall_info->arch == -1)
            ft_printf("Error: Unrecognized architecture \n");
        return (1);
    }
    return (0);
}

int main(int argc, char **argv, char **envp)
{
    t_syscall_info  syscall_info;
    int             ret;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s must have PROG [ARGS] ❌\n", argv[0]);
        return (1);
    }

    if (init_syscall_info(&syscall_info, argv, envp) != 0)
    {
        free_syscall_info(&syscall_info);
        return (1);
    }

    ret = ft_strace(&syscall_info, argv, envp);
    free_syscall_info(&syscall_info);
    return (ret);
}