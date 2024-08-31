#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
t_log* cpu_logger;
t_log* cpu_log_debug;
t_log* cpu_config;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;

t_log* cpu_logger;
t_log* cpu_logs_obligatorios;
t_config* cpu_config;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* LOG_LEVEL;

#endif