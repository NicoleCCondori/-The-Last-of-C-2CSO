#include <escuchar_cpu.h>
#include <m_conexiones.h>

void escuchar_cpu(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto memoria_escucha_cpu.c \n");
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(memoria_logger, "Desconexion de CPU");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida de CPU");
			break;
		}
	}

}
