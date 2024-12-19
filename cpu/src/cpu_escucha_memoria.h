#ifndef CPU_ESCUCHA_MEMORIA_H_
#define CPU_ESCUCHA_MEMORIA_H_

#include <c_conexiones.h>


void cpu_escucha_memoria();
void reciboContexto(t_paquete* paquete);
void recibir_instruccion_de_memoria(t_paquete* paquete);

#endif