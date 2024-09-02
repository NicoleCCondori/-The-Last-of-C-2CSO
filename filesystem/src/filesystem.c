#include <utils/utils.h>
#include <filesystem.h>
#include <commons/log.h>
#include <commons/config.h>


int main(int argc, char* argv[]) {

    logger_FS = iniciar_logger("fileSystem.log", "FILESYSTEM");
    log_info(logger_FS, "Se creo exitosamente el logger de filesystem");

    levantar_config_FS("src/filesystem.config");
    
    iniciar_conexiones_FS();

    //finalizar_conexiones(1, cliente_memoria);

    finalizar_modulo(logger_FS, config);

    free(config_filesystem);

    return EXIT_SUCCESS;

}

void levantar_config_FS (char* config_path){
    t_config* config = iniciar_configs(config_path);
    config_filesystem = malloc(sizeof(t_config_filesystem));

    config_filesystem->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
    config_filesystem->mount_dir = config_get_string_value(config,"MOUNT_DIR");
    config_filesystem->block_size= config_get_int_value(config,"BLOCK_SIZE");
    config_filesystem->block_count = config_get_int_value(config,"BLOCK_COUNT");
    config_filesystem->retardo_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
    config_filesystem->log_level= config_get_string_value(config,"LOG_LEVEL");
    }

void iniciar_conexiones_FS() {

    char* puerto =  string_itoa(config_filesystem->puerto_escucha);
    
    int servidor_FS = iniciar_servidor (puerto,logger_FS, "Servidor FS iniciado");
    conectarMemoria(servidor_FS, logger_FS, "MEMORIA");
}


void conectarMemoria(int servidor_FS, t_log* logger_FS, char* moduloCliente){

    log_info(logger_FS, "Espera conexion de memoria");

    int cliente_memoria = esperar_cliente(servidor_FS, logger_FS, "MEMORIA");

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
                return EXIT_FAILURE;
            default:
                log_warning(logger_FS,"Operacion desconocida. No quieras meter la pata");
                break;
		}
	}
}