#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    memoria_logger=log_create(".//tp.log","log_cliente",true,LOG_LEVEL_INFO);
    if (memoria_logger==NULL)
    {
        perror("Algo paso con el log. No se pudo crear");
        exit(EXIT_FAILURE);
    }
    memoria_log_obligatorios=log_create(".//tp.logs_olbigatorios.log","logs",true,LOG_LEVEL_INFO);
    if (memoria_log_obligatorios==NULL)
    {
        perror("Algo paso con el log. No se pudo crear");
        exit(EXIT_FAILURE);
    }
    t_config *memoria_config=config_create("src/memoria.config");
    if(memoria_config==NULL)
    {
        perror("Error al crear config");
        exit(EXIT_FAILURE);
    }
    PUERTO_ESCUCHA=config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    
    fd_memoria=iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria iniciada ");
    
    //Esperar conexion CPU

    log_info(memoria_logger,"Esperando CPU...");
    fd_cpu=esperar_cliente(fd_memoria,memoria_logger,"CPU");

    return 0;
}
