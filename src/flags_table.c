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