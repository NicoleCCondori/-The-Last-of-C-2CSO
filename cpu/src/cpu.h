#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
t_log* cpu_logger;
t_log* cpu_log_debug;
t_log* cpu_config;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

void cpu_escuchar_memoria(){
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:
		
		case PAQUETE:
		
			break;
		case -1:
			log_error(logger, "Desconexion de MEMORIA");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
	return EXIT_SUCCESS;
}




#endif