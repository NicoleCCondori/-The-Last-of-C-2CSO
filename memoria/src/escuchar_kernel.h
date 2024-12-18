#ifndef ESCUCHAR_KERNEL_H_
#define ESCUCHAR_KERNEL_H_

#include <m_conexiones.h>


void escuchar_kernel();

void crear_proceso(t_paquete* paquete_kernel);
void crear_hilo(t_paquete* paquete_kernel);


void envio_datos_a_FS(t_paquete* paquete_kernel);
void eliminar_paquete(t_paquete* paquete_kernel);

void asignar_memoria(t_paquete* paquete_kernel);

void finalizar_proceso(t_paquete* paquete_kernel);

void asignar_particiones_fijas(t_asignar_memoria* datos_asignar_memoria);
void dividir_particion(Particion* particion, uint32_t tamanio_proceso);
void asignar_particiones_dinamicas(t_asignar_memoria* datos_asignar_memoria);

void consolidar_particiones_libres(int indice);
Particion* algoritmo_first_fit(uint32_t tamanio_proceso);
Particion* algoritmo_best_fit(uint32_t tamanio_proceso);
Particion* algoritmo_worst_fit(uint32_t tamanio_proceso);
Particion* evaluarParticion(int tamanio);

t_datos_esenciales* deserializar_datos_dump_memory(t_paquete* paq_dump_memory);
void finalizar_hilo(t_paquete* paquete_kernel);
#endif