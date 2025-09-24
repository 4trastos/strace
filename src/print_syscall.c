#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void    print_flags(long value, const t_flag_entry *flags)
{
    bool    first_flag = true;
    long    remaining_value = value;

    if (value == 0)
    {
        ft_printf("0");
        return;
    }
    for (int i = 0; flags[i].name != NULL; i++)
    {
        if ((remaining_value & flags[i].value) == flags[i].value)
        {
            if (!first_flag)
                ft_printf("|");
            ft_printf("%s", flags[i].name);
            first_flag = false;
            remaining_value &= ~flags[i].value; //Eliminar la bandera del valor restante
        }
    }
    if (remaining_value != 0)
    {
        if (!first_flag)
            ft_printf("|");
        ft_printf("%p", remaining_value);
    }
}

const char *get_error_name(long errnum)
{
    int i = 0;

    errnum = errnum * -1;
    while (g_error_table[i].err_name != NULL)
    {
        if (g_error_table[i].err_num == errnum)
            return (g_error_table[i].err_name);
        i++;
    }
    return ("Unknown error");

}

void    print_syscall_args(pid_t pid, const t_syscall_info *info, const t_syscall_entry *entry)
{
    (void)pid;
    ft_printf("%s(", entry->name);

    for (int i = 0; i < 6; i++)
    {
        if (entry->arg_types[i] == VOID)
            break;
        
        if (info->syscall_numb == 9 && i == 2)              // SYS_mmap, tercer argumento (prot)
            print_flags(info->arguments[i], g_prot_flags);
        else if (info->syscall_numb == 9 && i == 3)         // SYS_mmap, cuarto argumento (flags)
            print_flags(info->arguments[i], g_map_flags);
        else if (info->syscall_numb == 16 && i == 1)     // 16 es SYS_ioctl
            print_flags(info->arguments[i], g_ioctl_cmds);
        else if (info->syscall_numb == 21 && i == 2)        // SYS_access, second argument
            print_flags(info->arguments[i], g_access_flags);
        else if (info->syscall_numb == 257 && i == 1)       // SYS_openat, segundo argumento
            print_flags(info->arguments[i], g_openat_flags); 
        else 
        {
            if (entry->arg_types[i] == INT)
                ft_printf("%d", info->arguments[i]);
            else if (entry->arg_types[i] == POINTER || entry->arg_types[i] == STRING)
                ft_printf("%p", info->arguments[i]);
        }
        if (i < 5 && entry->arg_types[i + 1] != VOID)
            ft_printf(", ");
    }
    if (info->return_value < 0)
        ft_printf(") = -1 %s\n", get_error_name(info->return_value));
    else
    {
        if (info->syscall_numb == 9 || info->syscall_numb == 12) // 9 es mmap, 12 es brk
            ft_printf(") = %p\n", info->return_value);
        else
            ft_printf(") = %d\n", info->return_value);
    }
}