#ifndef K_CONEXIONES_H_
#define K_CONEXIONES_H_

#include <utils/utils.h>
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

//Variables globales
extern t_log* kernel_logger;
extern t_log* kernel_logs_obligatorios;

extern t_config_kernel* valores_config_kernel;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_memoria;

extern pthread_t hilo_cpu_dispatch;
extern pthread_t hilo_cpu_interrupt;
extern pthread_t hilo_memoria;

//Colas para diagrama de 5 estados
extern t_queue* cola_new;
extern t_queue* cola_exec;
extern t_queue* cola_ready;
extern t_queue* cola_blocked;
extern t_queue* cola_exit;

//Listas
extern t_list* lista_procesos;

void inicializar_kernel();
void configurar_kernel();
void iniciar_proceso(char* archivo_pseudocodigo,int tamanio_proceso);
void asignar_espacio_memoria(int fd_memoria, int tamanio_proceso, PCB* pcb);
TCB* iniciar_hilo(uint32_t tid,int prioridad,uint32_t pid);

void conectar_cpu_dispatch();
void conectar_cpu_interrupt();
void conectar_memoria();

void kernel_escucha_cpu_dispatch();
void kernel_escucha_cpu_interrupt();
void kernel_escucha_memoria();

void planificador_cortoPlazo();
void planificador_largoPlazo();
void planificador_corto_plazo();
void planificador_largo_plazo();

#endif
