#include <utils/utils.h>
#include <memoria.h>

t_config_memoria* valores_config_memoria;
t_log* logger_memoria;

int main(int argc, char* argv[]) {
    //loggers
    logger_memoria = iniciar_logger("memoria.log", "MEMORIA.LOG");
    memoria_logs_obligatorios = iniciar_logger("memoria_logs_obligatorios.log", "logs OBLIGATORIOS MEMORIA");

    //config
    valores_config_memoria = configurar_memoria();

    //Conexiones
    conectar_con_FS();

    //finalizar_conexiones(1, cliente_memoria);
    finalizar_modulo(logger_memoria, config);

    return EXIT_SUCCESS;
}


t_config_memoria* configurar_memoria(){

    t_config_memoria* config = malloc(sizeof(t_config_memoria));
    config->config = iniciar_configs("src/memoria.config");
    
    if (config->config == NULL){
        log_info(logger_memoria, "Error al levantar el Config");
        exit(1);
    }
    
    config->puerto_escucha = config_get_string_value(config->config,"PUERTO_ESCUCHA");
    config->ip_filesystem = config_get_string_value(config->config,"IP_FILESYSTEM");
    config->puerto_filesystem = config_get_string_value(config->config,"PUERTO_FILESYSTEM");
    config->tam_memoria = config_get_string_value(config->config,"TAM_MEMORIA");
    config->path_instrucciones = config_get_string_value(config->config,"PATH_INSTRUCCIONES");
    config->retardo_respuesta = config_get_string_value(config->config,"RETARDO_RESPUESTA");
    config->esquema = config_get_string_value(config->config,"ESQUEMA");
    config->algoritmo_busqueda = config_get_string_value(config->config,"ALGORITMO_BUSQUEDA");
    /**config->particiones = config_get_array_value(config->config,"PARTICIONES");*/
    config->log_level = config_get_string_value(config->config,"LOG_LEVEL");

    return config;
}


int conectar_con_FS(){

    //int cliente_memoria = crear_conexion("127.0.0.1", "45030", "FILESYSTEM",memoria_logger);
    if (logger_memoria == NULL) {
    fprintf(stderr, "Logger no inicializado.\n");
    exit(EXIT_FAILURE);
}
    int cliente_memoria = crear_conexion(valores_config_memoria->ip_filesystem, valores_config_memoria->puerto_filesystem, "FILESYSTEM",logger_memoria);
    
    //Servidor MEMORIA
    //fd_memoria = iniciar_servidor(config_memoria->puerto_escucha, memoria_logger,"Memoria iniciada ");
    //printf("fd_memoria: %d\n", fd_memoria);
    
    //Esperar conexion CPU
    log_info(logger_memoria,"Esperando CPU...");
    fd_cpu = esperar_cliente(fd_memoria,logger_memoria,"CPU");
    handshakeServer(fd_cpu);
    printf("fd_cpu: %d\n", fd_cpu);

    //se crea un hilo para escuchar mensajes de CPU
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, (void*)memoria_escuchar_cpu, NULL);
    pthread_join (hilo_cpu,NULL);

    return 0;
}