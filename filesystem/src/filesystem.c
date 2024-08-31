#include <utils/utils.h>
#include <filesystem.h>
#include <commons/log.h>
#include <commons/config.h>

int main(int argc, char* argv[]) {

    logger_FS = iniciar_logger("fileSystem");
    levantar_config_FS("filesystem");
    inicializar_file_system();


    return EXIT_SUCCESS;

}

void inicializar_file_system () {
    server_fileSystem = iniciar_servidor(config_filesystem.puerto_escucha,logger_FS, "Servidor FS iniciado")
    crearHilos();
    }


void crearHilos()
{
    cliente_memoria = esperar_cliente(server_fileSystem, logger_FS, "Memoria");
    pthread_create(&hiloMemoria, NULL, recibirMemoria, NULL);
}



void levantar_config_FS (char* config_path){
    config = iniciar_config(config_path);

    if (config == NULL){
        log_info(logger_FS, "Error al levantar config");
        exit(1);
    }

    config_filesystem.puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
    config_filesystem.mount_dir = config_get_string_value(config,"MOUNT_DIR");
    config_filesystem.block_size= config_get_int_value(config,"BLOCK_SIZE");
    config_filesystem.block_count = config_get_int_value(config,"BLOCK_COUNT");
    config_filesystem.retardo_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
    config_filesystem.log_level= config_get_string_value(config,"LOG_LEVEL");
    }
