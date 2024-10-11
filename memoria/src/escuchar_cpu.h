#ifndef ESCUCHAR_CPU_H_
#define ESCUCHAR_CPU_H_

#include <utils/utils.h>
#include <utils/serializar.h>


void escuchar_cpu();
void devolver_contexto_ejecucion();
TCB* buscar_tcb_por_tid(uint32_t tid_buscado);
char* obtener_instruccion();





#endif