#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>

typedef struct{
	t_config* config;
	char* puerto_escucha;
	char* ip_filesystem;
	char* puerto_filesystem;
	char* tam_memoria;
	char* path_instrucciones;
	char* retardo_respuesta;
    char* esquema;
    char* algoritmo_busqueda;
    t_list* particiones;
    char* log_level;
}t_config_memoria;

t_log* memoria_logger;
t_log* memoria_log_obligatorios;

t_config_memoria* valores_config_memoria;

int fd_memoria;
int fd_FS;
int fd_cpu;
int fd_kernel;

pthread_t hilo_FS;
pthread_t hilo_cpu;
pthread_t hilo_kernel;

void inicializar_memoria();
void configurar_memoria();

void conectar_con_FS();
void conectar_cpu();
void conectar_kernel();

void memoria_escucha_FS();
void memoria_escucha_cpu();
void memoria_escucha_kernel();

#endif