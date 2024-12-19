#ifndef CICLO_DE_INSTRUCCION
#define CICLO_DE_INSTRUCCION

#include <c_conexiones.h>


void* ciclo_de_instruccion(void* arg);

void fetch(uint32_t pc, uint32_t tidHilo, uint32_t pid);
t_instruccion* decode(/*char* instruccion*/);
void execute(t_instruccion* instruccion, RegistrosCPU* registros, uint32_t* pc,uint32_t tidHilo);
void check_interrupt(int fd_kernel_interrupt, int fd_memoria, t_contextoEjecucion* contexto);

#endif