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



/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
int crear_conexion(char* ip, char* puerto,char* name_server);
int iniciar_servidor(char* puerto, t_log* logger, char* msj_server);
int esperar_cliente(int socket_servidor, t_log* logger, char* msj);
int recibir_operacion(int socket_cliente);

//void saludar(char* quien);

#endif
