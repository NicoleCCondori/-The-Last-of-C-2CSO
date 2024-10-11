#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <utils/utils.h>
//semaforos
sem_t sem_binario_memoria;
sem_t sem_mutex_cola_ready;

uint32_t pid = 0;

void crear_proceso(int tamanio_proceso, char* path, int prioridad_main);
void asignar_espacio_memoria(int fd_memoria, uint32_t pid,int tam_proceso);
TCB* iniciar_hilo(uint32_t tid,int prioridad,uint32_t pid, char* path);
void enviar_a_memoria(int fd_memoria,TCB* hilo_main);
void iniciar_proceso(PCB* proceso_new);

void planificador_de_largo_plazo();

void planificador_corto_plazo();

#endif