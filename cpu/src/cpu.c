#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();

    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();

    //liberar los logs y config
    finalizar_modulo(cpu_logger, cpu_logs_obligatorios, valores_config_cpu->config);
    free(valores_config_cpu);
    
    //finalizar las conexiones
    close(fd_cpu_dispatch);
    close(fd_cpu_interrupt);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    close(fd_memoria);
    
    return 0;
}

