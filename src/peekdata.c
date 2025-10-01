#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int ft_read_word(pid_t pid, unsigned long addr, unsigned long *val)
{
    char    path[256];
    char    *pid_str;
    int     fd;
    ssize_t bytes_read;
    
    // 1. Construir la ruta /proc/<pid>/mem
    pid_str = ft_itoa(pid);
    if (!pid_str)
    {
        *val = 0;
        return (-1);
    }
    ft_strcpy(path, "/proc/");
    ft_strlcat(path, pid_str, sizeof(path));
    ft_strlcat(path, "/mem", sizeof(path));
    free(pid_str);

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        *val = 0;
        return (-1);
    }

    // 2. Leer 8 bytes (sizeof(unsigned long) en 64-bit) desde la dirección 'addr'
    bytes_read = pread(fd, val, sizeof(unsigned long), (off_t)addr);
    close(fd);
    if (bytes_read !=  sizeof(unsigned long))
    {
        *val = 0;
        return (-1);
    }

    return (0);
}

void    ft_read_argv(pid_t pid, unsigned long addr)
{
    unsigned long   ptr_addr;
    unsigned long   string_addr;
    char            buffer[256];
    int             i;
    int             result;

    i = 0;
    ft_printf("[");

    // Bucle para leer punteros del vector (cada puntero ocupa sizeof(unsigned long) bytes)
    while (1)
    {
        ptr_addr = addr + (i * sizeof(unsigned long));

        result = ft_read_word(pid, ptr_addr, &string_addr);
        if (result == -1)
        {
            if (i == 0)
                ft_printf("/* Error reading vector */");
            break;
        }

        // Si el puntero leído es NULL (0), hemos llegado al final del vector (convención argv/envp)
        if (string_addr == 0)
        {
            if (i == 0)
                ft_printf("NULL");
            else if (i > 20)
                ft_printf("/* %d vars */", i);
            else
                ft_printf(", NULL");
            break;
        }

        // Si no es el primer elemento, imprimimos la coma y el espacio
        if (i > 0)
            ft_printf(", ");
        
        // El valor leído (string_addr) es la dirección de la cadena real
        ft_read_string_from_mem(pid, string_addr, buffer, sizeof(buffer));
        ft_printf("\"%s\"", buffer);

        i++;
    }
    
    ft_printf("]");
}
