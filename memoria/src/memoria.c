#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    conectar_con_FS();
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_cpu();

    conectar_kernel();

    //falta finalizar las conexiones

    free(valores_config_memoria);

    return 0;
}

