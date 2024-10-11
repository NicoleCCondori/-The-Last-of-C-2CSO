#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_

#include <utils/utils.h>

typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset;
    void* stream; // Payload
} t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(op_code codigo_op);
void crear_buffer(t_paquete* paquete);
void destruir_buffer_paquete(t_paquete* paquete);

void agregar_buffer_Uint32(t_buffer* buffer, uint32_t entero);
void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero);
void agregar_buffer_string(t_buffer* buffer, char *args);
//sumo
void agregar_buffer_registrosCPU(t_buffer* buffer, RegistrosCPU* registro);
//.....

uint32_t leer_buffer_Uint32(t_buffer* buffer);
uint8_t leer_buffer_Uint8(t_buffer* buffer);
char *leer_buffer_string(t_buffer* buffer);

void *serializar_paquete(t_paquete *paquete, int bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);