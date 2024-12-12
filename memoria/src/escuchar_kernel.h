#ifndef ESCUCHAR_KERNEL_H_
#define ESCUCHAR_KERNEL_H_

#include <m_conexiones.h>


void escuchar_kernel();

void crear_proceso(t_paquete* paquete_kernel);
void crear_hilo(t_paquete* paquete_kernel);
void envio_datos_a_FS(t_paquete* paquete_kernel);

void eliminar_paquete(t_paquete* paquete_kernel);

void asignar_memoria();
/*
particion_t* algoritmo_best_fit(uint32_t tamanio_proceso);
particion_t* algoritmo_worst_fit(uint32_t tamanio_proceso);
particion_t* algoritmo_first_fit(uint32_t tamanio_proceso);
void dividir_particion(particion_t* particion, uint32_t tamanio_proceso);
void asignar_particiones_dinamicas(uint32_t tamanio_proceso, uint32_t pid);
void asignar_particiones_fijas(uint32_t tamanio_proceso, uint32_t pid);
*/
#endif