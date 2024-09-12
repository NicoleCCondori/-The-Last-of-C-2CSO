#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <utils/utils.h>


typedef struct{
	t_config* config;
	char* puerto_escucha;
	char* mount_dir;
	int block_size;
	int block_count;
	int retardo_acceso_bloque;
	char* log_level;
}t_config_filesystem;

t_log* FS_logger;
t_log* FS_logs_obligatorios;

t_config_filesystem* valores_config_FS;

int fd_memoria;
int fd_FS;

pthread_t hilo_memoria;

//Funciones de inicio
void inicializar_FS();
void configurar_FS();

void conectar_memoria();
void escuchar_memoria();
//void iterator(char* value);



#endif