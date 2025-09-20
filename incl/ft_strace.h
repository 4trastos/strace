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

typedef struct s_syscall_info
{
    int     arch;
    long    syscall_numb;
    long    return_value;
    long    arguments[6];
    char    *name;
}   t_syscall_info;

typedef struct s_signals
{
    /* data */
}   t_signals;



//*** main functions ***/

int detect_arch(const char *path);

//*** CPU logic ***/



//*** comunications & signals ***/

void    reading_regs(pid_t pid, t_syscall_info *syscall_info);
/* void    reading_signals(siginfo_t *siginfo, t_signals *signals);
void    reading_return_value(struct user_regs_struct *regs, t_syscall_info *syscall_info);
 */
//*** auxiliary functions ***/

#endif

/* 

### La estructura de registros

Para manejar registros de 64 bits de forma segura, es mejor usar la estructura `user_regs_struct` que viene en la cabecera `<sys/user.h>`.
Esta estructura está diseñada para este propósito y te evita tener que manejar los campos de los registros de forma manual. **No es necesario que recrees esta estructura por tu cuenta**.

Tu estructura `t_registers` es una abstracción, pero para ser funcional, necesitas que contenga los registros del procesador. El `ptrace` te da acceso a ellos.

-   **`int bits`**: Este campo es útil si quieres manejar binarios de 32 y 64 bits. En `x86-64`, un registro de 32 bits es un subconjunto de uno de 64 bits. Puedes usar este campo para saber qué tipo de binario estás inspeccionando y adaptar la forma en que lees y muestras los registros.
-   **`size_t num`**: Este campo podría usarse para almacenar el número de la llamada al sistema (`syscall`). El número de la `syscall` es el valor del registro `rax` (o `eax` en 32 bits) cuando se realiza la llamada.
-   **`unsigned char *ptr`**: Este puntero es demasiado genérico. Es mejor que uses un puntero a la estructura que contendrá los registros reales, como `struct user_regs_struct`.

La estructura que necesitas para leer los registros con `ptrace` es `struct user_regs_struct`. 

### Lo que deberías buscar y guardar

La información clave que debes buscar y guardar para cada llamada al sistema es la siguiente:

1.  **Número de la llamada al sistema (`syscall number`):** Se encuentra en el registro `rax` (o `eax` para 32 bits). Este es el identificador único de la función del kernel que se está invocando, como `open`, `read`, `write`, `close`, etc.
2.  **Argumentos de la llamada al sistema (`syscall arguments`):** Se encuentran en los registros `rdi`, `rsi`, `rdx`, `rcx`, `r8`, y `r9` (en ese orden para los primeros seis argumentos en 64 bits). Estos valores son cruciales para entender qué datos se están pasando a la `syscall`. Si un argumento es un puntero a una cadena de caracteres (como en `open("ruta", ...)`), tendrás que usar `ptrace(PTRACE_PEEKDATA, ...)` para leer esa cadena.
3.  **Valor de retorno de la llamada al sistema (`return value`):** Una vez que la `syscall` ha terminado, el valor de retorno se encuentra de nuevo en el registro `rax`. Este es el resultado de la operación (por ejemplo, el descriptor de archivo de `open` o el número de bytes leídos en `read`).
4.  **Estado de la señal (`signal status`):** `ptrace` a menudo detiene el proceso hijo por señales (como `SIGTRAP`). Necesitas verificar el `status` que devuelve `waitpid` para saber si la detención se debe a una `syscall` (`WSTOPSIG & 0x80`) o a otra señal. Si es una señal, `PTRACE_GETSIGINFO` te dará más información sobre ella.

Un enfoque práctico es usar una estructura para almacenar la información de una llamada al sistema en particular, como su número, sus argumentos y el valor de retorno. Luego, dentro del bucle principal de tu `strace`, puedes leer los registros, rellenar tu estructura y, finalmente, imprimir la información. */