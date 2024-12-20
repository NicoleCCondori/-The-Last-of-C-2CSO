#include <memoria_escucha_FS.h>

// no era cliente?
void memoria_escucha_FS(){
    while (1){
		int cod_op = recibir_operacion(fd_FS);
		switch (cod_op)
		{
		case MENSAJE:
		break;

		case PAQUETE:
			break;
			
		case -1:
			log_error(memoria_logger, "Desconexion de FILESYSTEM");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida de FILESYSTEM");
			break;
		}
	}
	//close(fd_cpu_dispatch); //liberar_conexion(fd_cpu_dispatch);
}