#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>

t_log* memoria_logger;
t_log* memoria_log_debug;
t_log* memoria_log_obligatorios;
t_log* memoria_config;

int fd_memoria;
int fd_cpu;

char* PUERTO_ESCUCHA;


void memoria_escuchar_cpu(){
    bool control_key=1;
   while (control_key)
	{
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op)
		{
		case MENSAJE:
			//
			break;
		case PAQUETE:
		//
			break;
		case -1:
			log_error(logger, "Desconexion con CPU");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida con CPU");
			break;
		}
	}
	return EXIT_SUCCESS;
} 

}


#endif