#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <stdbool.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ptrace.h>
# include <wait.h>
# include <errno.h>

//*** main functions ***/



//*** ptrace logic ***/

long    ft_ptrace(enum __ptrace_request op, pid_t pid, void *addr, void *data);

//*** strace logc ***/



//*** comunications & signals ***/



//*** auxiliary functions ***/

#endif