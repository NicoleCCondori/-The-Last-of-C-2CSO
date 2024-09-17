#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    memoria_logger=iniciar_logger(".//tp.log","log_cliente");
    
    memoria_log_obligatorios=iniciar_logger(".//tp.logs_olbigatorios.log","logs");

    t_config* memoria_config=iniciar_config("src/memoria.config");
    
    PUERTO_ESCUCHA=config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    
    fd_memoria=iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria iniciada ");
    
    //Esperar conexion CPU

    conectar_con_FS();
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_cpu();

    conectar_kernel();

    //falta finalizar las conexiones

    free(valores_config_memoria);

    return 0;
}

