#include "utils.h"

#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_
typedef struct {
    uint32_t PID;
    uint32_t TID;
    int prioridad;
    char* path;
} t_crear_hilo;

typedef struct {
    uint32_t PID;
    uint32_t TID;
} t_enviar_contexto;

typedef struct {
    uint32_t TID;
    uint32_t PC;
} t_obtener_instruccion;

t_paquete* crear_paquete(op_code codigo_op);
void crear_buffer(t_paquete* paquete);
void destruir_buffer_paquete(t_paquete* paquete);

void agregar_buffer_Uint32(t_buffer* buffer, uint32_t entero);
void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero);
void agregar_buffer_string(t_buffer* buffer, char *args);

uint32_t leer_buffer_Uint32(t_buffer* buffer);
uint8_t leer_buffer_Uint8(t_buffer* buffer);
char *leer_buffer_string(t_buffer* buffer);

void *serializar_paquete(t_paquete *paquete, int bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
t_paquete* recibir_paquete(int socket_cliente);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
void *recibir_buffer(int *size, int socket_cliente);

/*AGREGADO DE MEMORIA*/
    t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete);
    t_enviar_contexto* deserializar_enviar_contexto(t_paquete* paquete);
    void serializar_enviar_contexto(t_paquete* paquete_devolver_contexto,t_ContextoEjecucion* contextoEjecucion);
    t_obtener_instruccion* deserializar_obtener_instruccion(t_paquete* paquete);
    void serializar_enviar_instruccion(t_paquete* paquete_enviar_instruccion, char* instruccion);
    void eliminar_paquete(t_paquete* paquete);

#endif