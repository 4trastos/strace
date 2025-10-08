#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int should_skip_process_syscall(pid_t pid, long syscall_numb, pid_t main_pid)
{
    // Si no es el proceso principal, filtrar muchas syscalls
    if (pid != main_pid)
    {
        long allowed_syscalls[] = {
            56,   // clone
            231,  // exit_group
            60,   // exit
            61,   // wait4
            14,   // rt_sigprocmask
            13,   // rt_sigaction
            -1
        };
        
        // Verificar si esta syscall está permitida para procesos hijos
        for (int i = 0; allowed_syscalls[i] != -1; i++)
        {
            if (syscall_numb == allowed_syscalls[i])
                return 0; // No saltar - está permitida
        }
        
        // Para procesos hijos, filtrar casi todo
        return 1;
    }
    
    // Para el proceso principal, usar la lista normal
    return should_skip_syscall(syscall_numb);
}

int is_thread_process(pid_t pid)
{
    char path[256];
    char buffer[1024];
    int fd;
    ssize_t bytes_read;
    
    // Construir path manualmente sin convertir PID a string
    if (pid < 10)
    {
        if (pid == 0) ft_strcpy(path, "/proc/0/status");
        else if (pid == 1) ft_strcpy(path, "/proc/1/status");
        else if (pid == 2) ft_strcpy(path, "/proc/2/status");
        else return 0;
    }
    else
        return (pid > getpid() + 1);
    
    fd = open(path, O_RDONLY);
    if (fd == -1)
        return 0;
    
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        char *tgid_line = strstr(buffer, "Tgid:");
        if (tgid_line) {
            int tgid;
            if (sscanf(tgid_line, "Tgid:\t%d", &tgid) == 1)
                return (tgid != pid);
        }
    }
    
    return 0;
}

int detect_process_arch(pid_t pid)
{
    struct iovec iov;
    struct user_regs_struct regs64;
    
    iov.iov_base = &regs64;
    iov.iov_len = sizeof(regs64);
    
    if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == 0) {
        return ARCH_64;
    }
    
    return ARCH_32;
}