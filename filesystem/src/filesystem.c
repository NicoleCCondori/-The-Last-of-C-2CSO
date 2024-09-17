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

