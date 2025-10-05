#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void    unblock_signals(void)
{
    sigset_t    empty;
    
    sigemptyset(&empty);
    sigprocmask(SIG_SETMASK, &empty, NULL);
}