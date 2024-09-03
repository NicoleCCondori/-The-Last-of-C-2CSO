#ifndef UTILS_H_
#define UTILS_H_

#include<stdlib.h>
#include<stdio.h>

#include<commons/log.h>
#include<commons/config.h>

#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include <pthread.h>
#include<signal.h>

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


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





void finalizar_modulo(t_log* logger, t_config* config);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* logger);
t_list* recibir_paquete(int socket_cliente);

#endif
