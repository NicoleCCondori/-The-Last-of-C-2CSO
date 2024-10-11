#include <kernel_escucha_cpu_dispatch.h>
#include <k_conexiones.h>

void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto kernel_escucha_cpu_dispatch.c \n");
	
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_cpu_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU-Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de CPU-Dispatch");
			break;
		}
	}
	//close(fd_cpu_dispatch); //liberar_conexion(fd_cpu_dispatch);
}