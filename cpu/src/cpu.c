#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();

    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();
    
    //liberar los logs y config
    free(valores_config_cpu);
    return 0;
}

