#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void    print_flags(long value, t_flag_entry *flags)
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

char *get_error_name(long errnum)
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

void    print_syscall_entry(pid_t pid, t_syscall_info *info, t_syscall_entry *entry)
{
    if (info->syscall_numb < 0)
        return;
    ft_printf("%s(", entry->name);

    for (int i = 0; i < 6; i++)
    {
        if (entry->arg_types[i] == VOID)
            break;
        
        // SYS_write (1) o SYS_read (0): el segundo argumento (i=1) es el buffer y el tercero (i=2) es la longitud (count)
        if ((info->syscall_numb == 1 || info->syscall_numb == 0) && i == 1)
        {
            char    temp_buffer[64];                                // Mostrar los primeros 64 bytes
            if (info->arguments[i] == 0)
                ft_printf("NULL");
            else
                ft_read_buffer_from_mem(pid, info->arguments[i], info->arguments[2], temp_buffer, sizeof(temp_buffer));
        }
        else if ((info->syscall_numb == 59) && (i == 1 || i == 2))  // SYS_execve (59): Lectura de vector de argv/envp (i=1 y i=2)
            ft_read_argv(pid, info->arguments[i]);

        // --- Manejo de FLAGS (Mmap, ioctl, access, openat) ---
        else if (info->syscall_numb == 9 && i == 2)                 // SYS_mmap, tercer argumento (prot)
            print_flags(info->arguments[i], g_prot_flags);
        else if (info->syscall_numb == 9 && i == 3)                 // SYS_mmap, cuarto argumento (flags)
            print_flags(info->arguments[i], g_map_flags);
        else if (info->syscall_numb == 16 && i == 1)                // 16 es SYS_ioctl
            print_flags(info->arguments[i], g_ioctl_cmds);
        else if (info->syscall_numb == 21 && i == 2)                // SYS_access, second argument
            print_flags(info->arguments[i], g_access_flags);
        else if (info->syscall_numb == 256 && i == 2)               // SYS_openat, tercer argumento son flags
            print_flags(info->arguments[i], g_openat_flags); 
        else 
        {
            if (entry->arg_types[i] == INT)
                ft_printf("%d", info->arguments[i]);
            else if (entry->arg_types[i] == POINTER || entry->arg_types[i] == STRING)
            {
                if (entry->arg_types[i] == STRING)
                {
                    char temp_str[128];
                    if (info->arguments[i] == 0)
                        ft_printf("NULL");
                    else
                    {
                        // Se usa la función de lectura de strings (terminadas en null) por defecto
                        ft_read_string_from_mem(pid, info->arguments[i], temp_str, sizeof(temp_str));
                        ft_printf("\"%s\"", temp_str);
                    }
                }
                else
                    ft_printf("%p", info->arguments[i]);
            }
        }
        if (i < 5 && entry->arg_types[i + 1] != VOID)
            ft_printf(", ");
    }
}

void    print_syscall_exit(t_syscall_info *info)
{
    if (info->return_value < 0)
        ft_printf(") = -1 %s\n", get_error_name(info->return_value));
    else
    {
        // 9 es mmap, 12 es brk, 59:execve (valores que se imprimen en hexadecimal)
        if (info->syscall_numb == 9 || info->syscall_numb == 12 || info->syscall_numb == 59)
            ft_printf(") = %p\n", info->return_value);
        else
            ft_printf(") = %d\n", info->return_value);
    }
}