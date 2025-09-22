#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <stdbool.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ptrace.h>
# include <sys/reg.h>
# include <sys/wait.h>
# include <sys/user.h>
# include <sys/uio.h>
# include <errno.h>
# include <signal.h>
# include <fcntl.h>
# include <elf.h>

# define    ARCH_64 64
# define    ARCH_32 32

# ifdef __x86_64__
struct user_regs_struct_32
{
    uint32_t    ebx;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    ebp;
    uint32_t    eax;
    uint32_t    xds;
    uint32_t    xes;
    uint32_t    xfs;
    uint32_t    xgs;
    uint32_t    orig_eax;
    uint32_t    eip;
    uint32_t    xcs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    xss;
};
# endif

typedef struct s_syscall_info
{
    int     arch;
    long    syscall_numb;
    long    return_value;
    long    arguments[6];
    char    *path;
    char    **command_path;
    char    *binary;
}   t_syscall_info;

typedef struct s_signals
{
    /* data */
}   t_signals;


//*** CPU logic ***/

char    *ft_findpath(char **envp);
int     etect_arch(const char *path);

//*** comunications & signals ***/

void    reading_regs(pid_t pid, t_syscall_info *syscall_info);
/* void    reading_signals(siginfo_t *siginfo, t_signals *signals);
void    reading_return_value(struct user_regs_struct *regs, t_syscall_info *syscall_info);
 */
//*** auxiliary functions ***/

int     ft_strncmp(const char *s1, const char *s2, size_t len);
char    **ft_split(char *str, char c);
size_t  len(char *str);
char    *ft_strdup(char *str, int len);

#endif