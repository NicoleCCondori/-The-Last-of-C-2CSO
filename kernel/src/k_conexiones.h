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

void inicializar_kernel();
void configurar_kernel();

void conectar_cpu_dispatch();
void conectar_cpu_interrupt();
void conectar_memoria();

void kernel_escucha_cpu_dispatch();
void kernel_escucha_cpu_interrupt();
void kernel_escucha_memoria();

#endif
