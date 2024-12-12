#include <memoria.h>

int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    sem_t mutex_memoria;
    sem_init(&mutex_memoria, 0, 1);

    conectar_con_FS();
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_cpu();

    conectar_kernel();

    conectar_con_FS();

    //liberar los logs y config

    finalizar_modulo(memoria_logger,memoria_log_obligatorios,valores_config_memoria->config);
    
    free(valores_config_memoria);
    free(memoria);
    free(lista_particiones);

    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);
    close(fd_kernel);
    close(fd_cpu);
    
    list_destroy_and_destroy_elements(lista_particiones, free);
    free(memoria);

    return 0;
}
