#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct{
	t_config* config;
	char* puerto_escucha;
	char* mount_dir;
	int block_size;
	int block_count;
	int retardo_acceso_bloque;
	char* log_level;
}t_config_filesystem;

// Declaración de variables

pthread_t hiloMemoria;

t_log* logger_FS;
t_log* memoria_logs_obligatorios;
int cliente_memoria;
int servidor_fileSystem;


//Funciones de inicio
t_config_filesystem* configurar_FS();
void iniciar_conexiones_FS();
void conectarMemoria(int socket_servidor, t_log* logger_FS, char* moduloCLiente);
void atender_memoria();
void iterator(char* value);

//Declaro funciones utiles
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);



#endif