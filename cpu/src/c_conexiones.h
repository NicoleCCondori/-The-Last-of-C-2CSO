#ifndef C_CONEXIONES_H_
#define C_CONEXIONES_H_

#include <utils/serializar.h>

//tip y pid del proceso que se esta ejecutando
extern uint32_t PidHilo;
extern uint32_t TidHilo;

//semaforos
extern sem_t sem_syscall;
extern pthread_mutex_t mutex_contextos2;

extern char* instruccionActual;
//char* operacionAux;
//uint32_t programCounterCpu;

//estructuras
typedef struct{
	t_config* config;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	char* log_level;
}t_config_cpu;

typedef struct {
    char* operacion;  // Nombre de la instruccion (SET, SUM, SUB, etc.)
    char* operando1;  // Primer operando
    char* operando2;  // Segundo operando
    bool es_syscall;  // Indica si es una syscall o no

    // Campos adicionales para syscalls
    char* archivo;     // Nombre del archivo (para syscalls que lo requieran)
    uint32_t tamanio;       // Tamanio (si aplica, por ejemplo, para CREATE o MEMORY)
    uint32_t prioridad;     // Prioridad (para operaciones como PROCESS_CREATE)
    int tiempo;        // Tiempo de IO (si es la syscall IO)
    char* recurso;       // Para MUTEX_CREATE, MUTEX_LOCK, etc.
    uint32_t tid;
    //Son esenciales para todas las syscall
    uint32_t PID;
    uint32_t TID;         // Para syscalls que manejan TID, como THREAD_JOIN, THREAD_CANCEL
} t_instruccion;

typedef struct 
{
	uint32_t base;
	uint32_t limite;
}particionMemoria;

extern particionMemoria parteActual;

extern t_log* cpu_logger;
extern t_log* cpu_log_debug;
extern t_log* cpu_logs_obligatorios;

extern t_config_cpu* valores_config_cpu;

//File descriptors
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

//Hilos 
extern pthread_t hilo_kernel_dispatch;
extern pthread_t hilo_kernel_interrupt;
extern pthread_t hilo_memoria;


void inicializar_cpu();
void configurar_cpu();

void conectar_kernel_dispatch();
void conectar_kernel_interrupt();
void conectar_memoria();

void cpu_escucha_memoria();
void escuchar_kernel_dispatch();
void escuchar_kernel_interrupt();

//CICLO DE INSTRUCCIONES
void ciclo_de_instruccion(t_contextoEjecucion* contexto);
void fetch(uint32_t tidHilo, uint32_t pc);
t_instruccion* decode(char* instruccion);
void execute(t_instruccion* instruccion, RegistrosCPU* registros,uint32_t *pc, uint32_t tid);
void execute_syscall(t_instruccion* instruccion, int fd_kernel_dispatch);

//INTERRUPCIONES
void check_interrupt(int fd_kernel_interrupt, int fd_memoria, t_contextoEjecucion* contexto);
void actualizar_contexto(int fd_memoria, t_contextoEjecucion* contexto_ejecucion);


//PETICIONES A MEMORIA
void inicializar_particion_de_memoria(uint32_t base, uint32_t limite);
//int enviar_pc_a_memoria(int fd_memoria,uint32_t PC,uint32_t TID);
void actualizar_contexto_de_ejecucion(int fd_memoria,PCB* PCB);
uint32_t MMU(uint32_t direccion_logica);
int enviar_pc_a_memoria(uint32_t PC,uint32_t TID);
char* recibir_instruccion_de_memoria();

//PETICIONES A KERNEL
void recibir_respuesta_kernel(int fd_kernel_interrupt);
//void enviar_syscall_a_kernel(t_paquete* paquete,int fd_kernel_dispatch);

void enviar_a_kernel_PROCESS_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID, char* archivo,uint32_t tamanio,uint32_t prioridad);
void enviar_a_kernel_IO(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,int tiempo);
void enviar_a_kernel_THREAD_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* archivo,uint32_t prioridad);
void enviar_a_kernel_THREAD_JOIN(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid);
void enviar_a_kernel_THREAD_CANCEL(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid);
void enviar_a_kernel_MUTEX_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso);
void enviar_a_kernel_MUTEX_LOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso);
void enviar_a_kernel_MUTEX_UNLOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso);
void enviar_a_kernel_DUMP_MEMORY(int fd_kernel_dispatch,uint32_t PID,uint32_t TID);
void enviar_a_kernel_THREAD_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID);
void enviar_a_kernel_PROCESS_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID);

void serializar_datos_esenciales(t_paquete* paquete,uint32_t PID, uint32_t TID);
void serializar_process_create(t_paquete* paquete_process_create, char* archivo,uint32_t tamanio,uint32_t prioridad);
void serializar_IO(t_paquete* paquete_IO, int tiempo);
void serializar_thread_create(t_paquete* paquete_thread_create,char* archivo, uint32_t prioridad);
void serializar_thread_join_y_cancel(t_paquete* paquete_thread_join_y_cancel,uint32_t tid);
void serializar_mutex(t_paquete* paquete_mutex,char* recurso);


//SEREALIZACION
//t_paquete* serializar_syscall(t_syscall_mensaje* mensaje);


//REGISTROS
uint32_t* obtenerRegistro(char* registro,RegistrosCPU* registros);

void log_registro(char* registro, RegistrosCPU* registros);

//ATENDER INSTRUCCIONES
void jnz_registro(char* registro, char* instruccion,RegistrosCPU* registros,uint32_t* pc);

void sub_registro(char* destino, char* origen,RegistrosCPU* registros);

void sum_registro(char* destino, char* origen,RegistrosCPU* registros);

void escribir_en_memoria(int fd_memoria, uint32_t  direccion_fisica, uint32_t dato,uint32_t tid);

void write_mem(char* registro_direccion, char* registro_datos,RegistrosCPU* registros,uint32_t tid);

uint32_t leer_desde_memoria(int fd_memoria,uint32_t direccion_fisica,uint32_t tid);

void read_mem(char* datos, char* direccion,RegistrosCPU* registros,uint32_t tid);

void set_registro(char* registro,char* valor,RegistrosCPU* registros);



#endif