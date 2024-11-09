#include <planificadores.h>
#include <k_conexiones.h>

//semaforos
sem_t sem_binario_memoria;
sem_t sem_mutex_cola_ready;

sem_t TCBaPlanificar;

uint32_t pid=0;

void planificador_de_largo_plazo()
{
    //sem_init(&sem_binario_memoria, 0, 0);// semaforo binario para otorgarle espacio de memoria a un proceso nuevo
    //sem_init(&sem_binario_memoria, 1, 0);//es uno porque lo compartiria con memoria
    //sem_init(&sem_mutex_cola_ready,0,1);//mutex de cola de ready
    //inicializar semáforos
    sem_init(&TCBaPlanificar, 0, 0);
    while(1){
        int prioridad = 0;
        crear_proceso(tamanio_proceso_main, archivo_pseudocodigo_main, prioridad);
       
    }
}

void planificador_corto_plazo(/*TCB* hilo*/){
    while(1){
        sem_wait(&TCBaPlanificar); //mientras haya tcbs en la lista_ready
        if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){
            printf("Planificacion fifo\n");
            
            /*sem_wait(&sem_mutex_cola_ready);
            list_add(lista_ready, hilo);
            sem_post(&sem_mutex_cola_ready);*/
            
            TCB* hilo_exec = list_get(lista_ready,0);
            //CAmbiar el estado del hilo
            hilo_exec->estadoHilo = EXEC;
            //LO agrego en la cola_exec
            queue_push(cola_exec, hilo_exec);

            //mandar a cpu tid y pid
            t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);
            serializar_hilo_cpu(hilo_cpu, hilo_exec->pid, hilo_exec->tid);

            enviar_paquete(hilo_cpu, fd_cpu_dispatch);
            // 

        }
        if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){
            printf("Planificación prioridades\n");
            //La máxima prioridad es el 0
            //1ro saber cuál es la menor prioridad ---> Menor le decimos al numero entero --> En sí sería la de MAXIMA PRIORIDAD
            TCB* hiloMayorPrioridad =buscar_hilo_menorNro_prioridad(); //Acá esta el nro entero más grande
            
            //Ahora el hiloMayorPrioridad es el hilo que tiene como prioridad el numero entero más chico
            //CAmbiar el estado del hilo
            hiloMayorPrioridad->estadoHilo = EXEC;
            //LO agrego en la cola_exec
            queue_push(cola_exec,hiloMayorPrioridad);

            //mandar a cpu tid y pid
            t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);
            serializar_hilo_cpu(hilo_cpu, hiloMayorPrioridad->pid, hiloMayorPrioridad->tid);

            enviar_paquete(hilo_cpu, fd_cpu_dispatch);
        }
        if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){
            printf("Planificacion CMN\n");
            int mayorNroPrioridad = buscar_hilo_mayorNroPrioridad(); //SIrve para saber la cantidad de colas a crear
            t_queue* cola_prioridad[mayorNroPrioridad+1];
            for(int i=0; i <= mayorNroPrioridad; i++){
                cola_prioridad[i] = queue_create();
            }
            for(int i=0; i< list_size(lista_ready); i++){
                TCB* hilo = list_get(lista_ready, i);
                if(hilo->prioridad <= mayorNroPrioridad){
                    queue_push(cola_prioridad[hilo->prioridad], hilo);
                }
            }

        }
    }
}
int buscar_hilo_mayorNroPrioridad(){
    TCB* hiloMayorNroPrioridad = list_get(lista_ready,0);
    for(int i=1; i< list_size(lista_ready); i++){
        TCB* hiloMuchoMay = list_get(lista_ready,i);
        if(hiloMuchoMay->prioridad > hiloMayorNroPrioridad->prioridad){ //comparo con enteros
            hiloMayorNroPrioridad = hiloMuchoMay;
        }
    }
    int mayorNroPrioridad = hiloMayorNroPrioridad->prioridad;
    return mayorNroPrioridad;
}

TCB* buscar_hilo_menorNro_prioridad(){
    TCB* hiloMenorNroPrioridad = list_get(lista_ready,0);
    for(int i=1; i< list_size(lista_ready); i++){
        TCB* hiloMuchoMenor = list_get(lista_ready,i);
        if(hiloMuchoMenor->prioridad < hiloMenorNroPrioridad->prioridad){ 
            hiloMenorNroPrioridad = hiloMuchoMenor;
        }
    }
    return hiloMenorNroPrioridad;
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
        PCB* proceso_agregar_tidM = buscar_proceso(lista_procesos, pid);//puede que necesitemos hacer un malloc

        if(proceso_agregar_tidM!= NULL){
            list_add(proceso_agregar_tidM->tid, &tid_main);  // Agrega el hilo main a la lista de hilos del proceso
        }

        //los queue_pop retornan un valor
        PCB* pcb_fuera_new = malloc(sizeof(PCB));
        pcb_fuera_new = queue_pop(cola_new); 
        if(pcb_fuera_new != NULL){
           printf("El proceso que salió de la cola NEW para ir a READY es %d\n",pcb_fuera_new->pid);  
        } 
        
        //CREACION DE HILO MAIN
        TCB* hilo_main = iniciar_hilo(tid_main, prioridad,proceso_agregar_tidM->pid,proceso_agregar_tidM->path_main);

        //informar a memoria
        enviar_a_memoria(fd_memoria,hilo_main);

        //Meterlo en la lista de TCBs general
        list_add(lista_tcbs, hilo_main);

        //Agregar el TCB tmb en la lista_ready va a ayudar para los algoritmos de corto plazo
        //Pensar en la idea de usar semáforos
        list_add(lista_ready,hilo_main);
        sem_post(&TCBaPlanificar);
        //planificador_corto_plazo(hilo_main);
        planificador_corto_plazo();
        //mandar_hilo_a_cola_ready(hilo_main);
        //queue_push(cola_ready,hilo_main); //consulta ¿pasamos a ready el tcb o pcb?
    } else {
		printf("No hay espacio en memoria\n");
        //sem_wait(&sem_binario_memoria);
        //Se mantiene en el estado NEW
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
    tcb->estadoHilo = READY;

    log_info(kernel_logs_obligatorios, "Creación de Hilo: “## (<PID>: %u <TID>: %u) Se crea el Hilo - Estado: READY”", tcb->pid, tcb->tid);
    return tcb;
}

//INICIAR_PROCESO
void iniciar_proceso(){
    PCB* proceso_new = queue_peek(cola_new);

    if (proceso_new != NULL) {
        //mandamos mensaje a memoria para saber si hay espacio,
        asignar_espacio_memoria(proceso_new->pid, proceso_new->tam_proceso, proceso_new->prioridad_main, proceso_new->path_main);

        
        
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
void destruir_pcb(void* elemento) {
    PCB* pcb = (PCB*)elemento; // Convertir void* a PCB*
    free(pcb); // Liberar la memoria del PCB
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

    //Si la lista pcb_afuera->tid solo tiene enteros solo hago un destroy, ya que no es necesario
	/*for(int i=0; i<list_size(pcb_afuera->tid);i++){ //libero los tcb
		uint32_t* tid = list_get(pcb_afuera->tid, i);
		free(tid);
	}*/
	list_destroy(pcb_afuera->tid); //destruye la lista

	// Avisar a memoria de la finalizacion
	//creo que seria algo como los cases de cpu pero con memoria, primero el fd_memoria, el pid,cod op y el send
				
	//sacarlo de la lista_procesos
    for (int i = 0; i < list_size(lista_procesos); i++) {
        PCB* pcb_en_lista = (PCB*)list_get(lista_procesos, i);
        if (pcb_en_lista->pid == pcb_afuera->pid) {
            // Encontramos el PCB a eliminar
            list_remove_and_destroy_element(lista_procesos, i, destruir_pcb);
            break; // Salir del bucle después de eliminar
        }
    }

    //iniciar otro proceso que estaba en new

    iniciar_proceso();

    return NULL;
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
