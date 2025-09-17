#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

int main(int argc, char **argv)
{
    t_registers registr;
    int         status;
    pid_t       pid;

    if (argc < 2)
    {
        ft_printf("❌ Use: %s <comand> [args...] ❌\n", argv[0]);
        return (1);
    }

    pid = fork();
    if (fork() == -1)
    {
        ft_printf("Error: Falied fork ( %s )\n", errno);
        exit(1);
    }

    if (pid == 0)
    {
        // Estamos en el proceso hijo (el programa que vamos a tracear).
        
        // Proceso hijo: no usa PTRACE_TRACEME.
        // Simplemente se detiene y espera a que el padre se apode de él.
        kill(getpid(), SIGSTOP);

        // Paso 2: El hijo se reemplaza con el programa del usuario.
        // La familia de funciones exec se usa para cargar
        // y ejecutar el nuevo programa, manteniendo el mismo PID.
        execvp(argv[1], &argv[1]);

        // Si execvp() regresa, significa que hubo un error (el programa no se pudo ejecutar).
        ft_printf("Error: execvp (errno %d)\n", errno);
        exit(1);
    }
    else
    {
        // Proceso padre: nuestro ft_strace.

        // Paso 3: El padre se apodera del proceso hijo.
        // PTRACE_SEIZE es la forma moderna de adjuntarse a un proceso.
        if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace SEIZE (errno %d)\n", errno);
            return (1);
        }

        // Se usa PTRACE_SETOPTIONS para habilitar opciones como
        // PTRACE_O_TRACESYSGOOD, que es fundamental para
        // distinguir las paradas de syscall de otras señales.
        if (ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD) == -1)
        {
            ft_printf("Error: ptrace SETOPTIONS (errno %d)\n", errno);
            return (1);
        }

        // El padre espera la parada inicial del hijo.
        waitpid(pid, &status, 0);

        // Paso 4: Bucle principal de traceo.
        while (1)
        {
            // Paso 5: El padre le dice al hijo que continúe hasta la siguiente llamada al sistema.
            // PTRACE_SYSCALL hace que el hijo se detenga al entrar y salir de cada syscall.
            if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1)
            {
                if (errno == ESRCH)
                    break;
                ft_printf("Error: ptrace SYSCALL (errno %d)\n", errno);
                break;
            }

            // El padre espera a que el hijo se detenga de nuevo.
            waitpid(pid, &status, 0);

            // Se verifica si el hijo ha terminado su ejecución.
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                ft_printf("+++ Exited with status %d +++\n", WEXITSTATUS(status));
                break;
            }

            // Aquí se debe usar PTRACE_GETREGSET para obtener los registros
            // y decodificar la llamada al sistema.
            // Para eso, se necesita una estructura para guardar los registros.
            // struct user_regs_struct regs;
            // ptrace(PTRACE_GETREGS, pid, NULL, &regs);

            // Y manejar las señales con PTRACE_GETSIGINFO
            // para cumplir con las reglas del proyecto.

            // Paso 7: Decodificar la syscall y sus argumentos.
            
            // Paso 8: Imprime la salida formateada.
            // ft_printf("syscall_name(arg1, arg2, ...) = return_value\n");
        }
    }

    return (0);
}