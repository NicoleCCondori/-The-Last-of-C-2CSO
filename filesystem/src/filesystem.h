#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct{
	int puerto_escucha;
	char* mount_dir;
	int block_size;
	int block_count;
	int retardo_acceso_bloque;
	char* log_level;
}t_config_filesystem;

// Declaración de variables

pthread_t hiloMemoria;

t_log* logger_FS;
t_config_filesystem* config_filesystem;
t_config* config;
char* puerto;


int servidor_fileSystem;
int cliente_memoria;

//Funciones de inicio
void levantar_config_FS (char* config_path);
void iniciar_conexiones_FS();
void conectarMemoria(int socket_servidor, t_log* logger_FS, char* moduloCLiente);
void atender_memoria();
void iterator(char* value);

//Declaro funciones utiles
char* string_itoa(int number);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);



#endif