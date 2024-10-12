#include <k_conexiones.h>

t_log* kernel_logger=NULL;
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

t_queue* cola_new;
t_queue* cola_exec;
t_queue* cola_blocked;

t_list* lista_ready;
t_list* lista_procesos;


void inicializar_kernel(){
    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    configurar_kernel();

    lista_procesos = list_create();
    lista_ready = list_create();
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

	//valores_config_kernel = config;

	//free(config);
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
/*
void planificador_de_largo_plazo(){
    while(1){
        int tam_proceso_main= tamanio_proceso_main;
        iniciar_proceso(tam_proceso_main);
    }
}


void planificador_corto_plazo(){
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){
        printf("Planificacion fifo\n");
    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){
        printf("Planificacion prioridades\n");
    }
    if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){
        printf("Planificacion CMN\n");
    }
}
*/

