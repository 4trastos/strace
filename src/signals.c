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

int should_skip_syscall(long syscall_numb)
{
    long skip_list[] = {
        431,  // set_robust_list
        -1
    };

    for (int i = 0; skip_list[i] != -1; i++)
    {
        if (syscall_numb == skip_list[i])
            return (1);
    }
    return (0);
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
