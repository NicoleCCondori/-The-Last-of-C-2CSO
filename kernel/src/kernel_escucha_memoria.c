#include <kernel_escucha_memoria.h>


void kernel_escucha_memoria(){
    //atender los msjs de memoria
    while (1){

		log_info(kernel_logger,"entre en el bucle de kernel escucha memoria");
		usleep(1000);
		t_paquete* paquete_kernel = recibir_paquete(fd_memoria);//es bloqueante?

		if (paquete_kernel == NULL) {
            log_error(kernel_logger, "Error al recibir paquete de memoria");
            sleep(1); // Esperar un poco antes de intentar de nuevo
            continue;
        } else {(kernel_logger, "Me llego un paquete de memoria, con el siguiente op code: %u", paquete_kernel->codigo_operacion);}

		switch (paquete_kernel->codigo_operacion)
		{
			case CONFIRMAR_ESPACIO_PROCESO:
				t_asigno_memoria* datos_proceso_memoria = deserializar_proceso_memoria(paquete_kernel);
			
				log_info(kernel_logger, "Contenido del bit de confirmacion: %u para confirmar_espacio_proceso",  datos_proceso_memoria->bit_confirmacion);

				if(datos_proceso_memoria==NULL){
					log_error(kernel_logger,"ERROR al deserializar el paquete");
					break;
				}

				if(datos_proceso_memoria->bit_confirmacion == 1){
					
					log_info(kernel_logger,"El pid es -> %u", datos_proceso_memoria->pid);
					crear_hilo(datos_proceso_memoria->pid);//crear el hilo main cuando se crea un nuevo proceso
					sem_post(&sem_plani_largo_plazo);
					
					log_info(kernel_logger,"Despues del post plani");
				
				} else {
					log_warning(kernel_logger,"No hay espacio en memoria\n");
					sem_post(&sem_plani_largo_plazo);
					//Se mantiene en el estado NEW
				}
				break;
			
			case CONFIRMAR_CREACION_HILO:
					log_info(kernel_logger, "OK LLEGO LA CONFIRMACION A CREAR el hilo");

					t_creacion_hilo* datos_hilo_memoria = deserializar_creacion_hilo_memoria(paquete_kernel);
					log_info(kernel_logger, "Contenido del bit de confirmacion(confirmar_creacion_hilo): %u",  datos_proceso_memoria->bit_confirmacion);

					if(datos_hilo_memoria==NULL){
						log_error(kernel_logger,"ERROR al deserializar el paquete creacion del hilo");
						break;
					}

					if(datos_hilo_memoria->bit_confirmacion == 1){
						log_info(kernel_logger,"El pid es: %u y el tid es: %u", datos_hilo_memoria->pid, datos_hilo_memoria->tid);
						confirmacion_crear_hilo(datos_hilo_memoria->pid, datos_hilo_memoria->tid);
						
        				//planificador_corto_plazo(hilo_main);
						sem_post(&TCBaPlanificar);
						planificador_corto_plazo();
						
					}else{
						log_warning(kernel_logger,"No se pudo crear el hilo\n");
					}	
				break;
				
			case CONFIRMAR_DUMP_MEMORY:
				log_info(kernel_logger,"Llego el OK de confirmar dump memory");
				break;

			case -1:
				log_error(kernel_logger, "Desconexion de MEMORIA");
				exit(EXIT_FAILURE);
			default:
				log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
				break;
		}
		if(paquete_kernel){
			eliminar_paquete(paquete_kernel);
		}
		
	}
}