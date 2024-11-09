#include <kernel.h>


int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_archivo_configuracion>\n", argv[0]);
        return EXIT_FAILURE;
    }

 //./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]

    archivo_pseudocodigo_main = argv[1];
	tamanio_proceso_main = atoi(argv[2]);

	printf("Archivo pseudocodigo: %s\n", archivo_pseudocodigo_main);
    printf("Tamaño proceso: %d\n", tamanio_proceso_main);

    inicializar_kernel();

    conectar_cpu_dispatch();
    conectar_cpu_interrupt();
    conectar_memoria();
    
    cola_new = queue_create();
    cola_exec = queue_create();
    cola_blocked = queue_create();

    planificador_largoPlazo();
    planificador_cortoPlazo();
    //iniciar_proceso(archivo_pseudocodigo,tamanio_proceso);

    //liberar los logs y config
 
    finalizar_modulo(kernel_logger, kernel_logs_obligatorios,valores_config_kernel->config);
    free(valores_config_kernel);
    
    //finalizar las conexiones
    close(fd_cpu_dispatch);
    close(fd_cpu_interrupt);
    close(fd_memoria);


    return 0;
}

