#ifndef F_CONEXIONES_H_
#define F_CONEXIONES_H_

//#include <utils/utils.h>
#include <utils/serializar.h>

typedef struct{
	t_config* config;
	char* puerto_escucha;
	char* mount_dir;
	int block_size;
	int block_count;
	int retardo_acceso_bloque;
	char* log_level;
}t_config_filesystem;

typedef struct{

	char* nombre_archivo;
	uint32_t size;
	uint32_t index_block;
}archivo_metadata;

//Variables globales
extern t_log* FS_logger;
extern t_log* FS_logs_obligatorios;

extern t_config_filesystem* valores_config_FS;

extern int fd_memoria;
extern int fd_FS;

extern pthread_t hilo_memoria;
extern char *bitmap_path;
extern char *bloques_path;
extern char *metadata_path;

//Funciones de inicio
void inicializar_FS();
void configurar_FS();

void conectar_memoria();
void escuchar_memoria();
//void iterator(char* value);

#endif