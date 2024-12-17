#include <memoria.h>


int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    pthread_mutex_init(&mutex_lista_particiones, NULL);
    pthread_mutex_init(&mutex_memoria, NULL);
    pthread_mutex_init(&mutex_contextos, NULL);
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_con_FS();
    
    conectar_cpu();

    conectar_kernel();

    

    //liberar los logs y config

    finalizar_modulo(memoria_logger,memoria_log_obligatorios,valores_config_memoria->config);
    
    //para evitar SF
    if (valores_config_memoria) free(valores_config_memoria);
    if (memoria) free(memoria);
    if (lista_particiones) {
        list_destroy_and_destroy_elements(lista_particiones, free);
    }
    if (lista_contextos) {
        list_destroy_and_destroy_elements(lista_contextos, free);
    }

    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);
    close(fd_kernel);
    close(fd_cpu);
    
    list_destroy_and_destroy_elements(lista_particiones, free);
    free(memoria);

    return 0;
}
