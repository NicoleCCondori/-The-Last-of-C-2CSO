#include <utils/utils.h>
#include <filesystem.h>
#include <commons/log.h>
#include <commons/config.h>

t_log* logger_FS;
t_log* memoria_logs_obligatorios;
int cliente_memoria;
int servidor_FS;
t_config_filesystem* valores_config_FS;

int main(int argc, char* argv[]) {

    //loggers
    logger_FS = iniciar_logger("fileSystem.log", "FILESYSTEM.log");
    memoria_logs_obligatorios = iniciar_logger("FS_logs_obligatorios.log", "logs OBLIGATORIOS FILESYSTEM");
    
    //config
    valores_config_FS = configurar_FS();
    
    iniciar_conexiones_FS();
    //finalizar_conexiones(1, cliente_memoria);
    //finalizar_modulo(logger_FS, valores_config_FS->config);
    log_destroy(logger_FS);
   // free(valores_config_FS);
    return EXIT_SUCCESS;
}

t_config_filesystem* configurar_FS() {
    t_config_filesystem* config = malloc(sizeof(t_config_filesystem));
    config->config = iniciar_configs("src/filesystem.config");

    config->puerto_escucha = config_get_string_value(config->config, "PUERTO_ESCUCHA");
    config->mount_dir = config_get_string_value(config->config, "MOUNT_DIR");
    config->block_size = config_get_int_value(config->config, "BLOCK_SIZE");
    config->block_count = config_get_int_value(config->config, "BLOCK_COUNT");
    config->retardo_acceso_bloque = config_get_int_value(config->config, "RETARDO_ACCESO_BLOQUE");
    config->log_level = config_get_string_value(config->config, "LOG_LEVEL");

    return config;
    }

void iniciar_conexiones_FS() {
    
    servidor_FS = iniciar_servidor (valores_config_FS->puerto_escucha,logger_FS, "Servidor FS iniciado");
    conectarMemoria(servidor_FS, logger_FS, "MEMORIA");
}


void conectarMemoria(int servidor_FS, t_log* logger_FS, char* moduloCliente){

    log_info(logger_FS, "Espera conexion de memoria");

    cliente_memoria = esperar_cliente(servidor_FS, logger_FS, "MEMORIA");

	pthread_t hilo_memoria;
	pthread_create(&hilo_memoria, NULL, (void *) atender_memoria, NULL);
	pthread_detach(hilo_memoria);
}

void iterator(char* value) {
	log_info(logger_FS,"%s", value);
}

void atender_memoria(){
	while(1){

		int op_code_memoria = recibir_operacion(cliente_memoria);

		switch(op_code_memoria){
            case MENSAJE:
                recibir_mensaje(cliente_memoria, logger_FS);
                break;
            case PAQUETE:
                t_list* lista = recibir_paquete(cliente_memoria);
                log_info(logger_FS, "Me llegaron los siguientes valores:\n");
                list_iterate(lista, (void*) iterator);
                break;
            case -1:
                log_error(logger_FS, "el cliente se desconecto. Terminando servidor");
                //return EXIT_FAILURE;
            default:
                log_warning(logger_FS,"Operacion desconocida. No quieras meter la pata");
                break;
		}
	}
}