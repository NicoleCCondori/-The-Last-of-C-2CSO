#ifndef M_CONEXIONES_H_
#define M_CONEXIONES_H_

#include <utils/serializar.h>


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
    char** particiones;
    char* log_level;
}t_config_memoria;

typedef struct {
    size_t inicio;
    size_t tamanio;
    bool libre;
    uint32_t pid;
} Particion;

extern t_log* memoria_logger;
extern t_log* memoria_log_obligatorios;

extern t_config_memoria* valores_config_memoria;

extern int fd_memoria;
extern int fd_FS;
extern int fd_cpu;
extern int fd_kernel;

extern pthread_t hilo_FS;
extern pthread_t hilo_cpu;
extern pthread_t hilo_kernel;

extern void* memoria;
extern t_list* lista_particiones;
extern int tamanio_memoria;

void inicializar_memoria();
void configurar_memoria();
void configurar_particiones();

void conectar_con_FS();
void conectar_cpu();
void conectar_kernel();

void memoria_escucha_FS();
void escuchar_cpu();
void escuchar_kernel();

void inicializar_lista_tcb();
#endif