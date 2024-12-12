#ifndef ESCUCHAR_KERNEL_DISPATCH_H_
#define ESCUCHAR_KERNEL_DISPATCH_H_

#include <c_conexiones.h>

void escuchar_kernel_dispatch();

void obtener_contexto(int fd_memoria,uint32_t pid, uint32_t tid);

#endif