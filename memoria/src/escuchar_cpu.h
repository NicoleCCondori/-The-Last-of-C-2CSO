#ifndef ESCUCHAR_CPU_H_
#define ESCUCHAR_CPU_H_

#include <utils/utils.h>
#include <utils/serializar.h>


void escuchar_cpu();
void devolver_contexto_ejecucion(t_paquete* paquete);
TCB* buscar_tcb_por_tid(uint32_t tid_buscado);
void actualizar_contexto_de_ejecucion(t_paquete* paquete);
void obtener_instruccion(t_paquete* paquete);
void actualizar_contexto_de_ejecucion(t_paquete* paquete);
void read_mem(t_paquete* paquete);
void write_mem(t_paquete* paquete);

#endif