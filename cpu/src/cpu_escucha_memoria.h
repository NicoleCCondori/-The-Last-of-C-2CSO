#ifndef CPU_ESCUCHA_MEMORIA_H_
#define CPU_ESCUCHA_MEMORIA_H_

#include <utils/serializar.h>


void cpu_escucha_memoria();
void reciboContexto(t_paquete* contextoEje);
char* recibir_instruccion_de_memoria(int fd_memoria);

#endif