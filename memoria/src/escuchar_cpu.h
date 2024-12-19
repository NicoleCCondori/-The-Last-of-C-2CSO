#ifndef ESCUCHAR_CPU_H_
#define ESCUCHAR_CPU_H_

#include <m_conexiones.h>



void escuchar_cpu();
void devolver_contexto_ejecucion(t_paquete* paquete);
void actualizar_contexto_de_ejecucion(t_paquete* paquete);
void obtener_instruccion(t_paquete* paquete);
void read_mem(t_paquete* paquete);
void write_mem(t_paquete* paquete);
char* obtener_instruccion_por_pc(uint32_t pc, char* path);
ContextoEjecucion* buscar_contexto_por_tid(uint32_t tid_buscado, uint32_t pid);

#endif