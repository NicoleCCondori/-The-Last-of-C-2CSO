#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>

t_log* memoria_logger;
t_log* memoria_log_debug;
t_log* memoria_config;

int fd_memoria;
int fd_cpu;

char* PUERTO_ESCUCHA;
char* puerto_FS;
char* ip_FS;
typedef struct{
	int puerto_escucha;
	char* ip_filesystem;
	char* puerto_filesystem;
	int tam_memoria;
	char* path_instrucciones;
	int retardo_respuesta;
    char* esquema;
    char* algoritmo_busqueda;
    t_list* particiones;
    char* log_level;
}t_config_memoria;

// Declaración de variables

pthread_t hiloFileSystem;

t_log* logger_memoria;
t_config_memoria* config_memoria;
t_config* config;

void levantar_config_memoria(char* config_path);
void iniciar_conexion_con_FS();

//Declaro funciones utiles
char* string_itoa(int number);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);


#endif