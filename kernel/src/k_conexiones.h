#ifndef K_CONEXIONES_H_
#define K_CONEXIONES_H_

#include <utils/utils.h>
//#include <utils/serializar.h>

//#include <kernel_escucha_cpu_dispatch.h>

typedef struct{
	t_config* config;
	char* ip_memoria;
	char* puerto_escucha;
	char* puerto_memoria;
	char* ip_cpu;
	char* puerto_cpu_dispatch;
	char* puerto_cpu_interrupt;
	char* algoritmo_planificacion;
	char* quantum;
	char* log_level;
}t_config_kernel;

 typedef struct {
        uint32_t pid;
        int tam_proceso;
    } t_asignar_memoria;

//Variables globales
extern t_log* kernel_logger;
extern t_log* kernel_logs_obligatorios;

extern t_config_kernel* valores_config_kernel;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_memoria;
extern char* archivo_pseudocodigo_main;
extern int tamanio_proceso_main;

extern pthread_t hilo_cpu_dispatch;
extern pthread_t hilo_cpu_interrupt;
extern pthread_t hilo_memoria;

//Colas para diagrama de 5 estados
extern t_queue* cola_new;
extern t_queue* cola_exec;
extern t_queue* cola_ready;
extern t_queue* cola_blocked;


//Listas
extern t_list* lista_procesos;

//semaforos
//sem_t sem_binario_memoria;

//inicializacion
void inicializar_kernel();
void configurar_kernel();

//iniciar proceso
//void iniciar_proceso(int tamanio_proceso);
//void asignar_espacio_memoria(int fd_memoria, uint32_t pid,int tam_proceso);
//TCB* iniciar_hilo(uint32_t tid,int prioridad,uint32_t pid);
//void enviar_a_memoria(int fd_memoria,TCB* hilo_main);

void conectar_cpu_dispatch();
void conectar_cpu_interrupt();
void conectar_memoria();

void kernel_escucha_cpu_dispatch();
void kernel_escucha_cpu_interrupt();
void kernel_escucha_memoria();

//planificadores
void planificador_cortoPlazo();
void planificador_largoPlazo();
void planificador_corto_plazo();
void planificador_de_largo_plazo();

#endif
