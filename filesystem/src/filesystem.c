#include <filesystem.h>


int main(int argc, char* argv[]) {

    inicializar_FS();

    conectar_memoria();

    //finalizar_conexiones(1, cliente_memoria);
    //finalizar_modulo(logger_FS, valores_config_FS->config);
    log_destroy(FS_logger);
    free(valores_config_FS);
    
    return EXIT_SUCCESS;
}

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

void conectar_memoria() {
    //Esperar conexion memoria

    fd_FS = iniciar_servidor (valores_config_FS->puerto_escucha, FS_logger, "Servidor FS iniciado");
    
    log_info(FS_logger, "Esperando memoria...");

    fd_memoria = esperar_cliente(fd_FS, FS_logger, "MEMORIA");
    handshakeServer(fd_memoria);

	pthread_create(&hilo_memoria, NULL, (void *) escuchar_memoria, NULL);
	pthread_join(hilo_memoria, NULL);
}

void escuchar_memoria(){
	while(1){

		int op_code_memoria = recibir_operacion(fd_memoria);

		switch(op_code_memoria){
            case MENSAJE:
                //recibir_mensaje(cliente_memoria, logger_FS);
                break;
            case PAQUETE:
                //t_list* lista = recibir_paquete(fd_memoria);
                //log_info(logger_FS, "Me llegaron los siguientes valores:\n");
                //list_iterate(lista, (void*) iterator);
                break;
            case -1:
                log_error(FS_logger, "Desconexion de FS");
                exit(EXIT_FAILURE);
            default:
                log_warning(FS_logger,"Operacion desconocida de FS");
                break;
		}
	}
}
/*
void iterator(char* value) {
	log_info(logger_FS,"%s", value);
}*/