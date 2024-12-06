#include <k_conexiones.h>

t_log* kernel_logger=NULL;
t_log* kernel_logs_obligatorios;
t_config_kernel* valores_config_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

<<<<<<< HEAD
=======

//uint32_t tid_main = 0;
>>>>>>> origin/checkpoint3
pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

<<<<<<< HEAD
=======
t_queue* cola_new; //Tiene PCBs
t_queue* cola_exec; //Tiene TCBs
t_queue* cola_blocked;

t_list* lista_ready; //Tiene TCBs
t_list* lista_procesos; //VA a estar compuesto por PCBs
t_list* lista_tcbs; //Va a estar compuesto por TCBs


>>>>>>> origin/checkpoint3
void inicializar_kernel(){
    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    configurar_kernel();

<<<<<<< HEAD
=======
    lista_procesos = list_create();
    lista_tcbs = list_create();
    lista_ready = list_create();

>>>>>>> origin/checkpoint3
}

void configurar_kernel() {
    valores_config_kernel = malloc(sizeof(t_config_kernel));

    valores_config_kernel->config = iniciar_configs("src/kernel.config");

    valores_config_kernel->ip_memoria = config_get_string_value(valores_config_kernel->config,"IP_MEMORIA");
    valores_config_kernel->puerto_escucha= config_get_string_value (valores_config_kernel->config , "PUERTO_ESCUCHA" );
    valores_config_kernel->puerto_memoria = config_get_string_value (valores_config_kernel->config , "PUERTO_MEMORIA" );
	valores_config_kernel->ip_cpu = config_get_string_value (valores_config_kernel->config , "IP_CPU");
	valores_config_kernel->puerto_cpu_dispatch = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_DISPATCH" );
    valores_config_kernel->puerto_cpu_interrupt = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_INTERRUPT" );
    valores_config_kernel->algoritmo_planificacion = config_get_string_value (valores_config_kernel->config , "ALGORITMO_PLANIFICACION");
    valores_config_kernel->quantum = config_get_string_value (valores_config_kernel->config , "QUANTUM");
    valores_config_kernel->log_level = config_get_string_value(valores_config_kernel->config, "LOG_LEVEL");

	//valores_config_kernel = config;
    printf("dsp %s \n",valores_config_kernel->puerto_escucha); //se ASIGNA BIEN

	//free(config);
}
//Mandamos mensaje a memoria para saber si hay espacio disponible
//Se encuentra en planificadores.c por eso lo comento. Tambien  comento en k_conexiones.h
/*void asignar_espacio_memoria(int fd_memoria, uint32_t pid, int tam_proceso){//
    int result;

    //CReamos el buffer para pcb
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
    //Obtenemos el resultado
	recv(fd_memoria, &result, sizeof(int32_t), 0);
	if (result == 0){
        printf("Hay espacio en memoria\n");//debemos crear el espacio de memoria para el proceso?
        //Hacer un signal del sem_binario_memoria
        sem_post(&sem_binario_memoria);
    } else
		printf("No hay espacio en memoria\n");
    
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}*/
//inicializar el hilo  --> LO COMENTO YA QUE ESTÁ EN planificadores.c
/*TCB* iniciar_hilo(uint32_t tid, int prioridad, uint32_t pid){
    
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

    return tcb;
}
*/
//Inicializar el primer proceso   --> LO COMENTO YA QUE ESTÁ EN planificadores.c
/*void iniciar_proceso(int tamanio_proceso){

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
    
    //Planificador de Largo Plazo -> Creación de procesos
    //agregar proceso a cola de NEW 
    queue_push(cola_new,pcb);
    //agregar a la lista de procesos al proceso
    list_add(lista_procesos, pcb);

    //mandamos mensaje a memoria para saber si hay espacio,
    //asignar_espacio_memoria(fd_memoria, pid, tamanio_proceso);
    sem_wait(&sem_binario_memoria);
    
    //Si hay Espacio , pasa a ready al proceso
    PCB* pcb_ready = queue_pop(cola_new); //donde se debería guardar si es que es el hilo el que pasa a ready?
    
    //crea el hilo tid 0
    uint32_t tid_main = 0;
    int prioridad = 0; //la prioridad máxima 0 (cero).

    // Agrega el tid_main a la lista de hilos del proceso (pcb->tid)
    uint32_t* ptr_tid_main = malloc(sizeof(uint32_t));  // Reserva memoria para almacenar tid_main
    *ptr_tid_main = tid_main;
    list_add(pcb_ready->tid, ptr_tid_main);  // Agrega el puntero a la lista de hilos en el PCB

    TCB* hilo_main = malloc(sizeof(TCB));
    hilo_main = iniciar_hilo(tid_main,prioridad,pcb_ready->pid);

    
    //enviar_a_mempria(archivo,tamaño del archivo,tid,pid);
    queue_push(cola_ready,hilo_main); //consulta ¿pasamos a ready el tcb o pcb?
    //crear_hilo(prioridad);
    
}*/

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
    pthread_join(hilo_memoria,NULL);
}

void kernel_escucha_memoria(){
    //atender los msjs de memoria
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

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
/*
void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_cpu_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

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
}*/

void kernel_escucha_cpu_interrupt(){

    //atender los msjs de cpu-interrupt , otra funcion?
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_cpu_interrupt);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de CPU-Interrupt");
			break;
		}
	}
	//close(fd_cpu_interrupt); //liberar_conexion(fd_cpu_interrupt);
}