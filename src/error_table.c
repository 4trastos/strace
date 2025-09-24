#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

const t_error_entry g_error_table[] =
{
    {0, "Success"},
    {EPERM, "EPERM"},
    {ENOENT, "ENOENT"},
    {EIO, "EIO"},
    {EAGAIN, "EAGAIN"},
    {EACCES, "EACCES"},
    {EBADF, "EBADF"},
    {EFAULT, "EFAULT"},
    {EINVAL, "EINVAL"},
    {ENOSYS, "ENOSYS"},
    {EEXIST, "EEXIST"},
    {0, NULL}
};
