#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>

char* instruccionActual;
char* operacionAux;
uint32_t programCounterCpu;
// Struc para dividir la instruccion
typedef struct 
{
	char* operacion;
	char* operando1;
	char* operando2;

}t_instruccion;
/* Podriamos utilizar listas con un vector en su interior para segmentar la linea
	stru
*/

typedef struct{
	t_config* config;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	char* log_level;
}t_config_cpu;
//Registros de la cpu


t_log* cpu_logger;
t_log* cpu_log_debug;
t_log* cpu_logs_obligatorios;

t_config_cpu* valores_config_cpu;
//File descriptors
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;
//Hilos 
pthread_t hilo_kernel_dispatch;
pthread_t hilo_kernel_interrupt;
pthread_t hilo_memoria;




void inicializar_cpu();
void configurar_cpu();

void conectar_kernel_dispatch();
void conectar_kernel_interrupt();
void conectar_memoria();

void cpu_escucha_memoria();
void escuchar_kernel_dispatch();
void escuchar_kernel_interrupt();

#endif