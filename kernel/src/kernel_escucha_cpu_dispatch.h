#ifndef KERNEL_ESCUCHA_CPU_DISPATCH_H_
#define KERNEL_ESCUCHA_CPU_DISPATCH_H_

#include <k_conexiones.h>

t_mutex* buscar_mutex(t_list* lista_mutex, char* recurso);

void enviar_memoria_dump_memory(int fd_memoria,t_datos_esenciales* invocadores);

void serializar_datos_dump_memory(t_paquete* paquete_a_memoria, t_datos_esenciales* invocadores);

bool condicion_pid(PCB* pcb, uint32_t pid);

void kernel_escucha_cpu_dispatch();

#endif