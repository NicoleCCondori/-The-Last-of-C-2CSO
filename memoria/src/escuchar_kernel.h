#ifndef ESCUCHAR_KERNEL_H_
#define ESCUCHAR_KERNEL_H_

#include <m_conexiones.h>


void escuchar_kernel();

void crear_proceso(void* arg);
void finalizar_proceso(void* arg);
void crear_hilo (void* arg);
void envio_datos_a_FS(void* arg);
void finalizar_hilo(void* arg);


void asignar_particiones_fijas(t_asignar_memoria* datos_asignar_memoria);
void dividir_particion(Particion* particion, uint32_t tamanio_proceso);
void asignar_particiones_dinamicas(t_asignar_memoria* datos_asignar_memoria);

void consolidar_particiones_libres(int indice);
Particion* algoritmo_first_fit(uint32_t tamanio_proceso);
Particion* algoritmo_best_fit(uint32_t tamanio_proceso);
Particion* algoritmo_worst_fit(uint32_t tamanio_proceso);
Particion* evaluarParticion(int tamanio);

t_datos_esenciales* deserializar_datos_dump_memory(t_paquete* paq_dump_memory);

#endif