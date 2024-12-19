#include <cpu_escucha_memoria.h>
#include <c_conexiones.h>

sem_t sem_instruccion;

void cpu_escucha_memoria(){
    bool control_key = 1;
    while (control_key)
	{
		
		t_paquete* paquete = recibir_paquete(fd_memoria);
		if (paquete == NULL) {
    log_error(cpu_logger, "No se recibió un paquete desde Memoria o hubo un error.");
    continue;
	}
	log_info(cpu_logger, "Se recibió un paquete desde Memoria, op_code: %d", paquete->codigo_operacion);
		op_code cod_op = paquete->codigo_operacion;
    	log_info(cpu_logger, "recibo paquete memoria!!");
		switch (cod_op)
		{
		case MENSAJE:
			break;
		case PAQUETE:
			break;

		case ENVIAR_CONTEXTO:
			log_info(cpu_logger, "Recibo op code enviar contexto");
			reciboContexto(paquete);
			break;

		case ENVIAR_INSTRUCCION:
			recibir_instruccion_de_memoria(paquete);
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

void reciboContexto(t_paquete* contextoEje)
{
	t_contextoEjecucion* contexto = deserializar_enviar_contexto_cpu(contextoEje);
	log_info(cpu_logger, "Leo contexto de memoria");
	inicializar_particion_de_memoria(contexto-> base, contexto->limite);
	log_info(cpu_logger, "Se inicializo la particion de memoria");
    ciclo_de_instruccion(contexto);
}

void recibir_instruccion_de_memoria(t_paquete* paquete) {    
    if (paquete == NULL) {
        log_error(cpu_logger, "Error al recibir paquete instruccion desde memoria");
        return NULL;
    } else {log_error(cpu_logger, "Recibi paquete instruccion desde memoria");}

    // Leer la instruccion del buffer del paquete
    char* instruccion = deserializar_enviar_instruccion(paquete);
    
    if (instruccion == NULL) {
        log_error(cpu_logger, "Error al leer la instruccion del buffer");
        eliminar_paquete(paquete);
        return;
    }

    // Loggear la instruccion recibida
    log_info(cpu_logger, "Instruccion recibida desde memoria: %s", instruccion);

    eliminar_paquete(paquete);
    
    instruccionActual = instruccion;
	sem_post(&sem_instruccion);
}