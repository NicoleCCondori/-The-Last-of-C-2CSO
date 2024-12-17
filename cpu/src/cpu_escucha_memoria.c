#include <cpu_escucha_memoria.h>
#include <c_conexiones.h>

void cpu_escucha_memoria(){
    bool control_key = 1;
    while (control_key)
	{
		
		t_paquete* contextoEje = recibir_paquete(fd_memoria);
		op_code cod_op = contextoEje->codigo_operacion;
    	
		switch (cod_op)
		{
		case MENSAJE:
			break;
		case PAQUETE:
			break;
		case ENVIAR_CONTEXTO:
			reciboContexto(contextoEje->buffer);
			break;
		case ENVIAR_INSTRUCCION:
			recibir_instruccion_de_memoria(fd_memoria);
			break;
		case -1:
			log_error(cpu_logger, "Desconexion de MEMORIA");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}

void reciboContexto(t_buffer* buffer)
{
	t_contextoEjecucion* contexto = leer_contexto_de_memoria(buffer);
	inicializar_particion_de_memoria(contexto-> base, contexto->limite);
    ciclo_de_instruccion(contexto);
}
