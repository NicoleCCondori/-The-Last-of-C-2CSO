#include <kernel.h>

int main(int argc, char* argv[]) {
    //saludar("kernel");
    inicializar_kernel();

    conectar_cpu_dispatch();
    conectar_cpu_interrupt();
    conectar_memoria();
    
<<<<<<< HEAD
=======
    cola_new = queue_create();
    cola_exec = queue_create();
    cola_blocked = queue_create();

    planificador_largoPlazo();
    planificador_cortoPlazo();
    //iniciar_proceso(archivo_pseudocodigo,tamanio_proceso);

>>>>>>> origin/checkpoint3
    //liberar los logs y config
    free(valores_config_kernel);

    //falta finalizar las conexiones
    return 0;
}




