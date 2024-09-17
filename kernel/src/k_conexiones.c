#include <k_conexiones.h>

t_log* kernel_logger=NULL;
t_log* kernel_logs_obligatorios;
t_config_kernel* valores_config_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;
uint32_t pid = 0;
//uint32_t tid_main = 0;
pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

t_queue* cola_new;
t_queue* cola_exec;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;

void inicializar_kernel(){
    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    configurar_kernel();

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
//Mnadamos mensaje a memoria para saber si hay espacio disponible
void asignar_espacio_memoria(int fd_memoria, int tamanio_proceso, PCB* pcb){
    int result;

	send(fd_memoria, &tamanio_proceso, sizeof(int), 0);
	recv(fd_memoria, &result, sizeof(int32_t), 0);

	if (result == 0)
		printf("Hay espacio en memoria\n");
        //Hacer un signal del sem_binario_memoria
	else
		printf("No hay espacio en memoria\n");
}
//inicializar el hilo
TCB* iniciar_hilo(uint32_t tid, int prioridad, uint32_t pid){
    
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
}

//Inicializar el primer proceso
void iniciar_proceso(char* archivo_pseudocodigo,int tamanio_proceso){

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
    
    //agregar proceso a cola de NEW 
    queue_push(cola_new,pcb);
    
    //mandamos mensaje a memoria para saber si hay espacio,
    
    //asignar_espacio_memoria(int fd_memoria, int tamanio_proceso, PCB* pcb);
    //wait (sem_binario_memoria);
    
    //SI HAY Espacio , pasa a ready al proceso
    PCB* pcb_ready = queue_pop(cola_new); //donde se debería guardar si es que es el hilo el que pasa a ready?
    //crea el hilo tid 0
    uint32_t tid_main = 0;
    list_add(pcb->tid, tid_main);
    int prioridad = 0; //la prioridad máxima 0 (cero).
    TCB* hilo_main = malloc(sizeof(TCB));
    hilo_main = iniciar_hilo(tid_main,prioridad,pid);
    queue_push(cola_ready,hilo_main);//consulta ¿pasamos a ready el tcb o pcb?
    crear_hilo(,prioridad);
    
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
    pthread_join(hilo_memoria,NULL);
}
