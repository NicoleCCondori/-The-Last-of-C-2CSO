#include <kernel_escucha_memoria.h>
#include <k_conexiones.h>

void kernel_escucha_memoria(){
    //atender los msjs de memoria
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:
		
			break;
		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de MEMORIA");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}