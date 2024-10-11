#include <planificadores.h>
#include <k_conexiones.h>




void planificador_de_largo_plazo()
{

    sem_init(&sem_binario_memoria, 0, 0);// semaforo binario para otorgarle espacio de memoria a un proceso nuevo
    //sem_init(&sem_binario_memoria, 1, 0);//es uno porque lo compartiria con memoria
    sem_init(&sem_mutex_cola_ready,0,1);//mutex de cola de ready

    while(1){
        int prioridad = 0;
        crear_proceso(tamanio_proceso_main, archivo_pseudocodigo_main, prioridad);
       
    }
}

void planificador_corto_plazo(TCB* hilo){
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){
        printf("Planificacion fifo\n");
        sem_wait(&sem_mutex_cola_ready);
        queue_push(cola_ready,hilo);
        sem_post(&sem_mutex_cola_ready);
        
    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){
        printf("Planificacion prioridades\n");
        
    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){
        printf("Planificacion CMN\n");

    }
}



//Mandamos hilo a memoria
void enviar_a_memoria(int fd_memoria,TCB* hilo){
    t_paquete* paquete_hilo = crear_paquete(HILO_READY);
    serealizar_hilo_ready(paquete_hilo, hilo);
    enviar_paquete(paquete_hilo, fd_memoria);
}

//Mandamos mensaje a memoria para saber si hay espacio disponible
void asignar_espacio_memoria(int fd_memoria, uint32_t pid, int tam_proceso){//
    int result;
/*
    //Creamos el buffer para pcb
    t_buffer* buffer = malloc(sizeof(t_buffer)); //liberar
    buffer->size = sizeof(uint32_t)
                    + sizeof(int);
    
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size); //liberar

    memcpy(buffer->stream + buffer->offset, &pid, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, &tam_proceso, sizeof(int));
    buffer->offset += sizeof(int);

    //llenar el paquete con el buffer
    t_paquete* paquete = malloc(sizeof(t_paquete)); //liberar
    paquete->codigo_operacion= ASIGNAR_MEMORIA;
    paquete->buffer = buffer;

    //Armamos el stream a enviar
    void* a_enviar = malloc(buffer->size + sizeof(uint8_t)+sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset,&(paquete->codigo_operacion),sizeof(uint8_t));
    offset+= sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    // Por último enviamos
    send(fd_memoria, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);
*/

    t_paquete* paquete_asignar_memoria = crear_paquete(ASIGNAR_MEMORIA);
    serializar_asignar_memoria(paquete_asignar_memoria, pid, tam_proceso);
    enviar_paquete(paquete_asignar_memoria, fd_memoria);
    
    //sem_wait(&sem_binario_memoria);
    
    //recibimos la respuesta de memoria/ podemos directamente usar semaforos con memoria
	recv(fd_memoria, &result, sizeof(int32_t), 0);
	if (result == 0){
        printf("Hay espacio en memoria\n");//debemos crear el espacio de memoria para el proceso?
        //Hacer un signal del sem_binario_memoria
        sem_post(&sem_binario_memoria);//provocaria varias recursos no necesarios. deberi ir en memoria
    } else {
		printf("No hay espacio en memoria\n");
        sem_wait(&sem_binario_memoria);
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
    tcb->registro->AX = 0;
    tcb->registro->BX = 0;
    tcb->registro->CX = 0;
    tcb->registro->DX = 0;
    tcb->registro->EX = 0;
    tcb->registro->FX = 0;
    tcb->registro->GX = 0;
    tcb->registro->HX = 0;
    tcb->path = strdup(path);
    //tcb->path_length = strlen(path)+1;
    log_info(kernel_logs_obligatorios, "Creación de Hilo: “## (<PID>: %u <TID>: %u) Se crea el Hilo - Estado: READY”", tcb->pid, tcb->tid);
    return tcb;
}

//INICIAR_PROCESO
void iniciar_proceso(PCB* proceso_new)
{
    if (proceso_new != NULL) {
        //mandamos mensaje a memoria para saber si hay espacio,
        asignar_espacio_memoria(fd_memoria, proceso_new->pid, proceso_new->tam_proceso);

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
    pcb->pc = 0;
    pcb->tam_proceso = tamanio_proceso;
    pcb->estado = NEW;
    
    //Planificador de Largo Plazo -> Creación de procesos
    log_info(kernel_logs_obligatorios, " Creación de Proceso: ## (<PID>: %u) Se crea el proceso - Estado: NEW",pcb->pid);

    //agregar proceso a cola de NEW 
    queue_push(cola_new, pcb);
    //agregar a la lista de procesos al proceso
    list_add(lista_procesos, pcb);
    
    PCB* pcb_new = queue_pop(cola_new);

    iniciar_proceso(pcb_new);
    
    
    //Si hay Espacio se crea el hilo main
    //crea el hilo tid 0
    uint32_t tid_main = 0;

    // Agrega el tid_main a la lista de hilos del proceso (pcb->tid)
    list_add(pcb->tid, tid_main);  // Agrega el hilo main a la lista de hilos del proceso
    //queue_push(cola_ready,pcb_ready); //el proceso no pasa a la cola de ready
    
    //CREACION DE HILO MAIN
    TCB* hilo_main = malloc(sizeof(TCB));//liberar
    hilo_main = iniciar_hilo(tid_main,prioridad_main,pcb->pid,archivo_pseudocodigo_main);

    //imformar a memoria
    enviar_a_memoria(fd_memoria,hilo_main);

    //encolar el hilo a la cola de ready
    planificador_corto_plazo(hilo_main);

    //mandar_hilo_a_cola_ready(hilo_main);
    //queue_push(cola_ready,hilo_main); //consulta ¿pasamos a ready el tcb o pcb?
    
    //liberar pcb_new
    free(pcb_new);

    //liberar hilo_main
 }

//FINALIZACION DE PROCESO
void* finalizar_proceso(PCB* proceso)
{
    //informar a memoria la finalizacion del proceso
    mensaje_finalizar_proceso(fd_memoria,proceso->pid);
    //confirmacion de parte de memoria

    log_info(kernel_logs_obligatorios, "Fin de Proceso: “## Finaliza el proceso <PID>: %u”", proceso->pid);


    //liberar proceso

    //iniciar otro proceso que estaba en new
    
    PCB* pcb_new = queue_pop(cola_new);

    iniciar_proceso(pcb_new);
    
}
//FINALIZACION DE HILO
void* finalizar_hilo(TCB* hilo)
{
    //informar a memoria

    log_info(kernel_logs_obligatorios, "Fin de Hilo: “## (<PID>: %u <TID>: %u ) Finaliza el hilo”", hilo->pid, hilo->tid);
    //liberar tcb

    //mover al estado ready a todos los hilos bloqueados por este hilo??
}