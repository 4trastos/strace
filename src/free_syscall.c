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

void    free_syscall_info(t_syscall_info *info)
{
    if (info)
    {
        if (info->command_path)
        {
            ft_free_split(info->command_path);
            info->command_path = NULL;
        }
        if (info->binary)
        {
            free(info->binary);
            info->binary = NULL;
        }
    }
}