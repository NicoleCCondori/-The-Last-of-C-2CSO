#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

t_log* kernel_logger;
t_log* kernel_logs_obligatorios;
t_config* kernel_config;

char* IP_MEMORIA;
char* PUERTO_ESCUCHA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
char* QUANTUM;
char* LOG_LEVEL;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;
pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

void inicializar_kernel();
void crearHilos();

void conexion_cpu_dispatch(){  //preguntar
 
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
}

void conexion_cpu_interrupt(){

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

#endif