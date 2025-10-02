#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"
#include <unistd.h>
#include <fcntl.h>

void    ft_read_string_from_mem(pid_t pid, unsigned long addr, char *buffer, size_t max_len)
{
    char    path[256];
    char    *pid_str;
    //char    *addr_str;
    int     fd;
    size_t  i;
    ssize_t bytes_read;

    if (max_len == 0 || addr == 0)
    {
        buffer[0] = '\0';
        return;
    }

    if (addr > 0x7fffffffffffUL)  // Direcciones muy altas probablemente inválidas en userspace
    {
        ft_strlcpy(buffer, "[invalid address]", max_len);
        return;
    }
    
    // 1. Abrir /proc/<pid>/mem
    ft_strcpy(path, "/proc/");
    pid_str = ft_itoa(pid);
    if (!pid_str)
    {
        ft_printf("[Error: ft_itoa failed]\n");
        return;
    }
    ft_strlcat(path, pid_str, sizeof(path));
    ft_strlcat(path, "/mem", sizeof(path));
    free(pid_str);

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        ft_printf("[Error ft_read_string: opening %s: ( %s )]\n", path, strerror(errno));
        buffer[0] = '\0';
        return;
    }

    // 2. Leer hasta max_len - 1 bytes
    //bytes_read = pread(fd, buffer, max_len - 1, (off_t)addr);
    bytes_read = -1;
    if (lseek(fd, (off_t)addr, SEEK_SET) != (off_t)-1)
         bytes_read = read(fd, buffer, max_len - 1);
    close(fd);

    if (bytes_read <= 0)
    {
        // Si falla la lectura, mostrar la dirección en formato hexadecimal
        ft_strcpy(buffer, "0x");
        unsigned long temp_addr = addr;
        char hex_chars[] = "0123456789abcdef";
        char hex_buffer[17]; // Para 64-bit: 16 chars + null
        int pos = 16;
        
        hex_buffer[pos] = '\0';
        while (pos > 0)
        {
            hex_buffer[--pos] = hex_chars[temp_addr & 0xf];
            temp_addr >>= 4;
            if (temp_addr == 0)
                break;
        }
        
        ft_strlcat(buffer, &hex_buffer[pos], max_len);
        return;
    }

    // 3. Asegurar terminación nula
    buffer[bytes_read] = '\0';

    // 4. Buscar el primer nulo y truncar si es necesario
    i = 0;
    while (i < (size_t)bytes_read)
    {
        if (buffer[i] == '\0')
            return; // String válido terminado con null
        i++;
    }
    
    // Si no encontramos nulo en los bytes leídos, truncar
    if (max_len > 0)
        buffer[max_len - 1] = '\0';
}

void    ft_read_buffer_from_mem(pid_t pid, unsigned long addr, size_t len, char *buffer, size_t max_len)
{
    char    path[256];
    char    *pid_str;
    int     fd;
    size_t  bytes_to_read;
    ssize_t bytes_read;

    if (max_len == 0 || addr == 0)
        return;

    // Determinar cuántos bytes leer: el mínimo entre la longitud del syscall (len) y
    // el espacio disponible en el buffer local (max_len - 1).
    bytes_to_read = len;
    if (len > max_len - 1)
        bytes_to_read = max_len - 1;

    // 1. Abrir /proc/<pid>/mem
    ft_strcpy(path, "/proc/");
    pid_str = ft_itoa(pid);
    if (!pid_str)
    {
        ft_printf("%p", (void *)addr);
        return;
    }
    ft_strlcat(path, pid_str, sizeof(path));
    ft_strlcat(path, "/mem", sizeof(path));
    free(pid_str);

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
         ft_printf("%p", (void *)addr);
         return;
    }

    // 2. Leer la cantidad exacta de bytes_to_read
    //bytes_read = pread(fd, buffer, bytes_to_read, (off_t)addr);
    bytes_read = -1;
    if (lseek(fd, (off_t)addr, SEEK_SET) != (off_t)-1)
        bytes_read = read(fd, buffer, bytes_to_read);
    close(fd);
    
    if (bytes_read <= 0)
    {
        ft_printf("%p", (void *)addr);
        return;
    }

    buffer[bytes_read] = '\0'; // Asegurar terminación para ft_printf
    ft_printf("\"");

    // Impresión con escape de caracteres
    for (size_t i = 0; i < (size_t)bytes_read; i++)
    {
        if (buffer[i] == '\0') // Detener si se encuentra un byte nulo
            break;
        if (buffer[i] == '\n')
            ft_printf("\\n");
        else if (buffer[i] == '\t')
            ft_printf("\\t");
        else if (buffer[i] >= 32 && buffer[i] <= 126)       // Caracteres imprimibles
            ft_printf("%c", buffer[i]);
        else
            ft_printf("\\%03o", (unsigned char)buffer[i]);  // Caracteres no imprimibles en octal
    }
    ft_printf("\"");

    // Si la longitud real del buffer es mayor a lo que leímos, añadir el truncado
    if (len > (size_t)bytes_read)
        ft_printf("...");
}