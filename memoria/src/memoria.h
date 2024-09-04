#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>

#include<commons/collections/list.h>

#include <utils/utils.h>

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <utils/hello.h>

t_log* memoryLogger;
t_log* memoryLoggerObs;
t_config* memoryConfig;

char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
char* TAM_MEMORIA;
char* PATH_INSTRUCCIONES;
char* RETARDO_RESPUESTA;
char* ESQUEMA;
char* ALGORITMO_BUSQUEDA;
char* PARTICIONES;
char* LOG_LEVEL;


#endif