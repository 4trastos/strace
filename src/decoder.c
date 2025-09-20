#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int detect_arch(const char *path)
{
    Elf64_Ehdr ehdr;
    int fd = open(path, O_RDONLY);
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

void reading_regs(pid_t pid, t_syscall_info *syscall_info)
{
    struct iovec            iov;
    struct user_regs_struct regs;

    iov.iov_base = &regs;
    iov.iov_len = sizeof(regs);

    if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
    {
        ft_printf("Error: GETREGSET ( %s )\n", strerror(errno));
        return;
    }

    if (syscall_info->arch == ARCH_64)
    {
        syscall_info->syscall_numb = regs.orig_rax;
        syscall_info->arguments[0] = regs.rdi;
        syscall_info->arguments[1] = regs.rsi;
        syscall_info->arguments[2] = regs.rdx;
        syscall_info->arguments[3] = regs.r10;
        syscall_info->arguments[4] = regs.r9;
        syscall_info->arguments[5] = regs.r8;
    }
    else
    {
        // 32bits
    }
}

/* void    reading_signals(siginfo_t *siginfo, t_signals *signals)
{

}

void    reading_return_value(struct user_regs_struct *regs, t_syscall_info *syscall_info)
{

} */