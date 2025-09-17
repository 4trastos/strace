#include "incl/ft_strace.h"
#include "lib/printf/ft_printf.h"

int main(int argc, char **argv)
{
    pid_t   pid;
    int     status;

    if (argc < 2)
    {
        ft_printf("Use: %s <comand> [args...]\n", argv[0]);
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
        
        // Paso 2: El hijo se marca a sí mismo para ser traceado por su padre.
        // PTRACE_TRACEME hace que el kernel notifique al padre de eventos como execve().
        // Manejar el error si ptrace() falla.
        if (ft_ptrace(PTRACE_TRACEME, pid, NULL, NULL) == -1)
        {
            ft_printf("Error: ptrace TRACEME ( %s )\n", errno);
            exit(1);
        }

        // Paso 3: El hijo se reemplaza con el programa del usuario.
        // La familia de funciones exec (en este caso execvp) se usa para cargar
        // y ejecutar el nuevo programa, manteniendo el mismo PID.
        // El argv+1 es para pasar el comando y sus argumentos, ignorando nuestro
        // propio nombre de programa (ft_strace).
        execvp(argv[1], &argv[1]);

        // Si execvp() regresa, significa que hubo un error (el programa no se pudo ejecutar).
        ft_printf("Error: execvp ( %s )\n", errno);
        exit(1);
    }
    else
    {
        // Estamos en el proceso padre (nuestro ft_strace).

        // El padre espera a que el hijo se detenga después del execve.
        // WNOHANG para evitar que se bloquee si el hijo ya ha salido.
        // WUNTRACED para que nos notifique cuando el hijo se detiene por ptrace.
        waitpid(pid, &status, WUNTRACED);

        // Paso 4: Bucle principal de traceo.
        // Continúa mientras el proceso hijo no haya terminado.
        while (1)
        {
            // Paso 5: El padre le dice al hijo que continúe hasta la siguiente llamada al sistema.
            // PTRACE_SYSCALL hace que el hijo se detenga al entrar y salir de cada syscall.
            // Maneja el error si ptrace() falla.
            if (ft_ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1)
            {
                if (errno == ESRCH)
                    break;
                ft_printf("Error: ptrace SYSCALL ( %s)\n", errno);
                break;
            }

            // El padre espera a que el hijo se detenga de nuevo.
            waitpid(pid, &status, 0);

            // Verifica si el hijo ha terminado su ejecución.
            if (WIFEXITED(status) || WIFSIGNALED(status))
                break;      // Si el hijo terminó, salimos del bucle.
            
            // Paso 6: Obtiene los registros para decodificar la syscall.
            // Para esto, se necesita una estructura para guardar los registros.
            // struct user_regs_struct regs;
            // ptrace(PTRACE_GETREGS, pid, NULL, &regs);

            // Paso 7: Decodificar la syscall y sus argumentos.
            // Aquí es donde se necesita la tabla de syscalls para mapear
            // el número de syscall a su nombre y leer los argumentos.
            
            // Paso 8: Imprime la salida formateada.
            // printf("syscall_name(arg1, arg2, ...) = return_value\n");

            // No olvidar manejar las señales que recibe el proceso hijo.
            // Se puede usar ptrace(PTRACE_GETSIGINFO, ...) para obtener detalles
            // de la señal y reportarla.
        }
        
    }

    return (0);
}