#include <cpu.h>



int main(int argc, char* argv[]) {

    sem_init(&sem_syscall,0,0);

    inicializar_cpu();
    
    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();
 
    //liberar los logs config y semaforos
    //liberar semaforo
    free(valores_config_cpu);
    return 0;
}



///////////////////INSTRUCCIONES ////////////////////


/////////// System Calls //////////

//Primero compara a que Syscall corresponde y le manda el mensaje a kernel. Mensaje es serializado y empaquetado.

//SERIALIZO LAS SYSCALL

/* 
void actualizar_contexto_de_ejecucion(int fd_memoria,PCB* PCB){
    log_info(cpu_logger,"## TID: <%d> - Actualizo Contexto Ejecucion",pcb->tid);

    uint32_t cantidad_tids=list_size(PCB->tid);

    if(send(fd_memoria,&cantidad_tids,sizeof(uint32_t),0)<0){
        log_error(cpu_logger,"Error enviando la cantidad de TIDs");
        return;
    }
    
    for (int i = 0; i < cantidad_tids; i++)
    {
        uint32_t* tid=list_get(PCB->tid,i);

        if(send(fd_memoria,tid,sizeof(uint32_t),0)<0){
            log_error(cpu_logger,"Error enviando el TID %d",*tid);
        }
        else{
            log_info(cpu_logger,"TID enviado %d",*tid);
        }
    }
    uint32_t pc=PCB->pc;

    if(send(fd_memoria,&pc,sizeof(uint32_t),0)<0){
        log_error(cpu_logger,"Error enviando el PC al modulo de MEMORIA");
        else{
            log_info(cpu_logger,"PC enviado: %d",pc);
        }
    }
    

}*/