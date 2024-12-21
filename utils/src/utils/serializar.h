#include "utils.h"

#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_

typedef struct {
    uint32_t TID;
    RegistrosCPU* RegistrosCPU;
	uint32_t PC;
    uint32_t base;
    uint32_t limite;
    uint32_t pid;
} t_contextoEjecucion;

t_paquete* crear_paquete(op_code codigo_op);
void crear_buffer(t_paquete* paquete);
void destruir_buffer_paquete(t_paquete* paquete);

void agregar_buffer_Uint32(t_buffer* buffer, uint32_t entero);
void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero);
void agregar_buffer_string(t_buffer* buffer, char *args);
t_contextoEjecucion* deserializar_enviar_contexto_cpu(t_paquete* paquete);

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
    uint32_t PC;
    uint32_t TID;
    uint32_t PID;
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
    uint32_t PID;
    uint32_t TID;
    char* nomArchP;
    uint32_t tamProceso;
    uint32_t prioridadHM;
}t_process_create;

typedef struct{
    uint32_t PID;
    uint32_t TID;
    char* nombreArchT;
    uint32_t prioridadH;
}t_thread_create;

typedef struct{
    uint32_t PID;
    uint32_t TID;
    int tiempo;
}t_IO;

typedef struct{
    uint32_t PID;
    uint32_t TID;
    uint32_t tid; //victima a bloquear
}t_thread_join_y_cancel;


typedef struct{
    uint32_t PID;
    uint32_t TID;
    char* recurso;
}t_mutex_todos;

//Estructura para despues que ingresemos a tipo de mutex
typedef struct{
    char* recurso;//recurso
    uint32_t tid;
    t_queue* bloqueados_mutex; // va a tener los hilos,es decir solo el uint32_t tid que realizaron el mutex_lock y no pueden usar el recurso
    //sem_t contador;
}t_mutex;

//=====================================

t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete);
t_enviar_contexto* deserializar_enviar_contexto(t_paquete* paquete);
void serializar_enviar_contexto_cpu(t_paquete* paquete_devolver_contexto,t_contextoEjecucion* contextoEjecucion);
t_obtener_instruccion* deserializar_obtener_instruccion(t_paquete* paquete);
void serializar_enviar_instruccion(t_paquete* paquete_enviar_instruccion, char* instruccion);
t_actualizar_contexto* deserializar_actualizar_contexto(t_paquete* paquete);
void eliminar_paquete(t_paquete* paquete);

/*AGREGADO KERNEL*/
typedef struct {
    uint32_t pid;
    int tam_proceso;
} t_asignar_memoria;

typedef struct {
    char* nombre_archivo;
    uint32_t tamanio;
    char* contenido;
}t_crear_archivo_memoria;

typedef struct{
    uint32_t dir_fis; 
    uint32_t valor;
    uint32_t pidHilo;
    uint32_t tidHilo;
}t_datos_write_mem;

typedef struct{
    uint32_t dir_fis;
    uint32_t pidHilo;
    uint32_t tidHilo;
}t_datos_read_mem;

typedef struct{
    uint32_t pid;
    uint32_t bit_confirmacion;
}t_asigno_memoria;

typedef struct{
    uint32_t pid;
    uint32_t tid;
    uint32_t bit_confirmacion;
}t_creacion_hilo;



void agregar_buffer_registrosCPU(t_buffer* buffer, RegistrosCPU* registro);
void serializar_hilo_cpu(t_paquete* hilo_cpu, uint32_t pid, uint32_t tid);
void serializar_hilo_ready(t_paquete* paquete_hilo, uint32_t pid, uint32_t tid, int prioridad, const char* path);
void* serializar_asignar_memoria(t_paquete* paquete_asignar_memoria, uint32_t pid, int tam_proceso);
t_asignar_memoria* deserializar_asignar_memoria(t_paquete* paquete);
RegistrosCPU* leer_buffer_registro(t_buffer* buffer);

//========================================================
t_datos_esenciales* deserializar_datos_esenciales(t_paquete* paquete);
t_crear_archivo_memoria* deserializar_crear_archivo_memoria(t_paquete* paquete);
t_process_create* deserializar_process_create(t_paquete* paquete);
t_IO* deserializar_IO(t_paquete* paquete);
t_thread_create* deserializar_thread_create(t_paquete* paquete);
t_thread_join_y_cancel* deserializar_thread_join_y_cancel(t_paquete* paquete);
t_mutex_todos* deserializar_mutex(t_paquete* paquete);

//==========================================================
t_datos_esenciales* deserializar_finalizar_hilo(t_paquete* paquete_memoria);
//=====================
void enviar_mensaje(char *mensaje, int socket);
t_datos_write_mem* deserializar_write_mem(t_paquete* paquete);
void* serializar_write_mem(t_paquete* paquete_write_mem, uint32_t dir_fis, uint32_t valor,uint32_t PidHilo, uint32_t TidHilo);
void* serializar_read_mem(t_paquete* paquete_enviar_datos_lectura, uint32_t direccion_fisica, uint32_t PidHilo, uint32_t TidHilo);
t_datos_read_mem* deserializar_read_mem(t_paquete* paquete);
void* serializar_enviar_DUMP_MEMORY(t_paquete* paquete_dump_memory, void* datos,uint32_t tamanio,char* nombre);

void* serializar_obtener_contexto(t_paquete* paquete_obtener_contexto, uint32_t pid, uint32_t tidHilo);
int deserializar_int(t_paquete* paquete);
void serializar_int(t_paquete* paquete, int numero);
void serializar_proceso_memoria(t_paquete* paquete, uint32_t pid, uint32_t bit_confirmacion);
t_asigno_memoria* deserializar_proceso_memoria(t_paquete* paquete);

void serializar_hilo_memoria(t_paquete* paquete, uint32_t bit_confirmacion,uint32_t pid,uint32_t tid);
t_creacion_hilo*  deserializar_creacion_hilo_memoria(t_paquete* paquete_kernel_hilo);
char* deserializar_enviar_instruccion(t_paquete* paquete);
void serializar_obtener_instruccion(t_paquete* paquete, uint32_t pc, uint32_t tid, uint32_t pid);

#endif