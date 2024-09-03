#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    logger_memoria = iniciar_logger("memoria.log", "MEMORIA");
    log_info(logger_memoria, "Se creo exitosamente el logger de memoria");

    levantar_config_memoria("src/memoria.config");
    
    iniciar_conexion_con_FS();

    //finalizar_conexiones(1, cliente_memoria);

    finalizar_modulo(logger_memoria, config);

    free(config_memoria);

    return EXIT_SUCCESS;

}


void levantar_config_memoria(char* config_path){
    t_config* config = iniciar_configs(config_path);

    t_config_memoria* config_memoria;
    config_memoria = malloc(sizeof(t_config_memoria));

    if (config == NULL){
        log_info(logger_memoria, "Error al levantar el Config");
        exit(1);
    }

    char* puerto_FS = config_get_string_value(config,"PUERTO_FILESYSTEM");
    char* ip_FS = config_get_string_value(config,"IP_FILESYSTEM");

    printf(puerto_FS);
    printf(ip_FS);

    /**
    config_memoria->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
    config_memoria->ip_filesystem = config_get_string_value(config,"IP_FILESYSTEM");
    config_memoria->puerto_filesystem = config_get_string_value(config,"PUERTO_FILESYSTEM");
    config_memoria->tam_memoria = config_get_int_value(config,"TAM_MEMORIA");
    config_memoria->path_instrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    config_memoria->retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    config_memoria->esquema = config_get_string_value(config,"ESQUEMA");
    config_memoria->algoritmo_busqueda = config_get_string_value(config,"ALGORITMO_BUSQUEDA");
    config_memoria->particiones = config_get_array_value(config,"PARTICIONES");
    config_memoria->log_level = config_get_string_value(config,"LOG_LEVEL");*/
}


void iniciar_conexion_con_FS(){

    /**char* puerto_FS= string_itoa(config_memoria->puerto_filesystem);
    char* IP_FS= string_itoa(config_memoria->ip_filesystem);*/

    
    int cliente_memoria = crear_conexion(ip_FS , puerto_FS, "FILESYSTEM",memoria_logger);

    memoria_logger = iniciar_logger(".//memoria.log","log_MEMORIA");

    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_MEMORIA");

    memoria_config = iniciar_configs("src/memoria.config");
    
    PUERTO_ESCUCHA = config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    
    //Servidor MEMORIA
    fd_memoria = iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria iniciada ");
    //printf("fd_memoria: %d\n", fd_memoria);
    
    //Esperar conexion CPU
    log_info(memoria_logger,"Esperando CPU...");
    fd_cpu = esperar_cliente(fd_memoria,memoria_logger,"CPU");
    handshakeServer(fd_cpu);
    printf("fd_cpu: %d\n", fd_cpu);

    //se crea un hilo para escuchar mensajes de CPU
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, (void*)memoria_escuchar_cpu, NULL);
    pthread_join (hilo_cpu,NULL);

    return 0;
}