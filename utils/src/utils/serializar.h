#include "utils.h"

#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_

typedef struct {
    uint32_t TID;
    RegistrosCPU* RegistrosCPU;
	uint32_t PC;
    uint32_t base;
    uint32_t limite;
} t_contextoEjecucion;

t_paquete* crear_paquete(op_code codigo_op);
void crear_buffer(t_paquete* paquete);
void destruir_buffer_paquete(t_paquete* paquete);

void agregar_buffer_Uint32(t_buffer* buffer, uint32_t entero);
void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero);
void agregar_buffer_string(t_buffer* buffer, char *args);
t_contextoEjecucion* leer_contexto_de_memoria(t_buffer* buffer);

uint32_t leer_buffer_Uint32(t_buffer* buffer);
uint8_t leer_buffer_Uint8(t_buffer* buffer);
char *leer_buffer_string(t_buffer* buffer);

void *serializar_paquete(t_paquete *paquete, int bytes);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
t_paquete* recibir_paquete(int socket_cliente);


/*AGREGADO DE MEMORIA*/
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

typedef struct{
    uint32_t TID;
    t_contextoEjecucion* contexto_ejecucion;
}t_actualizar_contexto;

//Agrega Maykel
typedef struct{
    uint32_t operacion_length;
    char* operacion;
    uint32_t archivo_length;
    char* archivo;
    int tamanio;
    int prioridad;
    int tiempo;
    uint32_t recurso_length;
    char* recurso;
    int tid;
    uint32_t TID;
    uint32_t PID;
}t_syscall_mensaje;

//=====================================
typedef struct {
    uint32_t pid_inv;
	uint32_t tid_inv;
}t_datos_esenciales;

typedef struct{
    char* nomArchP;
    int tamProceso;
    int prioridadHM;
}t_process_create;
typedef struct{
    char* nombreArchT;
    uint32_t prioridadH;
}t_thread_create;
/*typedef struct{
    int tiempo;
}t_IO;

typedef struct{
    int tid;
}t_thread_join_y_cancel;
typedef struct{
    char* recurso;
}t_mutex;*/
//=====================================

t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete);
t_enviar_contexto* deserializar_enviar_contexto(t_paquete* paquete);
void serializar_enviar_contexto(t_paquete* paquete_devolver_contexto,t_contextoEjecucion* contextoEjecucion);
t_obtener_instruccion* deserializar_obtener_instruccion(t_paquete* paquete);
void serializar_enviar_instruccion(t_paquete* paquete_enviar_instruccion, char* instruccion);
t_actualizar_contexto* deserializar_actualizar_contexto(t_paquete* paquete);
void eliminar_paquete(t_paquete* paquete);

/*AGREGADO KERNEL*/
typedef struct {
    uint32_t pid;
    int tam_proceso;
} t_asignar_memoria;

void agregar_buffer_registrosCPU(t_buffer* buffer, RegistrosCPU* registro);
void* serializar_hilo_cpu(t_paquete* hilo_cpu, uint32_t pid, uint32_t tid);
void* serializar_hilo_ready(t_paquete* paquete_hilo,TCB* hilo);
void* serializar_asignar_memoria(t_paquete* paquete_asignar_memoria, uint32_t pid, int tam_proceso);
t_asignar_memoria* deserializar_asignar_memoria(t_paquete* paquete);
RegistrosCPU* leer_buffer_registro(t_buffer* buffer);

//========================================================
t_datos_esenciales* deserializar_datos_esenciales(t_paquete* paquete);
t_process_create* deserializar_process_create(t_paquete* paquete);
int deserializar_IO(t_paquete* paquete);
t_thread_create* deserializar_thread_create(t_paquete* paquete);
uint32_t deserializar_thread_join_y_cancel(t_paquete* paquete);
void* deserializar_mutex(t_paquete* paquete);
#endif