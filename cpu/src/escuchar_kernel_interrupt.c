#include <escuchar_kernel_interrupt.h>



void escuchar_kernel_interrupt(){
    //atender los msjs de kernel-interrupt 
    
    while (1)
	{
		op_code cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op)
		{
		case INTERRUPCION:
			log_info(cpu_logger,"SE CONFIRMO LA INTERRUPCION");
			control_key_interrupt=1;
		case -1:
			log_error(cpu_logger, "Desconexion de KERNEL-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL-Interrupt");
			break;
		}
	}
}

