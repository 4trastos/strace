# strace
`strace` es una herramienta de depuración para Linux que utiliza la función del kernel `ptrace` para monitorear las llamadas al sistema y las señales de un programa.

# ft_strace

> **Resumen del Proyecto:** Este proyecto tiene como objetivo recrear la herramienta de depuración `strace` de Linux, que monitoriza las llamadas al sistema y las señales de un programa. La herramienta se basa en la funcionalidad del kernel de Linux conocida como `ptrace`.

---

## Parte Obligatoria

Para la parte obligatoria, debes recrear el comando `strace` sin ninguna opción.

- El ejecutable debe llamarse **`ft_strace`**.
- Debes codificar en **C** y crear un **Makefile** que respete las reglas habituales.
- No hay restricciones de "norm" de 42, pero evita abusos.
- Es crucial manejar los errores con cuidado para que el programa no termine de forma inesperada (Segmentation fault, bus error, double free, etc.).
- El proyecto debe funcionar en una máquina virtual con un **kernel de Linux > 3.4**.
- Solo se te permite utilizar las siguientes opciones de `ptrace`:
  - `PTRACE_SYSCALL`
  - `PTRACE_GETREGSET`
  - `PTRACE_SETOPTIONS`
  - `PTRACE_GETSIGINFO`
  - `PTRACE_SEIZE`
  - `PTRACE_INTERRUPT`
  - `PTRACE_LISTEN`
- Debes manejar binarios tanto de **64 como de 32 bits**.
- **El manejo de señales es muy importante. Presta especial atención a ello.**
- El resultado en pantalla debe ser similar al `strace` original, pero no es necesario que sea una coincidencia exacta. No se permite llamar al `strace` real.

---

## Parte de Bonificación

La parte de bonificación será evaluada solo si la parte obligatoria es **perfecta**. Un trabajo "perfecto" significa que todas las funcionalidades obligatorias están completas y funcionan sin errores. Si no cumples con todos los requisitos obligatorios, el trabajo de bonificación no será considerado.

Los bonos disponibles son:

- La opción `-c`.
- Manejar la gestión del `PATH`.

---

## Entrega y Evaluación

- El proyecto debe ser entregado en tu repositorio de Git.
- Solo el trabajo dentro del repositorio será evaluado.
- El proyecto será corregido por humanos.
- Puedes organizar y nombrar tus archivos como desees, siempre y cuando sigas las reglas establecidas.

---

Este documento es una traducción del `subject` original de 42 y resume los puntos clave del proyecto.


# Guía de Estudio para ft_strace

## Resumen de `man strace`

`strace` es una herramienta de diagnóstico, depuración e instrucción del espacio de usuario para Linux. Es utilizada por un administrador o desarrollador para observar cómo un programa interactúa con el sistema operativo a través de las llamadas al sistema (syscalls).

### ¿Qué hace?
- **Traza llamadas al sistema:** Muestra las llamadas al sistema que un proceso realiza, incluyendo los argumentos, los valores de retorno y los errores.
- **Muestra señales:** Reporta las señales que el proceso recibe.
- **Detalles:** Permite ver el tiempo que tarda cada llamada, el número de bytes transferidos en operaciones de I/O, etc.
- **Sintaxis básica:** `strace [opciones] [comando]`

### ¿Por qué es útil?
- **Depuración:** Permite identificar la causa de fallos (ej. un programa que termina con un error de I/O).
- **Análisis de comportamiento:** Ayuda a entender el flujo de ejecución de un programa.
- **Seguridad:** Puede usarse para ver si un programa está accediendo a archivos o recursos de red inesperados.

---

## Resumen de `man ptrace`

`ptrace` es una llamada al sistema de Linux que permite a un proceso (el "tracer") controlar la ejecución de otro proceso (el "tracee"). Es la base de herramientas como `strace` y depuradores como `gdb`.

### ¿Qué hace?
- **Control de procesos:** Permite al proceso tracer inspeccionar y modificar la memoria y los registros del proceso tracee.
- **Notificación de eventos:** El tracee envía eventos al tracer, como la entrada o salida de una llamada al sistema, o la recepción de una señal.
- **Flujo de ejecución:** El tracer puede hacer que el tracee se detenga, continúe, y se ejecute paso a paso.

### Opciones de `ptrace` más relevantes para este proyecto:
- **`PTRACE_TRACEME`:** Un proceso se marca a sí mismo como traceado, permitiendo que su padre lo controle.
- **`PTRACE_SYSCALL`:** El tracee se detiene en cada entrada y salida de una llamada al sistema.
- **`PTRACE_GETREGS`/`PTRACE_GETREGSET`:** Permite al tracer leer los valores de los registros del tracee. Esto es crucial para leer los argumentos de las syscalls y los valores de retorno.
- **`PTRACE_SETOPTIONS`:** Configura opciones de ptrace, como por ejemplo, el rastreo de todos los hijos del tracee (`PTRACE_O_TRACEFORK`).
- **`PTRACE_SEIZE`:** Adjunta a un proceso sin detenerlo inmediatamente, lo que puede ser útil para evitar condiciones de carrera.
- **`PTRACE_LISTEN`:** Permite que un tracer se adjunte a un proceso sin interferir en su ejecución, solo para eventos futuros.
- **`PTRACE_GETSIGINFO`:** Obtiene información detallada sobre la señal que ha causado la detención del tracee.

---

# lista completa de filtros de errores para strace y para qué sirve cada uno:

## Filtros Básicos de Errores:

```bash
# Todas las llamadas que fallaron (retornaron -1)
grep "= -1" /tmp/debug.log

# Muestra el código de error específico
grep -o "errno [0-9]*" /tmp/debug.log

# Llamadas que fueron interrumpidas por señales
grep "EINTR" /tmp/debug.log
```

## Filtros por Códigos de Error Específicos:

```bash
# Error de permiso denegado
grep "EPERM" /tmp/debug.log        # Operation not permitted

# Acceso denegado
grep "EACCES" /tmp/debug.log       # Permission denied

# Archivo o directorio no existe
grep "ENOENT" /tmp/debug.log       # No such file or directory

# Entrada/salida error
grep "EIO" /tmp/debug.log          # Input/output error

# Dispositivo no disponible
grep "ENODEV" /tmp/debug.log       # No such device

# No hay espacio en dispositivo
grep "ENOSPC" /tmp/debug.log       # No space left on device

# Memoria insuficiente
grep "ENOMEM" /tmp/debug.log       # Out of memory

# Recurso temporalmente no disponible
grep "EAGAIN" /tmp/debug.log       # Resource temporarily unavailable

# Archivo existe
grep "EEXIST" /tmp/debug.log       # File exists

# No es un directorio
grep "ENOTDIR" /tmp/debug.log      # Not a directory

# Es un directorio
grep "EISDIR" /tmp/debug.log       # Is a directory

# Argumento inválido
grep "EINVAL" /tmp/debug.log       # Invalid argument

# Demasiados archivos abiertos
grep "EMFILE" /tmp/debug.log       # Too many open files

# Sistema de archivos de solo lectura
grep "EROFS" /tmp/debug.log        # Read-only file system

# Dispositivo o recurso ocupado
grep "EBUSY" /tmp/debug.log        # Device or resource busy

# Timeout excedido
grep "ETIMEDOUT" /tmp/debug.log    # Connection timed out

# Conexión rechazada
grep "ECONNREFUSED" /tmp/debug.log # Connection refused
```

## Comandos Avanzados para Análisis de Errores:

```bash
# Contar frecuencia de errores
grep -o "errno [0-9]*" /tmp/debug.log | awk '{print $2}' | sort | uniq -c | sort -nr

# Ver errores con su descripción
grep "= -1" /tmp/debug.log | head -20

# Errores en operaciones de red
grep -E "ECONN|ETIMEDOUT|ENETUNREACH" /tmp/debug.log

# Errores en operaciones de archivos
grep -E "ENOENT|EACCES|EROFS|ENOSPC" /tmp/debug.log

# Errores de memoria
grep -E "ENOMEM|EFAULT" /tmp/debug.log
```

## Script Útil para Analizar Errores:

```bash
#!/bin/bash
# Analizador de errores de strace
LOG_FILE="/tmp/debug.log"

echo "=== ANÁLISIS DE ERRORES EN STRACE ==="
echo ""

# Total de llamadas
total_calls=$(wc -l < "$LOG_FILE")
echo "Total de llamadas: $total_calls"

# Llamadas con error
error_calls=$(grep -c "= -1" "$LOG_FILE")
echo "Llamadas con error: $error_calls"
echo "Porcentaje de error: $(echo "scale=2; ($error_calls * 100) / $total_calls" | bc)%"
echo ""

# Top 10 errores más frecuentes
echo "Top 10 errores más frecuentes:"
grep -o "errno [0-9]*" "$LOG_FILE" | awk '{print $2}' | sort | uniq -c | sort -nr | head -10
echo ""

# Errores por tipo de llamada
echo "Errores por tipo de llamada:"
grep "= -1" "$LOG_FILE" | awk '{print $1}' | sort | uniq -c | sort -nr
```

## Para Capturar Mejor la Información de Errores:

```bash
# Usa strace con opciones que muestran mejor los errores
strace -e verbose=all -s 1024 -o /tmp/debug.log <binary>

# O para ver tiempos y errores
strace -T -e trace=all -o /tmp/debug.log <binary>
```

## Conversión de Códigos de Error:

Si quieres entender qué significa cada código de error:
```bash
# Usa errno para descifrar códigos
errno 13    # EACCES
errno 2     # ENOENT
errno 12    # ENOMEM

# O ver todos los códigos
errno -l
```
