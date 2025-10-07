#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

t_flag_entry g_prot_flags[] =
{
    {0x1, "PROT_READ"},
    {0x2, "PROT_WRITE"},
    {0x4, "PROT_EXEC"},
    {0x0, "PROT_NONE"},
    {0, NULL}
};

t_flag_entry g_map_flags[] =
{
    {0x1, "MAP_SHARED"},
    {0x2, "MAP_PRIVATE"},
    {0x10, "MAP_FIXED"},
    {0x20, "MAP_ANONYMOUS"},
    {0x40, "MAP_GROWSDOWN"},
    {0x100, "MAP_DENYWRITE"},
    {0, NULL}
};

t_flag_entry g_openat_flags[] =
{
    {O_RDONLY, "O_RDONLY"},
    {O_WRONLY, "O_WRONLY"},
    {O_RDWR, "O_RDWR"},
    {O_CREAT, "O_CREAT"},
    {O_EXCL, "O_EXCL"},
    {O_NOCTTY, "O_NOCTTY"},
    {O_TRUNC, "O_TRUNC"},
    {O_APPEND, "O_APPEND"},
    {O_NONBLOCK, "O_NONBLOCK"},
    {O_DSYNC, "O_DSYNC"},
    {O_FSYNC, "O_FSYNC"},
    {O_ASYNC, "O_ASYNC"},
    {O_DIRECTORY, "O_DIRECTORY"},
    {O_NOFOLLOW, "O_NOFOLLOW"},
    {O_CLOEXEC, "O_CLOEXEC"},
    {0, NULL}
};

t_flag_entry g_access_flags[] = 
{
    {R_OK, "R_OK"},
    {W_OK, "W_OK"},
    {X_OK, "X_OK"},
    {F_OK, "F_OK"},
    {0, NULL}
};

t_flag_entry g_ioctl_cmds[] =
{
    {0x5401, "TIOCGWINSZ"},
    {0x5402, "TIOCSWINSZ"},
    {0, NULL}
};

t_flag_entry g_clone_flags[] =
{
    {0x00000100, "CLONE_VM"},
    {0x00000200, "CLONE_FS"},
    {0x00000400, "CLONE_FILES"},
    {0x00000800, "CLONE_SIGHAND"},
    {0x00002000, "CLONE_PTRACE"},
    {0x00004000, "CLONE_VFORK"},
    {0x00008000, "CLONE_PARENT"},
    {0x00010000, "CLONE_THREAD"},
    {0x00020000, "CLONE_NEWNS"},
    {0x00040000, "CLONE_SYSVSEM"},
    {0x00080000, "CLONE_SETTLS"},
    {0x00100000, "CLONE_PARENT_SETTID"},
    {0x00200000, "CLONE_CHILD_CLEARTID"},
    {0x00400000, "CLONE_DETACHED"},
    {0x00800000, "CLONE_UNTRACED"},
    {0x01000000, "CLONE_CHILD_SETTID"},
    {0x02000000, "CLONE_NEWCGROUP"},
    {0x04000000, "CLONE_NEWUTS"},
    {0x08000000, "CLONE_NEWIPC"},
    {0x10000000, "CLONE_NEWUSER"},
    {0x20000000, "CLONE_NEWPID"},
    {0x40000000, "CLONE_NEWNET"},
    {0x80000000, "CLONE_IO"},
    {0, NULL}
};

t_flag_entry g_wait4_flags[] =
{
    {0x01, "WNOHANG"},
    {0x02, "WUNTRACED"},
    {0x08, "WCONTINUED"},
    {0x80000000, "__WCLONE"}
};