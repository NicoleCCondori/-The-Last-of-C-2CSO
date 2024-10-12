#include <planificadores.h>
#include <k_conexiones.h>


void planificador_de_largo_plazo()
{
    //sem_init(&sem_binario_memoria, 0, 0);// semaforo binario para otorgarle espacio de memoria a un proceso nuevo
    //sem_init(&sem_binario_memoria, 1, 0);//es uno porque lo compartiria con memoria
    //sem_init(&sem_mutex_cola_ready,0,1);//mutex de cola de ready

    while(1){
        int prioridad = 0;
        crear_proceso(tamanio_proceso_main, archivo_pseudocodigo_main, prioridad);
       
    }
}

void planificador_corto_plazo(TCB* hilo){
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){
        printf("Planificacion fifo\n");
        
        sem_wait(&sem_mutex_cola_ready);
        list_add(lista_ready, hilo);
        sem_post(&sem_mutex_cola_ready);
        
        TCB* hilo_exec = list_get(lista_ready,1);

        queue_push(cola_exec, hilo_exec);

        //mandar a cpu tid y pid

        t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);
        serializar_hilo_cpu(hilo_cpu, hilo_exec->pid, hilo_exec->tid);

        enviar_paquete(hilo_cpu, fd_cpu_dispatch);
        // 

    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){
        printf("Planificacion prioridades\n");

        buscar_hilo_mayor_prioridad(hilo);
        
    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){
        printf("Planificacion CMN\n");

    }
}
void buscar_hilo_mayor_prioridad(TCB* hilo){
    return 0;
}

//Mandamos hilo a memoria
void enviar_a_memoria(int fd_memoria,TCB* hilo){
    t_paquete* paquete_hilo = crear_paquete(HILO_READY);
    serializar_hilo_ready(paquete_hilo, hilo);
    enviar_paquete(paquete_hilo, fd_memoria);
}

//Mandamos mensaje a memoria para saber si hay espacio disponible
void asignar_espacio_memoria(uint32_t pid, int tam_proceso, int prioridad, char* path_main){//
    int result;

    t_paquete* paquete_asignar_memoria = crear_paquete(ASIGNAR_MEMORIA);
    serializar_asignar_memoria(paquete_asignar_memoria, pid, tam_proceso);
    
    enviar_paquete(paquete_asignar_memoria, fd_memoria);
    
    //recibimos la respuesta de memoria/ podemos directamente usar semaforos con memoria
	recv(fd_memoria, &result, sizeof(int32_t), 0);
	if (result == 0){
        printf("Hay espacio en memoria\n");//debemos crear el espacio de memoria para el proceso?
        
        //Si hay Espacio se crea el hilo main
        //crea el hilo tid 0
        uint32_t tid_main = 0;

        // Agrega el tid_main a la lista de hilos del proceso (pcb->tid)
        PCB* proceso_ready = buscar_proceso(lista_procesos, pid);//puede que necesitemos hacer un malloc

        if(proceso_ready!= NULL){
            list_add(proceso_ready->tid, &tid_main);  // Agrega el hilo main a la lista de hilos del proceso
        }
        queue_pop(cola_new);
        
        //CREACION DE HILO MAIN
        TCB* hilo_main = malloc(sizeof(TCB));//liberar
        hilo_main = iniciar_hilo(tid_main, prioridad,proceso_ready->pid,proceso_ready->path_main);

        //informar a memoria
        enviar_a_memoria(fd_memoria,hilo_main);

        //encolar el hilo a la cola de ready
       
        planificador_corto_plazo(hilo_main);

        //mandar_hilo_a_cola_ready(hilo_main);
        //queue_push(cola_ready,hilo_main); //consulta ¿pasamos a ready el tcb o pcb?
    } else {
		printf("No hay espacio en memoria\n");
        //sem_wait(&sem_binario_memoria);

    }

    destruir_buffer_paquete(paquete_asignar_memoria);
/* No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
*/
}

//CREACION DE HILO
TCB* iniciar_hilo(uint32_t tid, int prioridad, uint32_t pid,char* path){
    
    TCB* tcb = malloc(sizeof(TCB));
    if(tcb == NULL){
        printf("Error al crear TCB\n");
    }
    
    tcb->tid = tid;
    tcb->pid = pid;
    tcb->prioridad = prioridad;
    tcb->path = strdup(path);

    log_info(kernel_logs_obligatorios, "Creación de Hilo: “## (<PID>: %u <TID>: %u) Se crea el Hilo - Estado: READY”", tcb->pid, tcb->tid);
    return tcb;
}

//INICIAR_PROCESO
void iniciar_proceso()
{
    PCB* proceso_new = queue_peak(cola_new);

    if (proceso_new != NULL) {
        //mandamos mensaje a memoria para saber si hay espacio,
        asignar_espacio_memoria(proceso_new->pid, proceso_new->tam_proceso, proceso_new->prioridad_main,proceso_new->path_main);

        
        
    } else {
        printf("La cola está vacía.\n");

    }
}

//CREACION DE PROCESO
void crear_proceso(int tamanio_proceso,char* path, int prioridad_main)
{

    PCB* pcb = malloc(sizeof(PCB));//liberar
    if (pcb == NULL){
        printf("Error al crear memoria para pcb\n");
    }
    
    pid++;
    pcb->pid = pid;
    pcb->tid = list_create();
    pcb->mutex = list_create();
    //pcb->pc = 0;
    pcb->tam_proceso = tamanio_proceso;
    pcb->estado = NEW;
    pcb->prioridad_main = prioridad_main;
    pcb->path_main = strdup(path);

    //Planificador de Largo Plazo -> Creación de procesos
    log_info(kernel_logs_obligatorios, " Creación de Proceso: ## (<PID>: %u) Se crea el proceso - Estado: NEW",pcb->pid);
    log_info(kernel_logger, "La prioridad del hilo main es: %u ", pcb->prioridad_main);

    //agregar proceso a cola de NEW 
    queue_push(cola_new, pcb);
    //agregar a la lista de procesos al proceso
    list_add(lista_procesos, pcb);
    
    iniciar_proceso();
    
  
    //liberar pcb_new
    //free(pcb_new);

    //liberar hilo_main
 }

void mensaje_finalizar_proceso(int fd_memoria,uint32_t pid){

}

bool condicion_pid(PCB* pcb, uint32_t pid){
	return pcb->pid == pid;
}
//FINALIZACION DE PROCESO
void* finalizar_proceso(PCB* pcb_afuera)
{
    //informar a memoria la finalizacion del proceso
    //mensaje_finalizar_proceso(fd_memoria,proceso->pid);
    //confirmacion de parte de memoria

    log_info(kernel_logs_obligatorios, "Fin de Proceso: ## Finaliza el proceso <PID>: %u", pcb_afuera->pid);


    //liberar proceso
    //debo cambiar el estado del pid a EXIT
    pcb_afuera->estado = EXIT;

	int i=0;
	for(i; i<list_size(pcb_afuera->tid);i++){ //libero los tcb
		int32_t* tid = list_get(pcb_afuera->tid,i);
		free(tid);
	}
	list_destroy(pcb_afuera->tid); //destruye la lista

	// Avisar a memoria de la finalizacion
	//creo que seria algo como los cases de cpu pero con memoria, primero el fd_memoria, el pid,cod op y el send
				
	//sacarlo de la lista_procesos
	list_remove_and_destroy_by_condition(lista_procesos, (void*)condicion_pid, pcb_afuera);

    //iniciar otro proceso que estaba en new

    iniciar_proceso();

    return 0;
}
//FINALIZACION DE HILO
void* finalizar_hilo(TCB* hilo)
{
    //informar a memoria

    log_info(kernel_logs_obligatorios, "Fin de Hilo: ## (<PID>: %u <TID>: %u ) Finaliza el hilo", hilo->pid, hilo->tid);
    //liberar tcb

    //mover al estado ready a todos los hilos bloqueados por este hilo??
    return 0;
}
