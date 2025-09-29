#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void    ft_read_string_from_mem(pid_t pid, unsigned long addr, char *buffer, size_t max_len)
{
    char    path[256];
    char    pid_str;
    int     fd;

    if (max_len == 0)
        return;
    
    ft_strcpy(path, "/proc/");
    pid_str = ft_itoa(pid);
}