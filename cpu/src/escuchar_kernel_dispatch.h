#ifndef ESCUCHAR_KERNEL_DISPATCH_H_
#define ESCUCHAR_KERNEL_DISPATCH_H_

#include <stdint.h>  

extern uint32_t pidHilo;
extern uint32_t tidHilo;

void escuchar_kernel_dispatch();

void obtener_contexto(int fd_memoria,uint32_t pid, uint32_t tid);

#endif