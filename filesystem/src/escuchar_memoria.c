#include <escuchar_memoria.h>
#include <f_conexiones.h>

void escuchar_memoria(){
	while(1){

		int op_code_memoria = recibir_operacion(fd_memoria);

		switch(op_code_memoria){
            case DUMP_MEMORY:
            //
                log_info(FS_logger,"OK");
                break;
            break;
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