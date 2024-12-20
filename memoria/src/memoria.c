#include <memoria.h>


int main(int argc, char* argv[]) {


    if (inicializar_memoria() != 0) {
        log_error(memoria_logger, "Fallo al inicializar memoria");
        liberar_recursos();
        return -1;
    }

    pthread_mutex_init(&mutex_lista_particiones, NULL);
    pthread_mutex_init(&mutex_memoria, NULL);
    pthread_mutex_init(&mutex_contextos, NULL);
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_con_FS();
    conectar_cpu();
    conectar_kernel();

    liberar_recursos();

    return 0;
}