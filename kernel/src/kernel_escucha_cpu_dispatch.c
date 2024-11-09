#include <kernel_escucha_cpu_dispatch.h>
#include <k_conexiones.h>
#include <planificadores.h>

void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto kernel_escucha_cpu_dispatch.c \n");
    bool control_key = 1;
    while (control_key){

		//int cod_op = recibir_operacion(fd_cpu_dispatch);
		//debo hacer los recv para lo que queda del paquete que seria el buffer
		//debo deserializar el buffer según el caso que corresponda
		t_paquete* datos_de_cpu = recibir_paquete(fd_cpu_dispatch);
		t_datos_esenciales* invocadores = deserializar_datos_esenciales(datos_de_cpu);
		
		log_info(kernel_logs_obligatorios, "## (<%d>:<%d>) - Solicitó syscall: <%d>", invocadores->pid_inv,invocadores->tid_inv, datos_de_cpu->codigo_operacion);
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
				PCB* pcb_afuera = buscar_proceso(lista_procesos, invocadores->pid_inv);
				if(pcb_afuera == NULL){
					printf("No se encontro el PID: %d", invocadores->pid_inv);
				}
				finalizar_proceso(pcb_afuera);
				
				log_info(kernel_logs_obligatorios,"## Finaliza el proceso <PID %d>", invocadores->pid_inv);
				break;
			case THREAD_CREATE://genera un nuevo TCB con un TID autoincremental - mandarlo a READY
				//Deserializamos, debe tener el nombre del archivo y su prioridad
				t_thread_create* tc_hilo = deserializar_thread_create(datos_de_cpu);
				//debemos ya tener el pid_inv y tid_inv de quien lo invoco
				PCB* pcb_buscado = buscar_proceso(lista_procesos, invocadores->pid_inv);
				if(pcb_buscado == NULL){
					printf("No se encontro el PID: %d", invocadores->pid_inv);
				}

				//creo el nuevo tcb con el mismo pid
				iniciar_hilo(invocadores->tid_inv,tc_hilo->prioridadH,invocadores->pid_inv,tc_hilo->nombreArchT);
				//iniciar_hilo(); // ver al final como quedo ya que es parecido al process_create
				//debe pasarse al estado ready tcb->estado=ready;
				//agregar el tcb a la lista de tids: list_add()
				//queue_push(cola_ready,ver si quedo con pcb o tcb)
				log_info(kernel_logs_obligatorios, "## (<PID %d>:<TID %d>) Se crea el Hilo - Estado: READY",invocadores->pid_inv, invocadores->tid_inv);
				
				break;
			case THREAD_JOIN:
				// Mover el hilo a estado BLOCK
				// Aclaración: el tid pasado por parámetro debe terminar sino el hilo que lo invocó va a seguir en BLOCK
				uint32_t tid_join = deserializar_thread_join_y_cancel(datos_de_cpu);
				TCB* tcb_a_bloquear = buscar_tcbs(lista_tcbs,invocadores->tid_inv ,invocadores->pid_inv);
				TCB* tcb_buscado = buscar_tcbs(lista_tcbs, tid_join,invocadores->pid_inv);
				if(tcb_buscado == NULL || tcb_buscado->estadoHilo == EXIT){
					printf("No se encontro o ya FINALIZO el TID: %d del PID: %d ", tid_join,invocadores->pid_inv);
					tcb_a_bloquear->estadoHilo = READY;
					list_add(lista_ready,tcb_a_bloquear);
				}else{
					tcb_a_bloquear->estadoHilo = BLOCKED;
					queue_push(cola_blocked,tcb_a_bloquear);
				}

				break;
			case THREAD_CANCEL:
				// Finalizar el hilo pasandolo a EXIT
				// Avisar a memoria de la finalizacion.
				// Aclaración: el hilo que lo invocó sigue con su ejecución si es que el hilo pasado por parámetro no existe o finalizo hace rato
				uint32_t tid_cancel = deserializar_thread_join_y_cancel(datos_de_cpu);
				break;
			case THREAD_EXIT:
				// Finaliza el hilo que lo invocó --> conectar con la funcion
				break;
			case MUTEX_CREATE:
				char* recurso_create = deserializar_mutex(datos_de_cpu);
				t_mutex* creado = malloc(sizeof(t_mutex));
				if(creado == NULL){
					printf("Error al crear el mutex\n");
				}
				creado->recurso = recurso_create;
				creado->tid = -1;  //Acá todavia no se le asigna ningún hilo
				creado->bloqueados_mutex = queue_create();
				//CONSULTAR: El tp da una sugerencia de usar un semaforo contador 
					//Para mutex_lock necesito verificar si esta sin asignar el mut deberia usar sem_trywait??? rta: no es necesario
				//sem_init(&(creado->contador),0,1); //semáforo contador = 1, es decir, sin asignar
				
				PCB* pcb_buscadoRC = buscar_proceso(lista_procesos, invocadores->pid_inv);
				if(pcb_buscadoRC == NULL){
					printf("No se encontro el PID: %d", invocadores->pid_inv);
				}
				list_add(pcb_buscadoRC->mutex, creado); //las estructura t_mutex lo agregamos en la lista mutex de cada pcb
				break;
			case MUTEX_LOCK:
				
				char* recurso_lock = deserializar_mutex(datos_de_cpu);

				PCB* pcb_buscadoRL = buscar_proceso(lista_procesos, invocadores->pid_inv);
				if(pcb_buscadoRL == NULL){
					printf("No se encontro el PID: %d", invocadores->pid_inv);
				}
				//1RO debe existir el mutex solicitado
				t_mutex* lockeado = buscar_mutex(pcb_buscadoRL->mutex, recurso_lock);
				if(lockeado == NULL){
					printf("NO EXISTE EL MUTEX\n");
				}
				// está asignado?
				if(lockeado->tid == -1){ //2do si no esta tomado -> se lo asigno a dicho hilo
					lockeado->tid = invocadores->tid_inv;
				}else{ //3ro si esta tomado -> 
					queue_push(lockeado->bloqueados_mutex,invocadores->tid_inv);
				}
				
				break;
			case MUTEX_UNLOCK:
				char* recurso_unlock = deserializar_mutex(datos_de_cpu);
				PCB* pcb_buscadoRU = buscar_proceso(lista_procesos, invocadores->pid_inv);
				if(pcb_buscadoRU == NULL){
					printf("No se encontro el PID: %d", invocadores->pid_inv);
				}
				//1RO verifico si existe el mutex solicitado
				t_mutex* unlock = buscar_mutex(pcb_buscadoRU->mutex, recurso_unlock);
				if(unlock == NULL){
					printf("NO EXISTE EL MUTEX\n");
				}
				if(unlock->tid == invocadores->tid_inv){
					int tid_cola = queue_pop(unlock->bloqueados_mutex);
					unlock->tid = tid_cola;
					//El hilo que lo invocó vuelve a la ejecución es decir a la cola de READY
				}else{
					//No pasa nada ya que no puede desbloquear nada
				}
					
				break;
			case DUMP_MEMORY:
				enviar_memoria_dump_memory(fd_memoria, invocadores);
				//Bloquea el hilo que lo invocó

				//Aplicar un semaforo
				//espera la rta de la memoria por medio del semaforo
					//se desbloquea y pasa a ready
				//Si da error --> el proceso pasa a EXIT
				break;
			
			case IO:
				int tiempo_io = deserializar_IO(datos_de_cpu);
				//Tiempo que pasará el hilo realizando la operación en Entrada y SAlida
				
				break;
			case PAQUETE:

				break;
			case -1:
				log_error(kernel_logger, "Desconexion de CPU-Dispatch");
				exit(EXIT_FAILURE);
			default:
				log_warning(kernel_logger, "Operacion desconocida de CPU-Dispatch");
				break;
		}
		//destruir_buffer_paquete(datos_de_cpu);
	}
	//close(fd_cpu_dispatch); //liberar_conexion(fd_cpu_dispatch);
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
	destruir_buffer_paquete(paquete_a_memoria);
}

void serializar_datos_dump_memory(t_paquete* paquete_a_memoria, t_datos_esenciales* invocadores){
	agregar_buffer_Uint32(paquete_a_memoria->buffer, invocadores->pid_inv);
    agregar_buffer_Uint32(paquete_a_memoria->buffer, invocadores->tid_inv);
}
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