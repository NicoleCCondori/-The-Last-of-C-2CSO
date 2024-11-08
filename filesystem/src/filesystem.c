#include <filesystem.h>

int main(int argc, char* argv[]) {

    inicializar_FS();
    fd_FS = iniciar_servidor (valores_config_FS->puerto_escucha, FS_logger, "Servidor FS iniciado");
    conectar_memoria();

    //liberar los logs y config
    finalizar_modulo(FS_logger, FS_logs_obligatorios, valores_config_FS->config);
    free(valores_config_FS);

    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);
    return EXIT_SUCCESS;
}

