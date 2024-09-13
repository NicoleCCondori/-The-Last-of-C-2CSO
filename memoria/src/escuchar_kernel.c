#include <escuchar_kernel.h>
#include <m_conexiones.h>

void escuchar_kernel(){
    //atender los msjs de KERNEL , otra funcion?
    printf("Ejecuto escuchar_kernel.c \n");
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(memoria_logger, "Desconexion de KERNEL");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida de KERNEL");
			break;
		}
	}

}