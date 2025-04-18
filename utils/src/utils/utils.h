#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/config.h>

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdint.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
	CONFIRMAR_ESPACIO_PROCESO,
	CONFIRMAR_CREACION_HILO,
	OBTENER_CONTEXTO,
	ACTUALIZAR_CONTEXTO,
	OBTENER_INSTRUCCION,
	ENVIAR_CONTEXTO,
	ENVIAR_INSTRUCCION,
	ASIGNAR_MEMORIA,

	HILO_READY,
	RECIBIR_TID,

	PROCESS_CREATE,
	PROCESS_EXIT,
	THREAD_CREATE,
	THREAD_JOIN,
	THREAD_CANCEL,
	THREAD_EXIT,
	MUTEX_CREATE,
	MUTEX_LOCK,
	MUTEX_UNLOCK,
	DUMP_MEMORY,
	IO,
	SYSCALL,
	FINALIZAR_HILO,
	FINALIZAR_PROCESO,
	WRITE_MEM,
	READ_MEM,
	CONFIRMAR_CONTEXTO_ACTUALIZADO,
	CONFIRMAR_DUMP_MEMORY,
	INTERRUPCION,
	ERROR,
	CONFIRMAR_WRITE_MEM,
	ENVIAR_READ_MEM,
	CONFIRMAR_INTERRUPCION
}op_code;

typedef enum{
	NEW,
	BLOCKED,
	READY,
	EXIT,
	EXEC
} estado_proceso_hilo;
typedef struct
{
	int size;
	uint32_t offset;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
	uint32_t EX;
	uint32_t FX;
	uint32_t GX;
	uint32_t HX;
} RegistrosCPU;

typedef struct 
{
	uint32_t pid; //Identificador del proceso
	t_list* lista_tid; //Lista de los identificadores de los hilos asociados al proceso
	t_list* mutex; //Lista de los mutex creados para el proceso a lo largo de la ejecución de sus hilos
	uint32_t tid_contador;
	//uint32_t pc; //Program Counter, indica la próxima instrucción a ejecutar
	estado_proceso_hilo estado; //para saber en que estado se encuntra el proceso/hilo
	int tam_proceso;
	int prioridad_main;
	char* path_main;
} PCB;

typedef struct 
{
	uint32_t pid; //Identificador del proceso al que pertenece
	uint32_t tid; //Identificador del hilo
	uint32_t tid_que_lo_bloqueo;
	int prioridad;//0 maxima prioridad
	RegistrosCPU* registro;
	char* path;
	uint32_t pc; //Program Counter, indica la próxima instrucción a ejecutar
	estado_proceso_hilo estadoHilo;
} TCB;

//semaforos
// Inicialización del semáforo binario
extern sem_t semaforo_binario;  // Declaración del semáforo binario

//listas
/*extern t_list* lista_tcbs;*/

PCB* buscar_proceso(t_list* lista_procesos, uint32_t pid);
/*TCB* buscar_tcbs(uint32_t tid,uint32_t pid);*/

//Funciones para cliente
int crear_conexion(char* ip, char* puerto,char* name_server,t_log *logger);
void handshakeClient(int fd_servidor, int32_t handshake);

//Funciones para servidor
int iniciar_servidor(char* puerto, t_log *logger, char* msj_server);
int esperar_cliente(int socket_servidor, t_log* nomre_logger, char* name_cliente);
int recibir_operacion(int socket_cliente);
void handshakeServer(int fd_client);

//Globales
t_log *iniciar_logger(char *path_log, char *nombre_log);
t_config* iniciar_configs(char* path_config);
void finalizar_modulo(t_log* logger, t_log* logger_obligatorio, t_config* config);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* logger);
char* recibir_mensajeV2(int socket);
t_list* recibir_paquete_lista(int socket_cliente);
void mostrar_valor_semaforo(sem_t *sem);
#endif
