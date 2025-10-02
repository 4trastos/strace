#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void    ft_free_split(char **str)
{
    if (!str)
        return;
    for (int i = 0; str[i] != NULL; i++)
    {
        free(str[i]);
        str[i] = NULL;
    }
    free(str);
}

void    free_syscall_info(t_syscall_info *syscall_info)
{
    if (syscall_info)
    {
        if (syscall_info->command_path)
        {
            ft_free_split(syscall_info->command_path);
            syscall_info->command_path = NULL;
        }
        if (syscall_info->binary)
        {
            free(syscall_info->binary);
            syscall_info->binary = NULL;
        }
    }
}