#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    printf("Iniciando programa 32-bit...\n");
    
    // Varios syscalls para probar
    int fd = open("/etc/passwd", O_RDONLY);
    if (fd > 0) {
        char buffer[100];
        read(fd, buffer, 10);
        close(fd);
    }
    
    getpid();
    getuid();
    
    printf("Finalizando programa 32-bit\n");
    return 0;
}