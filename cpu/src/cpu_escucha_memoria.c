#include <cpu_escucha_memoria.h>


void cpu_escucha_memoria(){
    
    while (1)
	{
		t_paquete* paquete_cpu_memoria = recibir_paquete(fd_memoria);
		if (paquete_cpu_memoria == NULL) {
    		log_error(cpu_logger, "No se recibió un paquete desde Memoria o hubo un error.");
    		return;
		}
		log_info(cpu_logger, "op code paquete: %u",  paquete_cpu_memoria->codigo_operacion);
		op_code cod_op = paquete_cpu_memoria->codigo_operacion;
    	log_info(cpu_logger, "Recibo paquete memoria!!");
		
		switch (cod_op)
		{
		case ENVIAR_CONTEXTO:
			log_info(cpu_logger, "Recibo op code enviar contexto");
			reciboContexto(paquete_cpu_memoria);
			break;
	
		case ENVIAR_INSTRUCCION:
			log_info(cpu_logger, "Recibo op code enviar instruccion");
			recibir_instruccion_de_memoria(paquete_cpu_memoria);
			
			break;
		case CONFIRMAR_CONTEXTO_ACTUALIZADO:
			int bit_confirmacion = deserializar_int(paquete_cpu_memoria);
			log_info(cpu_logger, "Contenido del bit de confirmacion: %u", bit_confirmacion);
;			
			if(bit_confirmacion == 1){
				log_info(cpu_logger,"Recibo el OK de memoria(Se actualizo el contexto exitosamente)");
				log_info(cpu_logger,"AHORA CPU DEBE ENVIAR EL PAQUETE THREAD_CREATED A KERNEL\n");
				
				log_info(cpu_logger,"El valor del semaforo antes del post");
				sem_post(&sem_syscall);
				log_info(cpu_logger,"El valor del semaforo despues de hacer el post");

			}else{
				log_warning(cpu_logger,"No llego el OK (actualizar_contexto)\n");
			}
			break;	
		
		case CONFIRMAR_WRITE_MEM:
			log_info(cpu_logger,"Confirmacion write mem");
			break;
		
		case ENVIAR_READ_MEM:
			log_info(cpu_logger,"Me llega la lectura de memoria");
			break;
		
		case -1:
			log_error(cpu_logger, "Desconexion de MEMORIA");
			return;
		
		default:
			log_warning(cpu_logger, "Operacion desconocida de MEMORIA");
			break;
		}

		if(paquete_cpu_memoria) {eliminar_paquete(paquete_cpu_memoria);};
	}
}

void reciboContexto(t_paquete* contextoEje)
{
	t_contextoEjecucion* contexto = deserializar_enviar_contexto_cpu(contextoEje);
	log_info(cpu_logger, "Leo contexto de memoria");
	inicializar_particion_de_memoria(contexto-> base, contexto->limite);
	log_info(cpu_logger, "Se inicializo la particion de memoria");
	control_key = 1;
	empezar_ciclo_de_instruccion(contexto);
}

void recibir_instruccion_de_memoria(t_paquete* paquete) {    
    if (paquete == NULL) {
        log_error(cpu_logger, "Error al recibir paquete instruccion desde memoria");
        exit(EXIT_FAILURE);
    } else {
		log_info(cpu_logger, "Recibi paquete instruccion desde memoria");
	}

    // Leer la instruccion del buffer del paquete
    char* instruccion = deserializar_enviar_instruccion(paquete);
    
    if (instruccion == NULL) {
        log_error(cpu_logger, "Error al leer la instruccion del buffer");
        return;
    }

    // Loggear la instruccion recibida
    log_info(cpu_logger, "Instruccion recibida desde memoria: %s", instruccion);


    instruccionActual = instruccion;
	sem_post(&sem_instruccion);
}