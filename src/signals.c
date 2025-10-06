#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

void print_blocked_signals(void)
{
    sigset_t current;
    sigprocmask(SIG_SETMASK, NULL, &current); // leer máscara actual
    printf("Blocked signals:\n");
    for (int sig = 1; sig < 32; sig++) {
        if (sigismember(&current, sig))
            printf("  %d (%s)\n", sig, strsignal(sig));
    }
    printf("----\n");
}

void    unblock_signals(void)
{
    sigset_t    empty;
    
    sigemptyset(&empty);
    sigprocmask(SIG_SETMASK, &empty, NULL);
}

void    block_critical_signals(void)
{
    sigset_t    blocked;

    sigemptyset(&blocked);
    sigaddset(&blocked, SIGHUP);
    sigaddset(&blocked, SIGINT);
    sigaddset(&blocked, SIGQUIT);
    sigaddset(&blocked, SIGPIPE);
    sigaddset(&blocked, SIGTERM);
    sigprocmask(SIG_BLOCK, &blocked, NULL);
}
