#include <kernel.h>


int main(int argc, char* argv[]) {

    /*if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_archivo_configuracion>\n", argv[0]);
        liberar_recursos_kernel();
        return EXIT_FAILURE;
    }*/

 //./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]

    //archivo_pseudocodigo_main = argv[1];
	//tamanio_proceso_main = atoi(argv[2]);

    archivo_pseudocodigo_main = "MEM_FIJAS";
	tamanio_proceso_main = 20;
    
    inicializar_kernel();

    log_info(kernel_logger, "Archivo pseudocodigo: %s\n", archivo_pseudocodigo_main);
    log_info(kernel_logger, "Tamaño proceso: %d\n", tamanio_proceso_main);
    conectar_memoria();
    conectar_cpu_dispatch();
    conectar_cpu_interrupt();

    
    int prioridad_hilo_main = 0;
    
    crear_proceso(tamanio_proceso_main, archivo_pseudocodigo_main, prioridad_hilo_main);

    planificador_largoPlazo();
    planificador_cortoPlazo();

    //Liberamos todos los recursos
    liberar_recursos_kernel();
    
    return 0;
}



