#include <escuchar_kernel_dispatch.h>
#include <c_conexiones.h>

void escuchar_kernel_dispatch(){
    //atender los msjs de kernel-dispatch
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case RECIBIR_TID:
		t_paquete* paquete_aux=recibir_paquete(fd_kernel_dispatch);
		tidHilo=leer_buffer_uint32(paquete_aux->buffer);

		case PAQUETE:

			break;
		case -1:
			log_error(cpu_logger, "Desconexion de KERNEL - Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL -Interrupt");
			break;
		}
	}    
}