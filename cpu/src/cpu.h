#ifndef CPU_H_
#define CPU_H_

#include <utils/utils.h>

char* instruccionActual;
char* operacionAux;
uint32_t programCounterCpu;


typedef struct {
    char* operacion;  // Nombre de la instrucción (SET, SUM, SUB, etc.)
    char* operando1;  // Primer operando
    char* operando2;  // Segundo operando
    bool es_syscall;  // Indica si es una syscall o no

    // Campos adicionales para syscalls
    char* archivo;     // Nombre del archivo (para syscalls que lo requieran)
    int tamanio;       // Tamaño (si aplica, por ejemplo, para CREATE o MEMORY)
    int prioridad;     // Prioridad (para operaciones como PROCESS_CREATE)
    int tiempo;        // Tiempo de IO (si es la syscall IO)
    int recurso;       // Para MUTEX_CREATE, MUTEX_LOCK, etc.
    int tid;           // Para syscalls que manejan TID, como THREAD_JOIN, THREAD_CANCEL
} t_instruccion;
// Struc para dividir la instruccion


typedef struct{
    uint32_t operacion_length;
    char* operacion;
    uint32_t archivo_length;
    char* archivo;
    int tamanio;
    int prioridad;
    int tiempo;
    int recurso;
    int tid;
}t_syscall_mensaje;
typedef struct 
{
	uint32_t base;
	uint32_t limite;
}particionMemoria;

particionMemoria parteActual;

typedef struct 
{
    t_list* tid;
    uint32_t pc;
}t_contexto;





typedef struct{
	t_config* config;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	char* log_level;
}t_config_cpu;
//Registros de la cpu


t_log* cpu_logger;
t_log* cpu_log_debug;
t_log* cpu_logs_obligatorios;

t_config_cpu* valores_config_cpu;
//File descriptors
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;
//Hilos 
pthread_t hilo_kernel_dispatch;
pthread_t hilo_kernel_interrupt;
pthread_t hilo_memoria;

sem_t sem_syscall;




void inicializar_cpu();
void configurar_cpu();

void conectar_kernel_dispatch();
void conectar_kernel_interrupt();
void conectar_memoria();

void cpu_escucha_memoria();
void escuchar_kernel_dispatch();
void escuchar_kernel_interrupt();

void ciclo_de_instruccion();
void fetch();
t_instruccion* decode(char*);
void execute(t_instruccion*);
uint32_t MMU(uint32_t);
void check_interrupt();

void inicializar_particion_de_memoria(uint32_t, uint32_t);
int enviar_pc_a_memoria(int, uint32_t);












int enviar_pc_a_memoria(int ,uint32_t );
char* recibir_instruccion_de_memoria(int);

//EXECUTE EN CPU
void set_registro(char* ,char* );
void read_mem(char* , char* );
void leer_desde_memoria(int ,uint32_t );
void write_mem(char* , char* );
void escribir_en_memoria(int ,uint32_t ,uint32_t );
void sum_registro(char* , char* );
void sub_registro(char* , char* );
void jnz_registro(char* , char* );
void log_registro(char* );
uint32_t obtenerRegistro(char* );

//Syscall
void execute_syscall(t_instruccion* , int );
char* serializar_syscall(t_syscall_mensaje* ,int* );
void enviar_sysscall_a_kernel(t_syscall_mensaje* ,int );




#endif