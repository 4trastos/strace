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
