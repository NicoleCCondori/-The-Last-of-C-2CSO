#include <f_conexiones.h>

t_log* FS_logger;
t_log* FS_logs_obligatorios;

t_config_filesystem* valores_config_FS;

int fd_memoria;
int fd_FS;

//pthread_t hilo_memoria;

void inicializar_FS(){
    FS_logger = iniciar_logger("fileSystem.log", "FILESYSTEM.log");
    FS_logs_obligatorios = iniciar_logger("FS_logs_obligatorios.log", "logs OBLIGATORIOS FILESYSTEM");
    configurar_FS();
}

void configurar_FS() {
    //t_config_filesystem* config = malloc(sizeof(t_config_filesystem));
    valores_config_FS = malloc(sizeof(t_config_filesystem));
    
    valores_config_FS->config = iniciar_configs("src/filesystem.config");

    valores_config_FS->puerto_escucha = config_get_string_value(valores_config_FS->config, "PUERTO_ESCUCHA");
    valores_config_FS->mount_dir = config_get_string_value(valores_config_FS->config, "MOUNT_DIR");
    valores_config_FS->block_size = config_get_int_value(valores_config_FS->config, "BLOCK_SIZE");
    valores_config_FS->block_count = config_get_int_value(valores_config_FS->config, "BLOCK_COUNT");
    valores_config_FS->retardo_acceso_bloque = config_get_int_value(valores_config_FS->config, "RETARDO_ACCESO_BLOQUE");
    valores_config_FS->log_level = config_get_string_value(valores_config_FS->config, "LOG_LEVEL");

}

/*void conectar_memoria() {
    //Esperar conexion memoria

    fd_FS = iniciar_servidor (valores_config_FS->puerto_escucha, FS_logger, "Servidor FS iniciado");
    
    log_info(FS_logger, "Esperando memoria...");

    fd_memoria = esperar_cliente(fd_FS, FS_logger, "MEMORIA");
    handshakeServer(fd_memoria);

	pthread_create(&hilo_memoria, NULL, (void *) escuchar_memoria, NULL);
	pthread_join(hilo_memoria, NULL);
}*/

//Conexión con multihilos
void conectar_memoria(){
    //Esperar conexion memoria

    log_info(FS_logger, "Esperando memoria...");
	while (1) //siempre está esperando
	{
		fd_memoria = esperar_cliente(fd_FS, FS_logger,"MEMORIA");
		if(fd_FS == -1){
			log_error(FS_logger, "Error creando conexión con memoria");
		}
		//log_info(FS_logger, "Conexión exitosa con MEMORIA");

		handshakeServer(fd_memoria);
        
        pthread_t hilo_memoria;
        int* fd_nueva_conexion_ptr = malloc(sizeof(int));
        *fd_nueva_conexion_ptr = fd_memoria;
		pthread_create(&hilo_memoria,NULL,(void*)escuchar_memoria,fd_nueva_conexion_ptr);
		pthread_detach(hilo_memoria);
	}
}


/*
void iterator(char* value) {
	log_info(logger_FS,"%s", value);
}*/