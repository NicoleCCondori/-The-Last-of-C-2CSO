#include <kernel_escucha_cpu_dispatch.h>


void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    
    //while (1){
		
		log_info(kernel_logger,"estoy en kernel escucha cpu dispatch");
		usleep(10000);
		t_paquete* datos_de_cpu = recibir_paquete(fd_cpu_dispatch);//es bloqueante
		log_info(kernel_logger,"Kernel escucha CPU dispatch LE LLEGO EL PAQUETE CON el Op code:%d!!!", datos_de_cpu->codigo_operacion);
		
		//log_info(kernel_logs_obligatorios, "Syscall recibida: ## (<PID>:%u : <TID>:%u) - Solicitó syscall: <%d>", invocadores->pid_inv,invocadores->tid_inv, datos_de_cpu->codigo_operacion);
		
		switch (datos_de_cpu->codigo_operacion) //siempre debo saber el pid y el tid de quien invocó
		{
			case PROCESS_CREATE:
				//Debo deserializar el buffer que contiene los parametros de nomArchP,tamProceso y prioridad del hilo main
				t_process_create* pc_hiloMain = deserializar_process_create(datos_de_cpu);
				//Crea el pcb. Tal vez crear funcion de crear pcb asi no se repite código
				crear_proceso(pc_hiloMain->tamProceso,pc_hiloMain->nomArchP,pc_hiloMain->prioridadHM);
				break;
			
			case PROCESS_EXIT: //Finalizo el pcb correspondiente al tcb, solo lo llama el tid 0
				// Envia todos los TCBs asociado al PCB y los manda a EXIT.
				// Avisar a memoria de la finalizacion
				//Deserializo el buffer y de ahi debo obtener el pid_afuera del proceso
				t_enviar_contexto* invocadores = deserializar_enviar_contexto(datos_de_cpu);
				log_info(kernel_logger, "Me llego PROCESS_EXIT con pid:%u tid:%u", invocadores->PID,invocadores->TID);
				if(invocadores->TID == 0){ //si el tid es 0, es decir que lo invoco el hilo main --> finaliza el proceso (PCB)

					//sem_wait(&mutex);
					PCB* pcb_afuera = buscar_proceso(lista_procesos, invocadores->PID);
					//sem_post(&mutex);
					if(pcb_afuera == NULL){
						printf("No se encontro el PID: %u \n", invocadores->PID);
					}

					finalizar_proceso(pcb_afuera, invocadores->TID);
				}
				
				break;

			case THREAD_CREATE://genera un nuevo TCB con un TID autoincremental - mandarlo a READY

				log_info(kernel_logger,"Ingresamos a la syscall crear hilo ");
				t_thread_create* tc_hilo = deserializar_thread_create(datos_de_cpu); //debemos ya tener el pid_inv y tid_inv de quien lo invoco
				log_info(kernel_logger,"hilo a crear PID:%u ,TID: %u , prioridad %u , PATH: %s\n",tc_hilo->PID, tc_hilo->TID,tc_hilo->prioridadH, tc_hilo->nombreArchT);
				
				//sem_wait(&mutex);
				PCB* pcb_buscado = buscar_proceso(lista_procesos, tc_hilo->PID);
				//sem_post(&mutex);
				
				if(pcb_buscado == NULL){
					log_error(kernel_logger,"No se encontro el PID: %u \n", tc_hilo->PID);
					break;
				}
				
				pcb_buscado->tid_contador++;

				uint32_t nuevo_tid = pcb_buscado->tid_contador++;
				log_info(kernel_logger,"El nuevo_tid es: %u", nuevo_tid);
				
				uint32_t* tid_copia = malloc(sizeof(uint32_t));
				if(tid_copia == NULL){
					log_info(kernel_logger,"Error al asignar memoria para tid_copia");					
				}
				*tid_copia = nuevo_tid;
				list_add(pcb_buscado->lista_tid, tid_copia);

				//creo el nuevo tcb con el mismo pid
				TCB* nuevo_hilo_iniciado = iniciar_hilo(nuevo_tid,tc_hilo->prioridadH, tc_hilo->PID, tc_hilo->nombreArchT);
				
				//agregar el tcb a la lista de tcbs
				//sem_post(&mutex);
				list_add(lista_tcbs,nuevo_hilo_iniciado);
				//sem_post(&mutex);

				log_info(kernel_logger,"ESTOY AGREGANDO ESTE HILO A LA LISTA TCBs con pid %u y tid %u", nuevo_hilo_iniciado->pid, nuevo_hilo_iniciado->tid);
				log_info(kernel_logger,"Se envia a memoria por medio de thread create");
				enviar_a_memoria(fd_memoria, nuevo_hilo_iniciado);
				log_info(kernel_logger,"Enviamos el hilo a memoria, esperar su confirmacion. Tendría que ir a kernel_escucha_memoria");
				//log_info(kernel_logs_obligatorios, "## Creación de Hilo: ## (<PID>:%u <TID>:%u) Se crea el Hilo - Estado: READY",invocadores->pid_inv, invocadores->tid_inv);
				break;
				
			case THREAD_JOIN:
				// Mover el hilo a estado BLOCK
				// Aclaración: el tid pasado por parámetro debe terminar sino el hilo que lo invocó va a seguir en BLOCK
				t_thread_join_y_cancel* tid_join = deserializar_thread_join_y_cancel(datos_de_cpu);

				//sem_wait(&mutex);
				TCB* tcb_a_bloquear = buscar_tcbs(tid_join->TID ,tid_join->PID);//BLOQUEADO, primero se busca el TID grande para saber quien invoco al thread_join
				//sem_post(&mutex);

				//sem_wait(&mutex);
				TCB* tcb_buscado = buscar_tcbs(tid_join->tid,tid_join->PID);//TID que invocó deepende del estado del tid chico
				//sem_post(&mutex);
				if(tcb_buscado == NULL || tcb_buscado->estadoHilo == EXIT){
					log_info(kernel_logger,"No se encontro o ya FINALIZO el TID: %u del PID: %u \n", tid_join->tid,tid_join->PID);
					tcb_a_bloquear->estadoHilo = READY;
					list_add(lista_ready,tcb_a_bloquear);
				}else{
					tcb_a_bloquear->estadoHilo = BLOCKED;
					tcb_a_bloquear->tid_que_lo_bloqueo = tid_join->TID;
					//sem_wait(&mutex);
					list_add(lista_blocked,tcb_a_bloquear);
					//sem_post(&mutex);
					log_info(kernel_logs_obligatorios, "Motivo de Bloqueo: ## (<PID>:%u <TID>:%u) - Bloqueado por: PTHREAD_JOIN", tcb_a_bloquear->pid,tcb_a_bloquear->tid);
				}

				break;
			case THREAD_CANCEL:
				// Finalizar el hilo pasandolo a EXIT
				// Avisar a memoria de la finalizacion.
				// Aclaración: el hilo que lo invocó sigue con su ejecución si es que el hilo pasado por parámetro no existe o finalizo hace rato
				t_thread_join_y_cancel* tid_cancel = deserializar_thread_join_y_cancel(datos_de_cpu);

				//sem_wait(&mutex);
				TCB* tcb_a_exit = buscar_tcbs(tid_cancel->TID,tid_cancel->PID);
				//sem_post(&mutex);
				if(tcb_a_exit == NULL || tcb_a_exit->estadoHilo == EXIT){
					log_info(kernel_logger,"No se encontro o ya FINALIZO el TID: %u del PID: %u (thread_cancel)\n", tid_cancel->TID, tid_cancel->PID);
					//sigue con su ejecución
				} else{
					finalizar_hilo(tcb_a_exit);
				}
				break;
				
			case THREAD_EXIT:
				t_datos_esenciales* invocadores2 = deserializar_datos_esenciales(datos_de_cpu);
				// Finaliza el hilo que lo invocó --> conectar con la función
				//sem_wait(&mutex);
				TCB* hilo_a_finalizar = buscar_tcbs(invocadores2->tid_inv, invocadores2->pid_inv);
				//sem_post(&mutex);
				if (hilo_a_finalizar == NULL) {
					printf("No se encontró el TID: %u del PID: %u para finalizar (thread_exit)\n", invocadores2->tid_inv, invocadores2->pid_inv);
				}else {
					finalizar_hilo(hilo_a_finalizar);
				}
				//Puedo poner que si finaliza el hilo que los demás hilos se desbloquean y pasan a ready
				break;
				
			case MUTEX_CREATE:
				t_mutex_todos* recurso_create = deserializar_mutex(datos_de_cpu);
				//Por el momento tengo el pid y tid que invoco junto con el recurso
				t_mutex* creado = malloc(sizeof(t_mutex));
				if(creado == NULL){
					log_error(kernel_logger,"Error al crear el mutex\n");
				}
				creado->recurso = recurso_create->recurso;
				creado->tid = -1;  //Acá todavia no se le asigna ningún hilo
				creado->bloqueados_mutex = queue_create(); // se inicializa una cola para ingresar 
				//CONSULTAR: El tp da una sugerencia de usar un semaforo contador 
					//Para mutex_lock necesito verificar si esta sin asignar el mut deberia usar sem_trywait??? rta: no es necesario
				//sem_init(&(creado->contador),0,1); //semáforo contador = 1, es decir, sin asignar
				//sem_wait(&mutex);
				PCB* pcb_buscadoRC = buscar_proceso(lista_procesos, recurso_create->PID);
				//sem_post(&mutex);
				if(pcb_buscadoRC == NULL){
					log_error(kernel_logger,"No se encontro el PID: %u", recurso_create->PID);
				}
				list_add(pcb_buscadoRC->mutex, creado); //las estructura t_mutex lo agregamos en la lista mutex de cada pcb
				break;
			
			case MUTEX_LOCK:	
				t_mutex_todos* recurso_lock = deserializar_mutex(datos_de_cpu); //Por el momento tengo el pid y tid que invoco junto con el recurso


				//sem_wait(&mutex);
				PCB* pcb_buscadoRL = buscar_proceso(lista_procesos, recurso_lock->PID);//Buscamos el pcb para poder ingresar a su lista mutex
				//sem_post(&mutex);
				if(pcb_buscadoRL == NULL){
					printf("No se encontro el PID: %u \n", recurso_lock->PID);
				}
				//sem_wait(&mutex);
				TCB* hilo_block_mutex = buscar_tcbs(recurso_lock->TID, recurso_lock->PID);//Buscamos tmb el tcb para despues ingresarlo a la lista de bloqueados
				//sem_post(&mutex);
				if(hilo_block_mutex == NULL){
					log_error(kernel_logger,"No se encontro el TID: %u del PID:%u\n", recurso_lock->TID,recurso_lock->PID);
				}

				//1RO debe existir el mutex solicitado
				t_mutex* lockeado = buscar_mutex(pcb_buscadoRL->mutex, recurso_lock->recurso);
				if(lockeado == NULL){
					log_error(kernel_logger,"NO EXISTE EL MUTEX SOLICITADO\n");
				}
				// está asignado?
				if(lockeado->tid == -1){ //2do) SI NO ESTA TOMADO -> se lo asigno a dicho hilo
					lockeado->tid = recurso_lock->TID;

				}else{ //3ro) SI ESTA TOMADO -> Ingresa a la cola de bloqueados de la lista mutex de la lista de procesos. Seria PCB->mutex->t_mutex->bloqueados_mutex
					//queue_push(lockeado->bloqueados_mutex,invocadores->tid_inv);
					queue_push(lockeado->bloqueados_mutex, (void *)(uintptr_t)recurso_lock->TID);
					//Tmb ingresa a la lista de bloqueados general de hilos
					hilo_block_mutex->estadoHilo = BLOCKED;
					hilo_block_mutex->tid_que_lo_bloqueo = recurso_lock->TID;
					//sem_wait(&mutex);
					list_add(lista_blocked, hilo_block_mutex);
					//sem_post(&mutex);
					log_info(kernel_logs_obligatorios, "Motivo de Bloqueo: ## (<PID>:%u <TID>:%u) - Bloqueado por: MUTEX", recurso_lock->PID, recurso_lock->TID);
				}
				
				break;
			case MUTEX_UNLOCK:
				t_mutex_todos* recurso_unlock = deserializar_mutex(datos_de_cpu);//Por el momento tengo el pid y tid que invoco junto con el recurso


				//sem_wait(&mutex);
				PCB* pcb_buscadoRU = buscar_proceso(lista_procesos, recurso_unlock->PID);//Buscamos el pcb para poder ingresar a su lista mutex
				//sem_post(&mutex);
				if(pcb_buscadoRU == NULL){
					log_error(kernel_logger,"No se encontro el PID: %u \n", recurso_unlock->PID);
				}

				//1ro) verifico si existe el mutex solicitado
				t_mutex* unlock = buscar_mutex(pcb_buscadoRU->mutex, recurso_unlock->recurso);
				if(unlock == NULL){
					log_error(kernel_logger,"NO EXISTE EL MUTEX\n");
				}

				//2do) Si coinciden que el tid que tiene el recurso en ese momento con el que invoco la syscall -> Se desbloquea el primer hilo(tid) que s encontraba en la cola de bloqueados de ese recurso
				if(unlock->tid == recurso_unlock->TID){
					//El hilo que lo invocó vuelve a la ejecución es decir a la cola de READY
					if (!queue_is_empty(unlock->bloqueados_mutex)){  //Verificamos que la cola bloqueados_mutex no este vacía 
						//uint32_t tid_cola = queue_pop(unlock->bloqueados_mutex);
						uint32_t tid_cola = (uint32_t)(uintptr_t)queue_pop(unlock->bloqueados_mutex); // Saco el primer uint32_t tid de la cola de bloqueados_mutex
						//sem_wait(&mutex);
						TCB* hilo_a_ready = buscar_tcbs(tid_cola, recurso_unlock->PID); //Busco el tcb que tenga el tid que recien saliò de la cola y con su respectivo pid
						//sem_post(&mutex);
						if (hilo_a_ready != NULL) {
							hilo_a_ready->estadoHilo = READY;
							unlock->tid = hilo_a_ready->tid;
							list_add(lista_ready, hilo_a_ready);
						}else{
							log_error(kernel_logger,"No se encontro el TID: %u del PID:%u\n", tid_cola,recurso_unlock->PID);
						}
					}else{
						//digo que el mutex ya no tiene un hilo asignado
						unlock->tid = -1;
					}
				}else{
					//No pasa nada ya que no puede desbloquear nada
				}
				free(recurso_unlock);
				break;
			
			case DUMP_MEMORY:
				t_datos_esenciales* invocadores3 = deserializar_datos_esenciales(datos_de_cpu);
				enviar_memoria_dump_memory(fd_memoria, invocadores3);
				//sem_wait(&mutex);
				TCB* tcb_bloc_dump = buscar_tcbs(invocadores3->tid_inv,invocadores3->pid_inv);
				//sem_post(&mutex);
				//Bloquea el hilo que lo invocó
				tcb_bloc_dump->estadoHilo=BLOCKED;
				log_info(kernel_logs_obligatorios, "Motivo de Bloqueo: ## (<PID>:%u <TID>:%u) - Bloqueado por: DUMP MEMORY", invocadores3->pid_inv,invocadores3->tid_inv);
				
				//sem_wait(&mutex);
				list_add(lista_blocked,tcb_bloc_dump);
				//sem_post(&mutex);
				

				//Aca hay quee crear otro case como se hizo en confirmar_espacio_memoria y confirmar_crear_hilo

				/*char* resultado = recibir_mensajeV2(fd_memoria);
				if(strcmp(resultado, "OK")== 0){
					tcb_bloc_dump->estadoHilo=READY;
					log_info(kernel_logger,"SE DESBLOQUEO el hilo de dump memory\n");
					list_add(lista_ready,tcb_bloc_dump);
				}else{
					finalizar_hilo(tcb_bloc_dump);
				}*/
				break;
			
			case IO:
				t_IO* tiempo_io = deserializar_IO(datos_de_cpu);

				//sem_wait(&mutex);
				TCB* hilo_a_dormir = buscar_tcbs(tiempo_io->TID,tiempo_io->PID);
				//sem_post(&mutex);
				if(hilo_a_dormir == NULL){
					printf("No se encontro el TID: %u del PID:%u\n", tiempo_io->TID,tiempo_io->PID);
				}else{
					hilo_a_dormir->estadoHilo = BLOCKED;
					
					//sem_wait(&mutex);
					list_add(lista_blocked, hilo_a_dormir);
					//sem_post(&mutex);
					log_info(kernel_logs_obligatorios, "Motivo de Bloqueo: ## (PID:%u TID:%u) - Bloqueado por: IO", tiempo_io->PID,tiempo_io->TID);
					//Tiempo que pasará el hilo realizando la operación en Entrada y SAlida
					usleep(tiempo_io->tiempo * 1000);

					//Lo debo sacar de la lista de bloqueados
					//list_remove(lista_blocked,hilo_a_dormir);
					list_remove_element(lista_blocked,hilo_a_dormir);
					log_info(kernel_logs_obligatorios, "Fin de IO: ## (PID:%u TID:%u) finalizó IO y pasa a READY", tiempo_io->PID,tiempo_io->TID);

					hilo_a_dormir->estadoHilo = READY;
					list_add(lista_ready, hilo_a_dormir);

				}
				break;
			
			/*case -1:
				log_error(kernel_logger, "Desconexion de CPU-Dispatch");
				exit(EXIT_FAILURE);*/
			default:
				log_warning(kernel_logger, "Operacion desconocida de CPU-Dispatch\n");
				break;
		}
		eliminar_paquete(datos_de_cpu);
	
	//}
}

bool condicion_pid(PCB* pcb, uint32_t pid){
	return pcb->pid == pid;
}

t_mutex* buscar_mutex(t_list* lista_mutex, char* recurso){
	for(int i=0; i<list_size(lista_mutex);i++)
	{
		t_mutex* mutex_actual = list_get(lista_mutex, i);
		if (strcmp(mutex_actual->recurso, recurso) == 0) {
            return mutex_actual;
        }

	}
	return NULL;
}

void enviar_memoria_dump_memory(int fd_memoria,t_datos_esenciales* invocadores){
	t_paquete* paquete_a_memoria = crear_paquete(DUMP_MEMORY);
    serializar_datos_dump_memory(paquete_a_memoria,invocadores);
	enviar_paquete(paquete_a_memoria, fd_memoria);
	eliminar_paquete(paquete_a_memoria);
}

void serializar_datos_dump_memory(t_paquete* paquete_a_memoria, t_datos_esenciales* invocadores){
	agregar_buffer_Uint32(paquete_a_memoria->buffer, invocadores->pid_inv);
    agregar_buffer_Uint32(paquete_a_memoria->buffer, invocadores->tid_inv);
}

//INTERRUPCION para mandar a cpu interrupt

//PARA MEMORIA
/*
void extraer_datos_kernel_dump_memory(){
	t_paquete* paq_dump_memory = recibir_paquete(fd_kernel_dispatch);
	t_datos_esenciales* datos_dm = deserializar_datos_dump_memory(paq_dump_memory);

	// no sé si va a crear otra struct para asignar los valores de t_datos_esenciales
}
t_datos_esenciales* deserializar_datos_dump_memory(t_paquete* paq_dump_memory){
	t_datos_esenciales* datos_dm = malloc(sizeof(t_datos_esenciales));

	datos_dm->pid_inv = leer_buffer_Uint32(paquete->buffer);
	datos_dm->tid_inv = leer_buffer_Uint32(paquete->buffer);
	return datos_dm;
}*/