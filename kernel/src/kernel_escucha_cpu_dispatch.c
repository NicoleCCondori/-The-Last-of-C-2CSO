#include <kernel_escucha_cpu_dispatch.h>
#include <k_conexiones.h>

void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto kernel_escucha_cpu_dispatch.c \n");
    bool control_key = 1;
    while (control_key){

		int cod_op = recibir_operacion(fd_cpu_dispatch);
		//debo hacer los recv para lo que queda del paquete que seria el buffer
		//debo deserializar el buffer según el caso que corresponda
		//log_info(kernel_logs_obligatorios, "## (<%d>:<%d>) - Solicitó syscall: <%s>", pid, tid, nombreSyscall);
		switch (cod_op) //siempre debo saber el pid y el tid de quien invocó
		{
			
			case PROCESS_CREATE:
				//Debo deserializar el buffer que contiene los parametros de nombreAcrh,tamProceso y prioridad del hilo main
	
				//Crea el pcb. Tal vez crear funcion de crear pcb asi no se repite código
				PCB* pcb = malloc(sizeof(PCB));
				if (pcb == NULL){
					printf("Error al crear pcb\n");
				}
				
				pid++;
				pcb->pid = pid;
				pcb->tid = list_create();
				pcb->mutex = list_create();
				pcb->pc = 0;
				pcb->tam_proceso = tamanio_proceso;
				pcb->estado = NEW;	

    			//Consulta a memoria si hay espacio
				if(asignar_espacio_memoria(fd_memoria, pid, tamanio_proceso)){
					// Si hay, crea el tid0. Tambien ver si hacer una función
					iniciar_hilo(tid,prioridad,pid)
					// Se pasa al estado READY
					// recordar agregarlo a la lista_procesos

				} else{ // Si no hay, se lo coloca en la cola NEW
					queue_push(cola_new, pcb);
				}

				//Problema: no quiero duplicar el código con el iniciar_proceso(int tamanio_proceso)}
				// ver tambien si hay funciones en el planificador de largo plazo e la sección de "creación de procesos" y asi los pueda relacionar:
					//Consultar e memoria de si hay espacio
					//Crear PCB y TCB para el tid0
					//Ponerlo en estado NEW
				break;
			
			case PROCESS_EXIT: //Finalizo el pcb correspondiente al tcb, solo lo llama el tid 0
				// Envia todos los TCBs asociado al PCB y los manda a EXIT.
				// Avisar a memoria de la finalizacion

				//Deserializo el buffer y de ahi debo obtener el pid_afuera del proceso
				
				PCB* pcb_afuera = buscar(pid_afuera, lista_procesos);
				if(pcb_afuera == NULL){
					printf("No se encontro el PID: %d", pid_afuera);
				}
				//debo cambiar el estado del pid a EXIT
				pcb_afuera->estado = EXIT;
				int i=0;
				for(i;i<list_size(pcb_afuera->tid);i++){ //libero los tcb
					TCB* tcb = list_get(pcb_afuera->tid,i);
					free(tcb->registro);
					free(tcb);
				}
				list_destroy(pcb_afuera->tid); //destruye la lista

				// Avisar a memoria de la finalizacion
				//creo que seria algo como los cases de cpu pero con memoria, primero el fd_memoria, el pid,cod op y el send
				
				//sacarlo de la lista_procesos
				list_remove_and_destroy_by_condition(lista_procesos, (void*)condicion_pid, pcb_afuera)
				// Colocarlo en la cola EXIT
				queue_push(cola_exit,pid_afuera);
				log_info(kernel_logs_obligatorios,"## Finaliza el proceso <PID %d>", pid_afuera);

				break;
			case THREAD_CREATE://genera un nuevo TCB con un TID autoincremental - mandarlo a READY
				//Deserializamos, debe tener el nombre del archivo y su prioridad
				//debemos ya tener el pid_inv y tid_inv de quien lo invoco
				PCB* pcb_buscado = buscar(pid_inv, lista_procesos);
				if(pcb_buscado == NULL){
					printf("No se encontro el PID: %d", pid_afuera);
				}

				//creo el nuevo tcb con el mismo pid
				//
				iniciar_hilo(); // ver al final como quedo ya que es parecido al process_create
				//debe pasarse al estado ready tcb->estado=ready;
				//agregar el tcb a la lista de tids: list_add()
				//queue_push(cola_ready,ver si quedo con pcb o tcb)
				log_info(kernel_logs_obligatorios, "## (<PID %d>:<TID %d>) Se crea el Hilo - Estado: READY");
				
				break;
			case THREAD_JOIN:
				// Mover el hilo a estado BLOCK
				// Aclaración: el tid pasado por parámetro debe terminar sino el hilo que lo invocó va a seguir en BLOCK
			case THREAD_CANCEL:
				// Finalizar el hilo pasandolo a EXIT
				// Avisar a memoria de la finalizacion.
				// Aclaración: el hilo que lo invocó sigue con su ejecución si es que el hilo pasado por parámetro no existe o finalizo hace rato
			case THREAD_EXIT:
				// Finaliza el hilo que lo invocó


			case MUTEX_CREATE:
				//No lo entendí, crea un nuevo mutex con un semáforo contador = 1?
			case MUTEX_LOCK:
				//1RO debe existir el mutex solicitado
				//2do si no esta tomado -> se lo asigno a dicho hilo
				//3ro si esta tomado -> 
			case MUTEX_UNLOCK:

			case DUMP_MEMORY:
			
			case IO:

			case PAQUETE:

				break;
			case -1:
				log_error(kernel_logger, "Desconexion de CPU-Dispatch");
				exit(EXIT_FAILURE);
			default:
				log_warning(kernel_logger, "Operacion desconocida de CPU-Dispatch");
				break;
		}
	}
	//close(fd_cpu_dispatch); //liberar_conexion(fd_cpu_dispatch);
}

bool condicion_pid(PCB* pcb, uint32_t pid){
	return pcb->pid == pid;
}