#include <kernel_escucha_cpu_interrupt.h>


void kernel_escucha_cpu_interrupt(){
    //atender los msjs de cpu-interrupt , otra funcion?
    while (1)
	{
		int cod_op = recibir_operacion(fd_cpu_interrupt);
		switch (cod_op)
		{
		case MENSAJE:
			break;
		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de CPU-Interrupt");
			break;
		}
	}
	//close(fd_cpu_interrupt); //liberar_conexion(fd_cpu_interrupt);
}