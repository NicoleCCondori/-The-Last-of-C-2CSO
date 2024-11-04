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
				//
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
				break;
			case THREAD_CANCEL:
				// Finalizar el hilo pasandolo a EXIT
				// Avisar a memoria de la finalizacion.
				// Aclaración: el hilo que lo invocó sigue con su ejecución si es que el hilo pasado por parámetro no existe o finalizo hace rato
				uint32_t tid_cancel = deserializar_thread_join_y_cancel(datos_de_cpu);
				break;
			case THREAD_EXIT:
				// Finaliza el hilo que lo invocó
				break;
			case MUTEX_CREATE:
				//No lo entendí, crea un nuevo mutex con un semáforo contador = 1?
				char* recurso_create = deserializar_mutex(datos_de_cpu);
				break;
			case MUTEX_LOCK:
				//1RO debe existir el mutex solicitado
				//2do si no esta tomado -> se lo asigno a dicho hilo
				//3ro si esta tomado -> 
				char* recurso_lock = deserializar_mutex(datos_de_cpu);
				break;
			case MUTEX_UNLOCK:
				char* recurso_unlock = deserializar_mutex(datos_de_cpu);
				break;
			case DUMP_MEMORY:
			
			case IO:
				int tiempo_io = deserializar_IO(datos_de_cpu);
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