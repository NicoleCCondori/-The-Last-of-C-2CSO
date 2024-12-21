#include <kernel_escucha_memoria.h>

void kernel_escucha_memoria(){
    //atender los msjs de memoria
    while (control_key_kernel_memoria == 1){
		log_info(kernel_logger,"Entre en el bucle de kernel escucha memoria");
		t_paquete* paquete_kernel = recibir_paquete(fd_memoria);//es bloqueante?

		if (paquete_kernel == NULL) {
            log_error(kernel_logger, "Error al recibir paquete de memoria");
            sleep(1); // Esperar un poco antes de intentar de nuevo
            //continue;
        } else {log_info(kernel_logger, "Me llego un paquete de memoria, con el siguiente op code: %u", paquete_kernel->codigo_operacion);}

		switch (paquete_kernel->codigo_operacion)
		{
			case CONFIRMAR_ESPACIO_PROCESO:
				pthread_t thread_confirmar_espacio_proceso;
				pthread_create(&thread_confirmar_espacio_proceso, NULL, (void*)confirmar_espacio_proceso, (void*)paquete_kernel);
				pthread_join(thread_confirmar_espacio_proceso, NULL);
				
				break;
			
			case CONFIRMAR_CREACION_HILO:
				pthread_t thread_confirmar_creacion_hilo;
				pthread_create(&thread_confirmar_creacion_hilo, NULL, (void*)confirmar_creacion_hilo, (void*)paquete_kernel);
				pthread_join(thread_confirmar_creacion_hilo, NULL);
				
				break;
				
			case CONFIRMAR_DUMP_MEMORY:
				pthread_t thread_confirmar_dump_memory;
				pthread_create(&thread_confirmar_dump_memory, NULL, (void*)confirmar_dump_memory, (void*)paquete_kernel);
				pthread_join(thread_confirmar_dump_memory, NULL);

				break;
			
			case CONFIRMAR_FINALIZAR_PROCESO:
				pthread_t thread_confirmar_finalizar_proceso;
				pthread_create(&thread_confirmar_finalizar_proceso, NULL, (void*)confirmar_finalizar_proceso, (void*)paquete_kernel);
				pthread_join(thread_confirmar_finalizar_proceso, NULL);
				break;

			case CONFIRMAR_FINALIZAR_HILO:
				pthread_t thread_confirmar_finalizar_hilo;
				pthread_create(&thread_confirmar_finalizar_hilo, NULL, (void*)confirmar_finalizar_hilo, (void*)paquete_kernel);
				pthread_join(thread_confirmar_finalizar_hilo, NULL);
				
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
	//	control_key_kernel_memoria = 0;
	}
}
void confirmar_espacio_proceso(void* arg)
{
	t_paquete* paquete_kernel = (t_paquete*)arg;
	t_asigno_memoria* datos_proceso_memoria = deserializar_proceso_memoria(paquete_kernel);
	log_info(kernel_logger, "Contenido del bit de confirmacion: %u para confirmar_espacio_proceso",  datos_proceso_memoria->bit_confirmacion);

	if(datos_proceso_memoria==NULL){
		log_error(kernel_logger,"ERROR al deserializar el paquete");
		//break;
	}

	if(datos_proceso_memoria->bit_confirmacion == 1){
		log_info(kernel_logger,"El pid es -> %u", datos_proceso_memoria->pid);
		crear_hilo(datos_proceso_memoria->pid);//crear el hilo main cuando se crea un nuevo proceso
		//sem_post(&sem_plani_largo_plazo);
		//log_info(kernel_logger,"Despues del post plani");

	} else {
		log_warning(kernel_logger,"No hay espacio en memoria\n");
		//sem_post(&sem_plani_largo_plazo);
		//Se mantiene en el estado NEW
	}
}

void confirmar_creacion_hilo(void* arg){
	t_paquete* paquete_kernel = (t_paquete*)arg;
	log_info(kernel_logger, "OK LLEGO LA CONFIRMACION A CREAR el hilo");

	t_creacion_hilo* datos_hilo_memoria = deserializar_creacion_hilo_memoria(paquete_kernel);
	log_info(kernel_logger, "Contenido del bit de confirmacion(confirmar_creacion_hilo): %u", datos_hilo_memoria->bit_confirmacion);

	if(datos_hilo_memoria==NULL){
		log_error(kernel_logger,"ERROR al deserializar el paquete creacion del hilo");
		//break;
	}

	if(datos_hilo_memoria->bit_confirmacion == 1){
		log_info(kernel_logger,"El pid es: %u y el tid es: %u", datos_hilo_memoria->pid, datos_hilo_memoria->tid);
		confirmacion_crear_hilo(datos_hilo_memoria->pid, datos_hilo_memoria->tid);
		log_info(kernel_logs_obligatorios, "Creacion de Hilo: “## (<PID>: %u <TID>: %u) Se crea el Hilo - Estado: READY”", datos_hilo_memoria->pid, datos_hilo_memoria->tid);
    	//planificador_corto_plazo(hilo_main);
		sem_post(&TCBaPlanificar);
		planificador_corto_plazo();
						
	}else{
		log_warning(kernel_logger,"No se pudo crear el hilo\n");
	}
}

void confirmar_finalizar_proceso(void* arg)
{
	/*t_paquete* paquete_kernel = (t_paquete*)arg;
	t_asigno_memoria* datos_finalizar_proceso = deserializar_proceso_memoria(paquete_kernel);
	log_info(kernel_logger,"Estoy en confirmar finalizar_proceso pid:%u",datos_finalizar_proceso->pid);
	log_info(kernel_logger, "Contenido del bit de confirmacion: %u para confirmar_espacio_proceso",  datos_finalizar_proceso->bit_confirmacion);

	if(datos_finalizar_proceso==NULL){
		log_error(kernel_logger,"ERROR al deserializar el paquete");
		//break;
	}

	if(datos_finalizar_proceso->bit_confirmacion == 1){
		log_info(kernel_logger,"El pid es -> %u", datos_finalizar_proceso->pid);
		//1ro) Veo en la lista de tcbs -->los que tengan el mismo pid los paso al estado EXIT
		////sem_wait(&mutex);
		PCB* pcb_afuera = buscar_proceso(lista_procesos, datos_finalizar_proceso->pid);
		
		for(int i=0; i<list_size(lista_tcbs);i++){
			TCB* tcb_en_lista = (TCB*)list_get(lista_tcbs,i);
			if(tcb_en_lista->pid == pcb_afuera->pid){
				finalizar_hilo(tcb_en_lista); 
				//en esta misma funcion ya destruye el tid que esta en la lista de tids del pcb
			}
		}
		////sem_post(&mutex);
		//2do) destruyo la lista de tids(enteros) que tiene el pcb -> por las dudas jsjs
		list_destroy(pcb_afuera->lista_tid);					
		//consultar si dejarlo en la lista de procesos general
		//en mi opinion dejarlo para saber el estado del proceso que seria en este caso EXIT
		//sacarlo de la lista_procesos:
		/*for (int i = 0; i < list_size(lista_procesos); i++) {
			PCB* pcb_en_lista = (PCB*)list_get(lista_procesos, i);
			if (pcb_en_lista->pid == pcb_afuera->pid) {
				// Encontramos el PCB a eliminar
				list_remove_and_destroy_element(lista_procesos, i, destruir_pcb);
				break; // Salir del bucle después de eliminar
			}
		}
		//debo actualizar el estado del proceso (pcb)
		pcb_afuera->estado = EXIT;
		pcb_afuera->tid_contador = -1;
		pcb_afuera->prioridad_main = -1;

		log_info(kernel_logs_obligatorios,"Fin de Proceso: ## Finaliza el proceso <PID> %u", pcb_afuera->pid);
		//sem_post(&sem_plani_largo_plazo);
		//planificador_de_largo_plazo();
		
	} else {
		log_warning(kernel_logger,"No Existe el proceso para que memoria lo finalice\n");
	}	*/
}

void confirmar_finalizar_hilo(void* arg)
{
	t_paquete* paquete_kernel = (t_paquete*)arg;
	t_creacion_hilo* datos_finalizar_hilo = deserializar_creacion_hilo_memoria(paquete_kernel);
	log_info(kernel_logger,"Estoy en confirmar finalizar_hilo pid:%u tid:%u",datos_finalizar_hilo->pid,datos_finalizar_hilo->tid);
	log_info(kernel_logger, "Contenido del bit de confirmacion(confirmar_creacion_hilo): %u", datos_finalizar_hilo->bit_confirmacion);

	if(datos_finalizar_hilo==NULL){
		log_error(kernel_logger,"ERROR al deserializar el paquete creacion del hilo");
		//break;
	}

	if(datos_finalizar_hilo->bit_confirmacion == 1){
		log_info(kernel_logger,"El pid es: %u y el tid es: %u", datos_finalizar_hilo->pid, datos_finalizar_hilo->tid);

		TCB* hilo = buscar_tcbs(datos_finalizar_hilo->tid,datos_finalizar_hilo->pid);
		log_info(kernel_logs_obligatorios, "Fin de Hilo: ## (<PID>:%u <TID>:%u ) Finaliza el hilo", hilo->pid, hilo->tid);
					
		//1ro) Ingreso el hilo a la lista de exit
		hilo->estadoHilo = EXIT;
		////sem_wait(&mutex);
		list_add(lista_exit, hilo);
		////sem_post(&mutex);
		//EL PASO 2 NO SE CUMPLE PORQUE ME GUSTARIA CONSERVAR EL ESTADO DEL TCB EN LA LISTA_TCBS
		//2do) lo saco de la lista de tcbs
		/*for(int i =0; i<list_size(lista_tcbs);i++){
			TCB* tcb_en_lista = (TCB*)list_get(lista_tcbs, i);
			if(tcb_en_lista->pid == hilo->pid && tcb_en_lista->tid == )
			list_remove_element (lista_tcbs, (void *) tcb_en_lista);
		}*/
		//list_remove_element(lista_tcbs,(void*)hilo);  CREO QUE ES MEJOR CONSERVARLO PARA SABER EL ESTADO

		//3ro a) lo saco de la lista tid de su pcb -> debo buscar en la lista de procesos con el pid
		//sem_wait(&mutex);
		PCB* tid_a_retirar = buscar_proceso(lista_procesos, hilo->pid);
		////sem_post(&mutex);
		if(tid_a_retirar == NULL){
			printf("No se encontro el PID: %d", hilo->pid);
			return;
		}
		//list_remove_element(tid_a_retirar->tid,(void*) hilo->tid);
		list_remove_element(tid_a_retirar->lista_tid, (void *)(uintptr_t)hilo->tid);

		//3ro b) ver que onda con los recursos de mutex-> deberia liberarlo tmb
		for(int i=0; i<list_size(tid_a_retirar->mutex); i++){
			t_mutex* mutex_pos = list_get(tid_a_retirar->mutex, i);

			if(mutex_pos->tid == hilo->tid){
				if (!queue_is_empty(mutex_pos->bloqueados_mutex)) {
					uint32_t nro_hilo = (uint32_t)(uintptr_t)queue_pop(mutex_pos->bloqueados_mutex);
					TCB* sgte_hilo = buscar_tcbs(nro_hilo,tid_a_retirar->pid);
					sgte_hilo->estadoHilo = READY;
					mutex_pos->tid = sgte_hilo->tid;
					//sem_wait(&mutex);
					list_add(lista_ready, sgte_hilo);
					////sem_post(&mutex);
				} else {
					//Si no hay hilos bloqueados, asignar el tid como vacio
					mutex_pos->tid = -1;
				}
			}
		}
		//4to) Pasar los hilos que estaban bloqueados a ready
		//mover al estado ready a todos los hilos bloqueados por este hilo
		desbloquear_hilos_por_tid(hilo);
						
	}else{
		log_warning(kernel_logger,"No Existe el hilo para que memoria lo finalicen\n");
	}
}

void confirmar_dump_memory(void* arg){
	log_info(kernel_logger,"Llego el OK de confirmar dump memory");
}