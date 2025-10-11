# Proyect-Sistemas-Operativos-ESIS

# Descripción
El proyecto se basa en implementa una mini shell en C++, similar a Bash, que permite ejecutar comandos del sistema, manejar pipes, redirecciones, procesos en segundo plano, alias, historial, y ejecución en paralelo mediante hilos (pthread).
Además, incluye gestión básica de memoria mediante sobrecarga de los operadores new y delete para mostrar estadísticas de asignación.

La shell fue desarrollada con el objetivo de simular un entorno de línea de comandos real, empleando programación del sistema en Linux (uso de fork(), exec(), dup2(), pipe(), pthread, señales, etc.).

# Requerimientos
Sistema operativo:

Linux o cualquier distribución UNIX (Ubuntu, Debian, Arch, etc.)

Dependencias:

Compilador compatible con C++17 o superior (g++)

Biblioteca de lectura de línea (libreadline)

Biblioteca de hilos (pthread)

Cabeceras estándar del sistema (unistd.h, sys/wait.h, etc.)

Instalación de dependencias (Ubuntu/Debian):
sudo apt update
sudo apt install g++ libreadline-dev

# Instrucciones
Ejecuta en la terminal dentro del directorio del proyecto:
g++ shell.cpp -o shell -lreadline -lpthread
./shell

# Comandos
| Comando                  | Descripción                                                                              |
| ------------------------ | ---------------------------------------------------------------------------------------- |
| `help`                   | Muestra la lista de comandos disponibles.                                                |
| `cd [dir]`               | Cambia el directorio actual. Si se omite, va al `HOME`.                                  |
| `pwd`                    | Muestra el directorio de trabajo actual.                                                 |
| `ls`                     | Lista los archivos del directorio actual (usa implementación propia o `ls` del sistema). |
| `echo [texto]`           | Imprime texto o redirige la salida (`>`, `>>`).                                          |
| `clear`                  | Limpia la pantalla.                                                                      |
| `exit` o `salir`         | Sale de la shell.                                                                        |
| `history`                | Muestra el historial de comandos ingresados.                                             |
| `alias nombre = comando` | Define o lista alias personalizados.                                                     |
| `meminfo`                | Muestra el conteo de asignaciones y liberaciones de memoria.                             |
| `parallel cmd1 cmd2 ...` | Ejecuta múltiples comandos en paralelo usando hilos.                                     |

# Caracteristicas
| Funcionalidad                                      | Descripción                                                         |
| -------------------------------------------------- | ------------------------------------------------------------------- | 
| Ejecución de comandos externos (`ls`, `cat`, etc.) | Se ejecutan mediante `fork()` y `execvp()`                          | 
| Redirección de entrada y salida (`>`, `>>`, `<`)   | Implementada con `dup2()` y `open()`                                |
| Manejo de señales (`SIGINT`, `SIGCHLD`)            | Ignora Ctrl+C en la shell principal y detecta finalización de hijos |
| Historial de comandos                              | Implementado con `readline` y vector interno                        |
| Cambiar de directorio (`cd`)                       | Implementado con `std::filesystem::current_path()`                  |

# Extras
| Funcionalidad                                   | Descripción                                                   |       
| ----------------------------------------------- | ------------------------------------------------------------- |
| Ejecución en paralelo (`parallel`)              | Usa `pthread_create()` para ejecutar comandos simultáneamente | 
| Pipes (`    `)                                  | Comunicación entre procesos mediante `pipe()`                 |
| Alias personalizados (`alias nombre = comando`) | Expansión de alias antes de ejecutar comandos                 |
| Ejecución en segundo plano (`&`)                | Permite continuar la shell sin esperar al proceso hijo        | 
| Redirección múltiple combinada (`>, >>, <`)     | Redirecciones encadenadas en comandos                         |
| Contador de memoria (`meminfo`)                 | Muestra cuántas asignaciones y liberaciones se realizaron     |
| Expansión de `~` (HOME) en `cd`                 | Compatible con rutas como `~/Documentos`                      |
| Limpieza visual (`clear`)                       | Soporte multiplataforma (Linux y Windows)                     |

# Ejemplos de uso
pwd
/home/usuario

cd ~/Documentos
ls > archivos.txt

echo "Hola Mundo" >> salida.txt
cat < salida.txt

ls | grep cpp

parallel ls pwd whoami

alias ll = ls -l
ll

meminfo

# Manejo de memoria
void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
