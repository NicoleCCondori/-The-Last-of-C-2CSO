#ifndef KERNEL_ESCUCHA_MEMORIA_H_
#define KERNEL_ESCUCHA_MEMORIA_H_

#include <k_conexiones.h>


void kernel_escucha_memoria();

void confirmar_espacio_proceso(void* arg);
void confirmar_creacion_hilo(void* arg);
void confirmar_finalizar_proceso(void* arg);
void confirmar_finalizar_hilo(void* arg);
void confirmar_dump_memory(void* arg);


#endif