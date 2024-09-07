#include <kernel.h>

int main(int argc, char* argv[]) {
    //saludar("kernel");
    inicializar_kernel();

    conectar_cpu_dispatch();
    conectar_cpu_interrupt();
    conectar_memoria();
    
    //liberar los logs y config
    free(valores_config_kernel);

    //falta finalizar las conexiones
    return 0;
}




