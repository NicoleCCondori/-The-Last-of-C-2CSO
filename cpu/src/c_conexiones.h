#ifndef C_CONEXIONES_H_
#define C_CONEXIONES_H_

#include <utils/utils.h>

typedef struct{
	t_config* config;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	char* log_level;
}t_config_cpu;

extern t_log* cpu_logger;
extern t_log* cpu_log_debug;
extern t_log* cpu_logs_obligatorios;

extern t_config_cpu* valores_config_cpu;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

extern pthread_t hilo_kernel_dispatch;
extern pthread_t hilo_kernel_interrupt;
extern pthread_t hilo_memoria;
extern uint32_t tidHilo;


void inicializar_cpu();
void configurar_cpu();

void conectar_kernel_dispatch();
void conectar_kernel_interrupt();
void conectar_memoria();

void cpu_escucha_memoria();
void escuchar_kernel_dispatch();
void escuchar_kernel_interrupt();

#endif