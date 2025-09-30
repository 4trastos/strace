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

typedef enum
{
    VOID,
    INT,
    STRING,
    POINTER,
}   e_arg_type;

typedef struct s_flag_entry
{
    long        value;
    const char  *name;
}   t_flag_entry;

typedef struct s_error_entry
{
    int         err_num;
    const char  *err_name;
}   t_error_entry;

typedef struct s_syscall_entry
{
    const char *name;
    e_arg_type  arg_types[6];
}   t_syscall_entry;

typedef struct s_signal_entry
{
    int         signum;
    const char *name;
}   t_signal_entry;

typedef struct s_syscall_info
{
    int         arch;
    long        syscall_numb;
    long        return_value;
    long        arguments[6];
    char        *path;
    char        **command_path;
    char        *binary;
}   t_syscall_info;

extern const t_syscall_entry g_syscall_table[];
extern const t_error_entry g_error_table[];
extern const t_flag_entry g_prot_flags[];
extern const t_flag_entry g_map_flags[];
extern const t_flag_entry g_openat_flags[];
extern const t_flag_entry g_access_flags[];
extern const t_flag_entry g_ioctl_cmds[];
extern const t_signal_entry g_signals_table[];


//*** CPU logic ***/

char        *ft_findpath(char **envp);
int         detect_arch(char *path);
char        *get_binary(char **command_path, char *command_arg);

//*** comunications & signals ***/

void        reading_entry_regs(pid_t pid, t_syscall_info *syscall_info);
void        reading_exit_regs(pid_t pid, t_syscall_info *syscall_info);
void        print_syscall_entry(pid_t pid, const t_syscall_info *info, const t_syscall_entry *entry);
void        print_syscall_exit(const t_syscall_info *info);
void        print_flags(long value, const t_flag_entry *flags);
void        ft_read_string_from_mem(pid_t pid, unsigned long addr, char *buffer, size_t max_len);
void        ft_read_buffer_from_mem(pid_t pid, unsigned long addr, size_t len, char *buffer, size_t max_len);
void        ft_read_argv(pid_t pid, unsigned long addr);
int         ft_read_word(pid_t pid, unsigned long addr, unsigned long *val);
const char  *get_error_name(long errnum);
const char  *get_signal_name(int signum);
void        free_syscall_info(t_syscall_info *info);
void        ft_free_split(char **str);

//*** auxiliary functions ***/

int         ft_strncmp(char *src, char *str, int numb);
char        **ft_split(char *str, char c);
size_t      ft_strlen(char *str);
char        *ft_strdup(char *str, int len);
char        *ft_strjoin(char *str1, char *str2);
char        *ft_strcpy(char *dst, const char *src);
char	    *ft_itoa(int number);
size_t      ft_strlcat (char *dst, char *src, size_t len);

#endif