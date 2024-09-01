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

#endif