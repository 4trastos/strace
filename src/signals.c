#define _POSIX_C_SOURCE 200809L

#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

char  *get_signal_name(int signum)
{
    for (int i = 0; g_signals_table[i].name != NULL; i++)
    {
        if (g_signals_table[i].signum == signum)
            return (g_signals_table[i].name);
    }

    if (signum == SIGSEGV)
        return("SIGSEGV");
    else if (signum == SIGINT)
        return("SIGINT");
    else if (signum == SIGTERM)
        return("SIGTERM");
    else if (signum == SIGKILL)
        return("SIGKILL");
    else if (signum == SIGSTOP)
        return ("SIGSTOP");
    return ("UNKNOWN");
}

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
    (void) syscall_numb;
    long skip_list[] = {
        431,  // set_robust_list (en threads)
        158,  // arch_prctl (en threads)
        334,  // rseq (en threads)
        13,   // rt_sigaction (en threads)
        14,   // rt_sigprocmask (en threads)
        435,  // syscall_435
        9,    // mmap (en threads)
        318,  // getrandom (en threads)
        12,   // brk (en threads)
        34,   // pause (en threads)
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
    
    sigemptyset(&empty);                        // Inicializa a 0 con sigemptyset
    sigprocmask(SIG_SETMASK, &empty, NULL);     // SIG_SETMASK con &empty
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
    sigprocmask(SIG_BLOCK, &blocked, NULL);     // SIG_BLOCK con &blocked
}
