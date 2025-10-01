#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

char  *get_signal_name(int signum)
{
    for (int i = 0; g_signals_table[i].name != NULL; i++)
    {
        if (g_signals_table[i].signum == signum)
            return (g_signals_table[i].name);
    }
    return ("UNKNOWN");
}

char    *get_binary(char **command_path, char *command_arg)
{
    char    *aux;
    char    *exe;

    while (*command_path)
    {
        aux = ft_strjoin(*command_path, "/");
        exe = ft_strjoin(aux, command_arg);
        free(aux);
        if (access(exe, F_OK) == 0)
            return (exe);
        free(exe);
        command_path++;
    }
    ft_printf("./ft_strace: Cannot find executable '%s'\n", command_arg);
    return (NULL);
}

char    *ft_findpath(char **envp)
{
    while (ft_strncmp("PATH", *envp, 4))
        envp++;
    return (*envp + 5);
}

int detect_arch(char *binary)
{
    Elf64_Ehdr  ehdr;
    if (!binary)
        return (-2);
    int fd = open(binary, O_RDONLY);
    if (fd < 0)
        return (-1);
    read(fd, &ehdr, sizeof(ehdr));
    close(fd);
    
    if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
        return (ARCH_64);
    else if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
        return (ARCH_32);
    return (-1);
}

void reading_entry_regs(pid_t pid, t_syscall_info *syscall_info)
{
    struct iovec                iov;
    struct user_regs_struct     regs;
    struct user_regs_struct_32  regs32;

    
    if (syscall_info->arch == ARCH_64)
    {
        iov.iov_base = &regs;
        iov.iov_len = sizeof(regs);
    
        if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
        {
            ft_printf("Error: GETREGSET 64-bit ( %s )\n", strerror(errno));
            return;
        }
        syscall_info->syscall_numb = regs.orig_rax;
        syscall_info->arguments[0] = regs.rdi;
        syscall_info->arguments[1] = regs.rsi;
        syscall_info->arguments[2] = regs.rdx;
        syscall_info->arguments[3] = regs.r10;
        syscall_info->arguments[4] = regs.r8;
        syscall_info->arguments[5] = regs.r9;
    }
    if (syscall_info->arch == ARCH_32)
    {
        iov.iov_base = &regs32;
        iov.iov_len = sizeof(regs32);

        if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
        {
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs32) == -1)
            //if (ptrace(PTRACE_GETREGSET, pid, NULL, &regs32) == -1)
            {
                ft_printf("Error: GETREGS 32-bit ( %s )\n", strerror(errno));
                return;
            }
        }
        syscall_info->syscall_numb = regs32.orig_eax;
        syscall_info->arguments[0] = regs32.ebx;
        syscall_info->arguments[1] = regs32.ecx;
        syscall_info->arguments[2] = regs32.edx;
        syscall_info->arguments[3] = regs32.esi;
        syscall_info->arguments[4] = regs32.edi;
        syscall_info->arguments[5] = regs32.ebp;
    }
}

void    reading_exit_regs(pid_t pid, t_syscall_info *syscall_info)
{
    struct iovec                iov;
    struct user_regs_struct     regs;
    struct user_regs_struct_32  regist;
    
    if (syscall_info->arch == ARCH_64)
    {
        iov.iov_base = &regs;
        iov.iov_len = sizeof(regs);

        if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
        {
            ft_printf("Error: GETREGSET ( %s )\n", strerror(errno));
            return;
        }
        syscall_info->return_value = regs.rax;
    }
    if (syscall_info->arch == ARCH_32)
    {
        iov.iov_base = &regist;
        iov.iov_len = sizeof(regist);

        if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
        {
            //if (ptrace(PTRACE_GETREGSET, pid, NULL, &regist) == -1)
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regist) == -1)
            {
                ft_printf("Error: GETREGS 32-bit ( %s )\n", strerror(errno));
                return;
            }
        }
        syscall_info->return_value = regist.eax;
    }
}
