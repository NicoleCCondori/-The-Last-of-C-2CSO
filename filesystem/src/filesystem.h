#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <f_conexiones.h>

void inicializar_bitmap();
void inicializar_bloques();
void inicializar_metadata(uint32_t size, uint32_t index_block,char* nombre_archivo);
#endif