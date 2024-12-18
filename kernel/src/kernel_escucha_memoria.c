#include <kernel_escucha_memoria.h>


void kernel_escucha_memoria(){
    //atender los msjs de memoria
    bool control_key = 1;
    while (control_key){
		/*op_code cod_op = recibir_operacion(fd_memoria);
		log_info(kernel_logger, "primer op code: %u",  cod_op);*/

		t_paquete* paquete_kernel = recibir_paquete(fd_memoria);
		log_info(kernel_logger, "op code paquete: %u",  paquete_kernel->codigo_operacion);

		switch (paquete_kernel->codigo_operacion)
		{
			case MENSAJE:
				break;
			case PAQUETE:
				log_info(kernel_logger,"enum paquete");
				break;
			case CONFIRMAR_ESPACIO_PROCESO:
				//t_paquete* paquete_kernel = recibir_paquete(fd_memoria);

				if (paquete_kernel == NULL || paquete_kernel->buffer->size < sizeof(t_asigno_memoria)) {
				log_error(kernel_logger, "ERROR: El tamaño del paquete recibido es insuficiente o el paquete es nulo");
				break;
				}

				printf("Tamaño del paquete recibido: %d\n", paquete_kernel->buffer->size);
				printf("Contenido del buffer recibido: ");
				for (int i = 0; i < paquete_kernel->buffer->size; i++) {
					printf("%02x ", paquete_kernel->buffer->stream);
				}
				printf("\n");

				t_asigno_memoria* datos_proceso_memoria = deserializar_proceso_memoria(paquete_kernel);
			
				log_info(kernel_logger, "Contenido del bit de confirmacion: %u",  datos_proceso_memoria->bit_confirmacion);


				if(datos_proceso_memoria==NULL){
					log_error(kernel_logger,"ERROR al deserializar el paquete");
					break;
				}

				if(datos_proceso_memoria->bit_confirmacion == 1){
					
					log_info(kernel_logger,"El pid es -> %u", datos_proceso_memoria->pid);
					crear_hilo(datos_proceso_memoria->pid);
					sem_post(&sem_plani_largo_plazo);

					sem_post(&TCBaPlanificar);
        			//planificador_corto_plazo(hilo_main);
					planificador_corto_plazo();
					//mandar_hilo_a_cola_ready(hilo_main);
					//queue_push(cola_ready,hilo_main); //consulta ¿pasamos a ready el tcb o pcb?
				} else {
					printf("No hay espacio en memoria\n");
					sem_post(&sem_plani_largo_plazo);
					//sem_wait(&sem_binario_memoria);
					//Se mantiene en el estado NEW
				}
				log_info(kernel_logger,"estoy entrando aca depues de que mamoria me da un ok de contexto");
				eliminar_paquete(paquete_kernel);
				// Señalar al receptor que los datos están disponibles
				sem_post(&semaforo_binario);  // Libera el semáforo binario
				break;
			
			case CONFIRMAR_CREACION_HILO:
					log_info(kernel_logger, "ok de creo el hilo");
					sem_post(&sem_binario_memoria);
					printf("No se pudo crear el hilo\n");
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de MEMORIA");
				exit(EXIT_FAILURE);
			default:
				log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
				break;
		}
	}
}