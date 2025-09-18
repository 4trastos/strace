El por qué `ptrace` es una herramienta tan poderosa y, al mismo tiempo, un riesgo de seguridad si no se maneja correctamente.

La capacidad de `ptrace` para **inyectar código** no es una función directa, sino una consecuencia de las tres acciones que te permite realizar sobre un proceso:

1.  **Leer y escribir en su memoria.**
2.  **Leer y modificar sus registros de la CPU.**
3.  **Controlar su ejecución paso a paso.**

Combinando estas tres capacidades, un atacante puede tomar el control completo del proceso y forzarlo a ejecutar cualquier código que desee.

### ¿Cómo se inyecta código con ptrace?

El proceso de inyección de código con `ptrace` es similar a cómo un depurador coloca un *breakpoint* o a cómo se implementa una herramienta como `strace`. Los pasos son los siguientes, desde el punto de vista de un atacante:

#### 1. Leer los registros
Lo primero es adjuntarse al proceso objetivo usando `PTRACE_SEIZE` y detenerlo. Una vez detenido, el atacante lee los registros de la CPU con `PTRACE_GETREGS`.  Esto es crucial porque los registros contienen el estado actual del proceso, incluyendo el **puntero de instrucción (`RIP`)**. El puntero de instrucción es el registro más importante en este ataque, ya que le dice a la CPU dónde está la siguiente instrucción a ejecutar.

#### 2. Escribir el código malicioso
Un atacante encuentra una región de memoria writable (con permisos de escritura) en el proceso objetivo. Luego, usa `ptrace` con la opción de escritura de memoria (por ejemplo, `PTRACE_POKEDATA`) para copiar un pequeño programa malicioso, conocido como **shellcode**, en esa región. Este shellcode puede ser tan simple como un código para abrir una shell o tan complejo como un virus.

#### 3. Secuestrar el flujo de ejecución
Aquí es donde la magia ocurre. Una vez que el shellcode está en la memoria del proceso objetivo, el atacante usa `ptrace` para modificar el registro **`RIP`** y cambiar su valor. En lugar de que `RIP` apunte a la siguiente instrucción del programa legítimo, el atacante lo modifica para que apunte al inicio del shellcode que acaba de inyectar.

#### 4. Continuar la ejecución
Finalmente, el atacante le dice al proceso objetivo que continúe su ejecución (`PTRACE_CONT`). La CPU del proceso, sin saber que ha sido manipulada, simplemente sigue la instrucción que ahora está en `RIP` y comienza a ejecutar el shellcode inyectado.

### Un ejemplo simplificado

Imagina que un atacante quiere inyectar un shellcode en un proceso `victim`.
1.  El atacante se adjunta a `victim`.
2.  El atacante encuentra una dirección de memoria libre en `victim` y escribe su shellcode allí.
3.  El atacante usa `ptrace` para cambiar el registro `RIP` de `victim` para que apunte a la dirección de memoria donde se ha escrito el shellcode.
4.  El atacante le dice a `victim` que continúe. `victim` ejecuta el código malicioso.

### ¿Por qué la seguridad del kernel es tan estricta?

Debido a esta capacidad, el kernel de Linux implementa varias medidas de seguridad para restringir el uso de `ptrace`. La principal es el **`ptrace_scope`**, una medida que limita quién puede depurar a quién.

* Normalmente, solo un proceso padre puede adjuntarse a su proceso hijo.
* Para adjuntarse a un proceso que ya está corriendo (como tu shell `bash`), el usuario debe tener permisos de superusuario (`root`).
* Algunos sistemas lo restringen aún más, permitiendo la depuración solo a procesos que ya son propiedad del mismo usuario y no tienen permisos elevados.

En tu caso, `strace` es un programa de depuración legítimo, pero sigue las mismas reglas. No puedes adjuntarte a un shell porque es un proceso del sistema y la operación no está permitida para evitar que un proceso cualquiera inyecte código en tu terminal o en otros programas importantes.