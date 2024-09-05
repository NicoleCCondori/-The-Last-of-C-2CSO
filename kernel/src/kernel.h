#ifndef KERNEL_H_
#define KERNEL_H_

#include <utils/utils.h>

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

t_log* kernel_logger;
t_log* kernel_logs_obligatorios;

t_config_kernel* valores_config_kernel;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

void inicializar_kernel();
void configurar_kernel();

void conectar_cpu_dispatch();
void conectar_cpu_interrupt();
void conectar_memoria();

void kernel_escucha_cpu_dispatch();
void kernel_escucha_cpu_interrupt();
void kernel_escucha_memoria();


#endif