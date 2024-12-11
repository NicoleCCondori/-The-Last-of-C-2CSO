#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <utils/utils.h>
//semaforos
extern sem_t sem_binario_memoria;
extern sem_t sem_mutex_cola_ready;
extern sem_t TCBaPlanificar;
extern uint32_t pid;

void crear_proceso(int tamanio_proceso, char* path, int prioridad_main);

void asignar_espacio_memoria(uint32_t pid,int tam_proceso, int prioridad, char* path_main);

TCB* iniciar_hilo(uint32_t tid, int prioridad, uint32_t pid,char* path);

void destruir_pcb(void* elemento);

void enviar_a_memoria(int fd_memoria,TCB* hilo_main);

void iniciar_proceso();

//void mensaje_finalizar_proceso(int fd_memoria,uint32_t pid);
char* informar_a_memoria_fin_proceso(int fd_memoria,uint32_t pid);
void* finalizar_proceso(PCB* proceso);

void finalizar_hilo(TCB* hilo);
void informar_a_memoria_fin_hilo(int fd_memoria, TCB* hilo);

void desbloquear_hilos_por_tid(TCB* hilo);

void planificador_de_largo_plazo();

void planificador_corto_plazo();

TCB* buscar_hilo_menorNro_prioridad();
int buscar_hilo_mayorNroPrioridad();


#endif