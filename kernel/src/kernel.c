#include <kernel.h>

int main(int argc, char* argv[]) {
    //saludar("kernel");
    inicializar_kernel();

    conectar_cpu_dispatch();
    conectar_cpu_interrupt();
    conectar_memoria();
    
    //liberar los logs y config
 
    finalizar_modulo(kernel_logger, kernel_logs_obligatorios,valores_config_kernel->config);
    free(valores_config_kernel);
    
    //finalizar las conexiones
    close(fd_cpu_dispatch);
    close(fd_cpu_interrupt);
    close(fd_memoria);


    return 0;
}




