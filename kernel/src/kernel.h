#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>

#include<commons/collections/list.h>
#include <utils/utils.h>

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>


//  ----Variables------
int kernelConexion;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMOS_PLANIFICACION;
char* QUANTUM;
char* LOG_LEVEL;


t_log* kernelLogger;
t_log* kernelLoggerObs;
t_config* kernelConfig;

pthread_t hiloKernel;

// --------------- Funciones --------------
//  Las cosas que se pueden hacer con el server
void *interacConServer(){
    
}


#endif