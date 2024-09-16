#include <k_conexiones.h>

t_log* kernel_logger=NULL;
t_log* kernel_logs_obligatorios;
t_config_kernel* valores_config_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;
uint32_t pid = 0;

pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

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

//Inicializar el primer proceso
 void iniciar_proceso(char* archivo_pseudocodigo,int tamanio_proceso){

    PCB* pcb = malloc(sizeof(PCB));
    if (pcb == NULL){
        printf("Error al crear pcb");
    }
    
    pid++;
    pcb->pid = pid;
    pcb->tid = list_create();
    pcb->mutex = list_create();
    pcb->pc = 0;
    pcb->registro->AX = 0;
    pcb->registro->BX = 0;
    pcb->registro->CX = 0;
    pcb->registro->DX = 0;
    pcb->registro->EX = 0;
    pcb->registro->FX = 0;
    pcb->registro->GX = 0;
    pcb->registro->HX = 0;
    pcb->tam_proceso = tamanio_proceso;
    pcb->estado = NEW;

    //mandamos mensaje a memoria para saber si hay espacio

    //SI HAY Espacio , pasa a ready al proceso

    //crea el hilo tid 0
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
