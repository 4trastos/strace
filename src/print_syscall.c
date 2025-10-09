#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int is_flag_zero(t_flag_entry *flags)
{
    for (int i = 0; flags[i].name != NULL; i++)
    {
        if (flags[i].value == 0)
            return 1;
    }
    return 0;
}

void    print_flags(long value, t_flag_entry *flags)
{
    bool    first_flag = true;
    long    remaining_value = value;

    if (value == 0 && !is_flag_zero(flags))
    {
        ft_printf("0");
        return;
    }

    for (int i = 0; flags[i].name != NULL; i++)
    {
        // Verificar igualdad exacta, no solo bit set
        if ((remaining_value & flags[i].value) == flags[i].value && flags[i].value != 0)
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
        ft_printf("%p", (void *)remaining_value);
    }
    else if (first_flag && value == 0)
        ft_printf("0");
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
    
    pthread_mutex_lock(&output_mutex);
    if (info->syscall_numb == 230)                      // SYS_clock_nanosleep
    {
        ft_printf("%s(", entry->name);
        
        // clock_id (arg0)
        if (info->arguments[0] == 0)
            ft_printf("CLOCK_REALTIME");
        else if (info->arguments[0] == 1)
            ft_printf("CLOCK_MONOTONIC");
        else
            ft_printf("%d", (int)info->arguments[0]);
        
        ft_printf(", ");
        
        // flags (arg1)
        if (info->arguments[1] == 0)
            ft_printf("0");
        else if (info->arguments[1] == 1)
            ft_printf("TIMER_ABSTIME");
        else
            ft_printf("%d", (int)info->arguments[1]);
        
        ft_printf(", %p, %p", (void *)info->arguments[2], (void *)info->arguments[3]);

        pthread_mutex_unlock(&output_mutex);
        return;
    }
    
    if (info->syscall_numb == 60 || info->syscall_numb == 231)  // SYS_exit (60) y SYS_exit_group (231)
    {
        ft_printf("%s(%d", entry->name, (int)info->arguments[0]);
        // Cierre manual para syscalls de 1 argumento
        for (int i = 1; i < 6; i++)
        {
            if (entry->arg_types[i] != VOID)
                ft_printf(", ...");
        }
        ft_printf(")");
        pthread_mutex_unlock(&output_mutex);
        return;
    }

    if (info->syscall_numb == 56) // SYS_clone - ENTRY
    {
        ft_printf("%s(", entry->name);
        
        if (info->arguments[1] == 0)
            ft_printf("child_stack=NULL");
        else
            ft_printf("child_stack=%p", (void *)info->arguments[1]);
        
        // flags (arg0)
        long flags = info->arguments[0];
        ft_printf(", flags=");
        
        // **MANEJAR SIGCHLD CORRECTAMENTE**
        int has_sigchld = 0;
        int sig = flags & 0xFF;
        if (sig == SIGCHLD)
        {
            has_sigchld = 1;
            flags &= ~0xFF;
        }
        
        // Imprimir flags de clone
        if (flags != 0)
        {
            print_flags(flags, g_clone_flags);
            if (has_sigchld)
                ft_printf("|SIGCHLD");
        }
        else if (has_sigchld)
            ft_printf("SIGCHLD");
        else
            ft_printf("0");
        
        // El orden de ft_strace está siguiendo el orden de registros (rdi, rsi, rdx, r10, r8, r9), no el de strace.
        //ft_printf(", parent_tid=%p", (void *)info->arguments[2]);           // parent_tid
        ft_printf(", child_tidptr=%p", (void *)info->arguments[3]);         // child_tid
        //ft_printf(", tls=%p", (void *)info->arguments[4]);                  // tls (r8)
        pthread_mutex_unlock(&output_mutex);
        return;
    }

    if (info->syscall_numb == 61) // SYS_wait4
    {
        ft_printf("%s(", entry->name);
        
        // pid (arg0)
        if ((pid_t)info->arguments[0] == -1)
            ft_printf("-1");
        else
            ft_printf("%d", (int)info->arguments[0]);
        
        // status (arg1)  
        if (info->arguments[1] == 0)
            ft_printf(", NULL");
        else
            ft_printf(", %p", (void *)info->arguments[1]);
        
        // options (arg2)
        ft_printf(" ,");
        print_flags(info->arguments[2], g_wait4_flags);
        
        // rusage (arg3)
        if (info->arguments[3] == 0)
            ft_printf(", NULL");
        else
            ft_printf(", %p", (void *)info->arguments[3]);
        
        pthread_mutex_unlock(&output_mutex);
        return;
    }
    
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

        // --- Manejo de FLAGS (mmap, ioctl, access, openat) ---
        else if (info->syscall_numb == 9 && i == 2)                 // SYS_mmap, tercer argumento (prot)
            print_flags(info->arguments[i], g_prot_flags);
        else if (info->syscall_numb == 9 && i == 3)                 // SYS_mmap, cuarto argumento (flags)
            print_flags(info->arguments[i], g_map_flags);
        else if (info->syscall_numb == 16 && i == 1)                // 16 es SYS_ioctl
            print_flags(info->arguments[i], g_ioctl_cmds);
        else if (info->syscall_numb == 21 && i == 2)                // SYS_access, segundo argument
            print_flags(info->arguments[i], g_access_flags);
        else if (info->syscall_numb == 257 && i == 0)                // SYS_openat, primer argumento
        {
            if ((int)info->arguments[i] == -100)
                ft_printf("AT_FDCWD, ");
            else
                ft_printf("%d, ", (int)info->arguments[i]);
            //continue;
        }
        else if (info->syscall_numb == 257 && i == 1)              // SYS_openat, segundo argumento (pathname)
        {
            char temp_str[128];
            if (info->arguments[i] == 0)
                ft_printf("NULL");
            else
            {
                ft_read_string_from_mem(pid, info->arguments[i], temp_str, sizeof(temp_str));
                
                if (temp_str[0] == '0' && temp_str[1] == 'x')       // Si devolvió una dirección hex
                    ft_printf("%p", (void *)info->arguments[i]);    // Mostrar la dirección original
                else if (temp_str[0] == '[')                        // Si devolvió un mensaje de error
                    ft_printf("%p", (void *)info->arguments[i]);    // Mostrar la dirección original  
                else
                    ft_printf("\"%s\"", temp_str);                  // String válido
            }
        }
        else if (info->syscall_numb == 257 && i == 2)               // SYS_openat, tercer argumento son flags
            print_flags(info->arguments[i], g_openat_flags); 
        else 
        {
            if (entry->arg_types[i] == INT && info->arguments[i] != -100)
                ft_printf("%d", (int)info->arguments[i]);
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
    pthread_mutex_unlock(&output_mutex);
}

void    print_syscall_exit(t_syscall_info *info)
{
    pthread_mutex_lock(&output_mutex);
    if (info->return_value < 0)
    {
        char *err_name = get_error_name(info->return_value);
        char *err_str = strerror(-info->return_value);
        ft_printf(") = -1 %s (%s)\n", err_name, err_str);
    }
    else
    {

        if (info->syscall_numb == 56)                           // SYS clone
        {
            if (info->return_value < 0)
                ft_printf(") = -1 %s\n", get_error_name(info->return_value));
            else if (info->return_value == 0)
                ft_printf(") = 0\n");                           // hijo
            else
                ft_printf(") = %d\n", (int)info->return_value); // padre
            pthread_mutex_unlock(&output_mutex);
            return;
        }
        
        // Suprimir el retorno para exit (60) y exit_group (231) **
        if (info->syscall_numb == 60 || info->syscall_numb == 231)
        {
            ft_printf(") = ?\n");
            pthread_mutex_unlock(&output_mutex);
            return;
        }
    
        if (info->return_value < 0)
            ft_printf(") = -1 %s\n", get_error_name(info->return_value));
        else
        {
            if (info->syscall_numb == 9 ||                  // mmap
                info->syscall_numb == 12 ||                 // brk
                info->syscall_numb == 59 ||                 // execve
                info->syscall_numb == 11 ||                 // munmap (a veces)
                info->syscall_numb == 25 ||                 // mremap
                info->syscall_numb == 192 ||                // mmap2 (32-bit)
                (info->return_value > 0xffffffff))          // Valores muy grandes
            {
                ft_printf(") = %p\n", (void *)info->return_value);
            }
            else
                ft_printf(") = %d\n", (int)info->return_value);
        }
    }
    pthread_mutex_unlock(&output_mutex);
}