#ifndef ESCUCHAR_MEMORIA_H_
#define ESCUCHAR_MEMORIA_H_

#include <f_conexiones.h>

void escuchar_memoria();

void crear_archivo();

uint32_t cantidad_bloques_libres(FILE *file_bitmap);

bool espacio_disponible(uint32_t bloques_necesarios);

uint32_t reservar_bloques(uint32_t bloques_necesarios, char *nombre_archivo);

void escribir_contenido_bloques(uint32_t primer_bloque, char* contenido, uint32_t bloques_necesarios, uint32_t tamanio, char *nombre_archivo);
#endif