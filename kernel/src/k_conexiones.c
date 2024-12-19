#include <k_conexiones.h>

t_log* kernel_logger;
t_log* kernel_logs_obligatorios;
t_config_kernel* valores_config_kernel;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

char* archivo_pseudocodigo_main;
int tamanio_proceso_main;


//uint32_t tid_main = 0;
pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

t_queue* cola_new; //Tiene PCBs
t_queue* cola_exec; //Tiene TCBs

t_list* lista_ready; //Tiene TCBs
t_list* lista_blocked; //me va ayudar a buscar en toda la lista
t_list* lista_exit; //Es mejor que tenga los TCBs sirve para los thread_join
t_list* lista_procesos; //VA a estar compuesto por PCBs
t_list* lista_tcbs; //Va a estar compuesto por TCBs

//semaforos
sem_t sem_binario_memoria;
sem_t sem_mutex_cola_ready;

sem_t mutex;
sem_t sem_plani_largo_plazo;
sem_t TCBaPlanificar;

uint32_t pid=0;


void inicializar_kernel(){

    sem_init(&mutex, 0, 1);
    sem_init(&sem_plani_largo_plazo, 0, 1);
    sem_init(&TCBaPlanificar, 0, 0);

    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    configurar_kernel();

    //Definimos las listas
    lista_procesos = list_create();
    lista_tcbs = list_create();
    lista_ready = list_create();
    lista_blocked = list_create();
    lista_exit = list_create();
    //definimos las colas
    cola_new = queue_create();
    cola_exec = queue_create();

}

void configurar_kernel() {
    valores_config_kernel = malloc(sizeof(t_config_kernel));

    valores_config_kernel->config = iniciar_configs("src/kernel.config");

    valores_config_kernel->ip_memoria = config_get_string_value(valores_config_kernel->config,"IP_MEMORIA");
    valores_config_kernel->puerto_memoria = config_get_string_value (valores_config_kernel->config , "PUERTO_MEMORIA" );
	valores_config_kernel->ip_cpu = config_get_string_value (valores_config_kernel->config , "IP_CPU");
	valores_config_kernel->puerto_cpu_dispatch = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_DISPATCH" );
    valores_config_kernel->puerto_cpu_interrupt = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_INTERRUPT" );
    valores_config_kernel->algoritmo_planificacion = config_get_string_value (valores_config_kernel->config , "ALGORITMO_PLANIFICACION");
    valores_config_kernel->quantum = config_get_string_value (valores_config_kernel->config , "QUANTUM");
    valores_config_kernel->log_level = config_get_string_value(valores_config_kernel->config, "LOG_LEVEL");

}

//Revisar mas adelante
void conectar_cpu_dispatch(){
    //Cliente KERNEL a CPU-dispatch
	fd_cpu_dispatch = crear_conexion(valores_config_kernel->ip_memoria, valores_config_kernel->puerto_cpu_dispatch, "CPU - Dispatch",kernel_logger);
	handshakeClient(fd_cpu_dispatch,2);
    
	//hilo para conectarse con cpu Dispatch / atender
    pthread_create(&hilo_cpu_dispatch, NULL, (void*)kernel_escucha_cpu_dispatch,NULL);
    pthread_detach(hilo_cpu_dispatch);
}

void conectar_cpu_interrupt(){
    //Cliente KERNEL a CPU-interrupt
	fd_cpu_interrupt = crear_conexion(valores_config_kernel->ip_cpu, valores_config_kernel->puerto_cpu_interrupt, "CPU - Interrupt",kernel_logger);
	handshakeClient(fd_cpu_interrupt,2);
    
    //hilo para conectarse con cpu - Interrupt / atender
    pthread_create(&hilo_cpu_interrupt, NULL, (void*)kernel_escucha_cpu_interrupt,NULL);
    pthread_detach(hilo_cpu_interrupt);
}

void conectar_memoria(){
    //cliente KERNEL - MEMORIA
    fd_memoria = crear_conexion(valores_config_kernel->ip_cpu,valores_config_kernel->puerto_memoria,"MEMORIA",kernel_logger);
    handshakeClient(fd_memoria,2);

    //hilo para conectarse con memoria / atender
    pthread_create(&hilo_memoria, NULL, (void*)kernel_escucha_memoria,NULL);
    pthread_detach(hilo_memoria);
}
void planificador_cortoPlazo(){
    pthread_t hilo_planificador_corto_plazo;
    pthread_create (&hilo_planificador_corto_plazo, NULL, (void*)planificador_corto_plazo, NULL);
    pthread_detach (hilo_planificador_corto_plazo);
}

void planificador_largoPlazo(){
    pthread_t hilo_planificador_largo_plazo;
    pthread_create (&hilo_planificador_largo_plazo, NULL, (void*)planificador_de_largo_plazo, NULL);
    pthread_join (hilo_planificador_largo_plazo,NULL);
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
    
    log_info(kernel_logger,"el pid a enviar a memoria es: %u , el tid: %u y prioridad: %d ",hilo->pid,hilo->tid,hilo->prioridad);
    serializar_hilo_ready(paquete_hilo, hilo->pid, hilo->tid, hilo->prioridad, hilo->path);
    enviar_paquete(paquete_hilo, fd_memoria);
    log_info(kernel_logger, "se envio el paquete");
    eliminar_paquete(paquete_hilo);
    log_info(kernel_logger, "se elimina el paquete");
}

//Mandamos mensaje a memoria para saber si hay espacio disponible
void asignar_espacio_memoria(uint32_t pid, int tam_proceso, int prioridad, char* path_main){//

    t_paquete* paquete_asignar_memoria = crear_paquete(ASIGNAR_MEMORIA);
    serializar_asignar_memoria(paquete_asignar_memoria, pid, tam_proceso);
    enviar_paquete(paquete_asignar_memoria, fd_memoria);
    
    log_info(kernel_logger,"Enviamos a memoria pid y tam_proceso\n");

    eliminar_paquete(paquete_asignar_memoria);
}

//CREACION DE HILO
void crear_hilo(uint32_t pid_confirmado){
        printf("Hay espacio en memoria\n");//Si hay Espacio se crea el hilo main con tid 0
        //Agregar el tid_main a la lista de hilos del proceso (pcb->tid). 1) Debemos identificar el pcb en la lista de procesos
        log_info(kernel_logger,"El pid es: %u",pid_confirmado);

        PCB* proceso_agregar_tidM = buscar_proceso(lista_procesos, pid_confirmado);
        if(proceso_agregar_tidM == NULL ){
            log_info(kernel_logger, "No encontro el proceso_agregar_tidM\n");
        }

        log_info(kernel_logger,"El tid es: %u",proceso_agregar_tidM->tid_contador);
        
        uint32_t tid_main = proceso_agregar_tidM->tid_contador;
        
        uint32_t* tid_copia = malloc(sizeof(uint32_t));//se libera en finalizar el proceso junto con la lista tid
	    *tid_copia = tid_main;

        if(proceso_agregar_tidM!= NULL){
            list_add(proceso_agregar_tidM->lista_tid, tid_copia);
            log_info(kernel_logger, "Agrego el hilo main a la lista de hilos del proceso"); // Agrega el hilo main a la lista de hilos del proceso
        }
        //2) Sacamos el pcb de la cola NEW, queue_pop retorna un valor
        PCB* pcb_fuera_new = queue_pop(cola_new); 
        if(pcb_fuera_new != NULL){ 
           log_info(kernel_logger,"El proceso que salió de la cola NEW para ir a READY es %d",pcb_fuera_new->pid);
        } 
        
        //CREACION DE HILO MAIN
        log_info(kernel_logger, "Envio datos tid %u, prioridad %u, pid %u, path",  pcb_fuera_new->tid_contador, pcb_fuera_new->prioridad_main,pcb_fuera_new->pid);
        TCB* hilo_main = iniciar_hilo(pcb_fuera_new->tid_contador, pcb_fuera_new->prioridad_main,pcb_fuera_new->pid, pcb_fuera_new->path_main);

        //informar a memoria el HILO A CREAR
        log_info(kernel_logger,"Enviamos el hilo a memoria");
        list_add(lista_tcbs, hilo_main);//Meterlo en la lista de TCBs general
        enviar_a_memoria(fd_memoria,hilo_main);
        //ESPERAMOS QUE MEMORIA NOS CONFIEME SI PUDO CREAR EL HILO
        
        free(proceso_agregar_tidM);
        //free(pcb_fuera_new);
}

void confirmacion_crear_hilo(uint32_t pid_hilo,uint32_t tid_hilo){
    TCB* hilo_pasar_a_ready = buscar_tcbs(lista_tcbs, tid_hilo, pid_hilo);
    if(hilo_pasar_a_ready == NULL ){
            log_info(kernel_logger, "No se puede agregar un hilo nulo a lista_ready");
            return;
    } else {
        log_info(kernel_logger, "Encontro el hilo_pasar_a_ready\n");
        if (lista_ready == NULL) {
            log_error(kernel_logger, "La lista lista_ready no está inicializada");
            return;
        }
        log_info(kernel_logger, "Puntero lista_ready: %p", lista_ready);
        log_info(kernel_logger, "Puntero hilo_pasar_a_ready: %p", hilo_pasar_a_ready);

        list_add(lista_ready, hilo_pasar_a_ready);
        log_info(kernel_logger, "Agrego hilo a la lista ready");}
}
//INICIAR_HILO
TCB* iniciar_hilo(uint32_t tid, int prioridad, uint32_t pid,char* path){
    
    TCB* tcb = malloc(sizeof(TCB));
    if(tcb == NULL){
        printf("Error al crear TCB\n");
    }
    
    tcb->tid = tid;
    tcb->pid = pid;
    tcb->tid_que_lo_bloqueo = -1; //xq aún no esta bloqueado por nadie
    tcb->prioridad = prioridad;
    tcb->path = strdup(path);
    tcb->estadoHilo = READY;

    log_info(kernel_logs_obligatorios, "Creacion de Hilo: “## (<PID>: %u <TID>: %u) Se crea el Hilo - Estado: READY”", tcb->pid, tcb->tid);
    return tcb;
}

//INICIAR_PROCESO
void iniciar_proceso(){
    PCB* proceso_new = queue_peek(cola_new);

    if (proceso_new != NULL) {
        //mandamos mensaje a memoria para saber si hay espacio,
        log_info(kernel_logger,"El pid es: %u",proceso_new->pid);
        
        asignar_espacio_memoria(proceso_new->pid, proceso_new->tam_proceso, proceso_new->prioridad_main, proceso_new->path_main);
        
    } else {
        printf("La cola esta vacia.\n");

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
    pcb->lista_tid = list_create(); //LIBERAR MEMORI
    pcb->tid_contador = 0;
    pcb->mutex = list_create(); //LIBERAR MEMORIA
    //pcb->pc = 0;
    pcb->tam_proceso = tamanio_proceso;
    pcb->estado = NEW;
    pcb->prioridad_main = prioridad_main;
    pcb->path_main = strdup(path);

    //Planificador de Largo Plazo -> Creación de procesos
    log_info(kernel_logs_obligatorios, "Creacion de Proceso: ## (<PID>: %u) Se crea el proceso - Estado: NEW",pcb->pid);
    log_info(kernel_logger, "La prioridad del hilo main es: %u ", pcb->prioridad_main);

    //agregar proceso a cola de NEW 
    queue_push(cola_new, pcb);
    //agregar a la lista de procesos al proceso
    list_add(lista_procesos, pcb);
    

    //iniciar_proceso();
    
  
    //liberar pcb_new
    //free(pcb_new);
    //free(pcb->path_main);
    //free(pcb);
    //liberar hilo_main
 }

void destruir_pcb(void* elemento) {
    PCB* pcb = (PCB*)elemento; // Convertir void* a PCB*
    free(pcb); // Liberar la memoria del PCB
}
//FINALIZACION DE PROCESO
void* finalizar_proceso(PCB* pcb_afuera)
{
    printf("Inicio para Finalizar un Proceso");
    //informar a memoria la finalizacion del proceso
    //mensaje_finalizar_proceso(fd_memoria,proceso->pid);
    //confirmacion de parte de memoria
    char* mensaje = informar_a_memoria_fin_proceso(fd_memoria, pcb_afuera->pid);
    if(strcmp(mensaje, "OK")== 0){
        log_info(kernel_logger,"Recibí el OK de parte de memoria (Finalizacion del proceso)");
        log_info(kernel_logs_obligatorios, "Fin de Proceso: ## Finaliza el proceso <PID>: %u", pcb_afuera->pid);
        
        //Si la lista pcb_afuera->tid solo tiene enteros solo hago un destroy, ya que no es necesario
        /*for(int i=0; i<list_size(pcb_afuera->tid);i++){ //libero los tcb
            uint32_t* tid = list_get(pcb_afuera->tid, i);
            free(tid);
        }*/
        //1ro) Veo en la lista de tcbs -->los que tengan el mismo pid los paso al estado EXIT
        for(int i=0; i<list_size(lista_tcbs);i++){
            TCB* tcb_en_lista = (TCB*)list_get(lista_tcbs,i);
            if(tcb_en_lista->pid == pcb_afuera->pid){
                finalizar_hilo(tcb_en_lista);
                //en esta misma funcion ya destruye el tid que esta en la lista de tids del pcb
            }
        }
        //2do) destruyo la lista de tids(enteros) que tiene el pcb -> por las dudas jsjs
        list_destroy(pcb_afuera->lista_tid); 
        //list_destroy_and_destroy_elements(pcb_afuera->tid,free);
        
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
        }*/
        //debo actualizar el estado del proceso (pcb)
        pcb_afuera->estado = EXIT;
        pcb_afuera->tid_contador = -1;
        pcb_afuera->prioridad_main = -1;

        //iniciar otro proceso que estaba en new
        iniciar_proceso();
    
    }else {
        log_error(kernel_logger, "Error al recibir el mensaje de memoria (Finalizacion del proceso)");
    }
    free(mensaje);

    return NULL;
}

char* informar_a_memoria_fin_proceso(int fd_memoria,uint32_t pid){
    t_paquete* paquete_memoria_fp = crear_paquete(FINALIZAR_PROCESO);
    agregar_buffer_Uint32(paquete_memoria_fp->buffer, pid);
    enviar_paquete(paquete_memoria_fp,fd_memoria);
    eliminar_paquete(paquete_memoria_fp);

    char* mensaje = recibir_mensajeV2(fd_memoria);
    return mensaje;
}

//FINALIZACION DE HILO
void finalizar_hilo(TCB* hilo)
{
    log_info(kernel_logs_obligatorios, "Fin de Hilo: ## (<PID>:%u <TID>:%u ) Finaliza el hilo", hilo->pid, hilo->tid);
    //1ro) Ingreso el hilo a la lista de exit
    hilo->estadoHilo = EXIT;
    list_add(lista_exit, hilo);
    
    //2do) lo saco de la lista de tcbs
    
    /*for(int i =0; i<list_size(lista_tcbs);i++){
        TCB* tcb_en_lista = (TCB*)list_get(lista_tcbs, i);
        if(tcb_en_lista->pid == hilo->pid && tcb_en_lista->tid == )
        list_remove_element (lista_tcbs, (void *) tcb_en_lista);
    }*/
    //list_remove_element(lista_tcbs,(void*)hilo);  CREO QUE ES MEJOR CONSERVARLO PARA SABER EL ESTADO

    //3ro) lo saco de la lista tid de su pcb -> debo buscar en la lista de procesos con el pid
    PCB* tid_a_retirar = buscar_proceso(lista_procesos, hilo->pid);
    if(tid_a_retirar == NULL){
		printf("No se encontro el PID: %d", hilo->pid);
        return;
	}
    //list_remove_element(tid_a_retirar->tid,(void*) hilo->tid);
    list_remove_element(tid_a_retirar->lista_tid, (void *)(uintptr_t)hilo->tid);

    //3ro a) ver que onda con los recursos de mutex-> deberia liberarlo tmb
    for(int i=0; i<list_size(tid_a_retirar->mutex); i++){
        t_mutex* mutex_pos = list_get(tid_a_retirar->mutex, i);
        if(mutex_pos->tid == hilo->tid){
            if (!queue_is_empty(mutex_pos->bloqueados_mutex)) {
                TCB* sgte_hilo = queue_pop(mutex_pos->bloqueados_mutex);

                sgte_hilo->estadoHilo = READY;
                mutex_pos->tid = sgte_hilo->tid;

                list_add(lista_ready, sgte_hilo);
            } else {
                //Si no hay hilos bloqueados, asignar el tid como vacio
                mutex_pos->tid = -1;
            }
        }
    }

    //4to)informar a memoria
    informar_a_memoria_fin_hilo(fd_memoria,hilo);
    
    //5to) Pasar los hilos que estaban bloqueados a ready
    //mover al estado ready a todos los hilos bloqueados por este hilo
    desbloquear_hilos_por_tid(hilo);

    //6to)liberar tcb
    free(hilo);

}
void informar_a_memoria_fin_hilo(int fd_memoria, TCB* hilo){
    t_paquete* paquete_memoria = crear_paquete(FINALIZAR_HILO);
    serializar_finalizar_hilo(paquete_memoria,hilo->pid,hilo->tid);
    enviar_paquete(paquete_memoria, fd_memoria);
    eliminar_paquete(paquete_memoria);

    char* mensaje = recibir_mensajeV2(fd_memoria);
    if(strcmp(mensaje, "OK") == 0){
        log_info(kernel_logger,"Recibi el OK de parte de memoria (Finalizacion del hilo)");
    }else {
        log_error(kernel_logger, "Error al recibir el mensake de memoria (Finalizacion del hilo)");
    }
    free(mensaje);
}

void desbloquear_hilos_por_tid(TCB* hilo){
    for(int i=0; i<list_size(lista_blocked); i++){
        TCB* hilo_bloqueado = list_get(lista_blocked,i);
        if(hilo_bloqueado->pid==hilo->pid && hilo_bloqueado->tid_que_lo_bloqueo==hilo->tid){
            hilo_bloqueado->estadoHilo = READY;
            hilo_bloqueado->tid_que_lo_bloqueo = -1;
            list_add(lista_ready,hilo_bloqueado);
            //lo debo sacar de la lista de blqueados
            list_remove(lista_blocked, i);
            i--; //se decrementa ya que elimine un hilo de la lista
        }           
    }
}


void liberar_recursos_kernel(){
    //liberar los logs y config
    finalizar_modulo(kernel_logger, kernel_logs_obligatorios,valores_config_kernel->config);
    free(valores_config_kernel);

    //finalizar las conexiones
    if (fd_memoria >= 0) close(fd_memoria);
    if (fd_cpu_dispatch >= 0) close(fd_cpu_dispatch);
    if (fd_cpu_interrupt>= 0) close(fd_cpu_interrupt);
    
    //liberar semaforos
    sem_destroy(&mutex);
    sem_destroy(&sem_plani_largo_plazo);
    sem_destroy(&TCBaPlanificar);

    //destruir listas
    /*list_destroy_and_destroy_elements(lista_blocked);
    list_destroy_and_destroy_elements(lista_exit);
    list_destroy_and_destroy_elements(lista_procesos);
    list_destroy_and_destroy_elements(lista_ready);
    list_destroy_and_destroy_elements(lista_tcbs);
    */
    //destruir cola
    /*queue_destroy_and_destroy_elements(cola_exec);
    queue_destroy_and_destroy_elements(cola_new);
    */
    log_info(kernel_logger,"Todos los recursos de kernel han sido liberados");
}