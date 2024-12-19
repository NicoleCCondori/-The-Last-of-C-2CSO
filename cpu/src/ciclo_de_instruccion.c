#include <ciclo_de_instruccion.h>

void* ciclo_de_instruccion(void* arg){

    t_contextoEjecucion* contexto = (t_contextoEjecucion*)arg;
    log_info(cpu_logger, "Fetch");
    log_info(cpu_logger, "El pid:%u  ,el tid:%u y el pc es:%u ",contexto->pid, contexto->TID, contexto->PC);
    fetch(contexto->PC, contexto->TID, contexto->pid); //Obtenemos la instruccion de memoria y la llamamos instruccion actual
    
    sem_wait(&sem_instruccion);

    t_instruccion* instruccionDecodificada = decode(/*instruccionActual*/); //INSTRUCCION ACTUAL ES LA INSTRUCCION QUE RECIBIMOS DE MEMORIA, LA CUAL DECODIFICADA Y GUARDA EN UNA NUEVA VARIABLE
    
    //Revisa si es syscall,de serlo envia la syscall a kernel, en caso de no serlo ejecuta segun sus operandos
    execute(instruccionDecodificada, contexto->RegistrosCPU, &contexto->PC, contexto->TID);

    log_info(cpu_logger, "Envio ahora pc %u, tid %u, pid %u", contexto->PC, contexto->TID, contexto->pid);
    
    enviar_pc_a_memoria(contexto->PC, contexto->TID, contexto->pid);

    free(instruccionDecodificada);

    check_interrupt(fd_kernel_interrupt,fd_memoria, contexto);
    

}

void fetch(uint32_t pc, uint32_t tidHilo, uint32_t pid){
    log_info(cpu_logger,"## TID: %d - FETCH - Program Counter: %d",tidHilo,pc);
    if (enviar_pc_a_memoria(pc, tidHilo, pid)==-1)
    {
        log_error(cpu_logger,"ERROR al enviar PC a MEMORIA");
        exit(EXIT_FAILURE);
    }
}

t_instruccion* decode(/*char* instruccion*/){

    t_instruccion* instruccionDecodificada= malloc(sizeof(t_instruccion));
    if(!instruccionDecodificada){
        log_error(cpu_logger,"ERROR al asignar memoria para la instruccion decodificada");
        exit(EXIT_FAILURE);
    }
    char* instruccionCopia= strdup(instruccionActual);
    if(!instruccionCopia){
        log_error(cpu_logger,"Error al duplicar instruccion");
        free(instruccionDecodificada);
        exit(EXIT_FAILURE);
    }

    char* token = strtok(instruccionCopia, " ");
    instruccionDecodificada->operacion = token ? strdup(token) : strdup("");

    token = strtok(NULL, " ");
    instruccionDecodificada->operando1 = token ? strdup(token) : strdup("");

    token = strtok(NULL, " ");
    instruccionDecodificada->operando2 = token ? strdup(token) : strdup("");
    //inicializo variables
    instruccionDecodificada->es_syscall=false;
    instruccionDecodificada->archivo=NULL;
    instruccionDecodificada->tamanio=0;
    instruccionDecodificada->prioridad=0;
    instruccionDecodificada->tiempo=0;
    instruccionDecodificada->recurso=0;
    instruccionDecodificada->tid=0;
    instruccionDecodificada->PID = PidHilo;
    instruccionDecodificada->TID = TidHilo;

    if(strcmp(instruccionDecodificada->operacion,"PROCESS_CREATE")==0 ||
    strcmp(    instruccionDecodificada->operacion,"TRHEAD_CREATE")==0)
    {
            instruccionDecodificada->es_syscall = true;
            instruccionDecodificada->archivo = strtok(NULL," ");
            instruccionDecodificada->tamanio = atoi(strtok(NULL," "));
            instruccionDecodificada->prioridad = atoi(strtok(NULL," "));
    }

    else if(strcmp(instruccionDecodificada->operacion,"IO")==0)
    {
        instruccionDecodificada->es_syscall = true;
        instruccionDecodificada->tiempo = atoi(instruccionDecodificada->operando1);
    }

    else if (strcmp(instruccionDecodificada->operacion,"MUTEX_CREATE")==0 ||
    strcmp(instruccionDecodificada->operacion,"MUTEX_LOCK")==0 ||
    strcmp(instruccionDecodificada->operacion,"MUTEX_UNLOCK")==0){
        instruccionDecodificada->es_syscall = true;
        instruccionDecodificada->recurso = strtok(NULL," ");
    }

    else if(strcmp(instruccionDecodificada->operacion,"THREAD_JOIN")==0 ||
    strcmp(instruccionDecodificada->operacion,"TRHEAD_CANCEL")==0){
        instruccionDecodificada->es_syscall = true;
        instruccionDecodificada->tid = atoi(instruccionDecodificada->operando1);
    }

    free(instruccionCopia);
    //free(token);
    return instruccionDecodificada;
}

void execute(t_instruccion* instruccion, RegistrosCPU* registros, uint32_t* pc,uint32_t tidHilo){
    if(instruccion->es_syscall){
        log_info(cpu_logger,"Ejecutando syscall: %s",instruccion->operacion);
        execute_syscall(instruccion,fd_kernel_dispatch);
        (*pc)++;
       
    }
    if(strcmp(instruccion->operacion,"SET")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SET - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
        set_registro(instruccion->operando1, instruccion->operando2, registros);
        if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }

    }
    else if(strcmp(instruccion->operacion,"SUM")==0){
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);

        sum_registro(instruccion->operando1,instruccion->operando2,registros);
         if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }
    else if (strcmp(instruccion->operacion,"SUB")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUB - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
        sub_registro(instruccion->operando1,instruccion->operando2,registros);
         if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }
    else if(strcmp(instruccion->operacion,"JNZ")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: JNZ - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);

        jnz_registro(instruccion->operando1,instruccion->operando2,registros,pc);
         if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }
    else if(strcmp(instruccion->operacion,"LOG")==0)
    {
        log_registro(instruccion->operando1,registros);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: LOG - <%s> ",tidHilo,instruccion->operando1);
         if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }
    else if(strcmp(instruccion->operacion,"WRITE_MEM")==0){
        write_mem(instruccion->operando1,instruccion->operando2,registros,tidHilo);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: WRITE_MEM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
         if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }
    else if(strcmp(instruccion->operacion,"READ_MEM")==0){
        read_mem(instruccion->operando1,instruccion->operando2,registros,tidHilo);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: READ_MEM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
        if(strcmp(instruccion->operando2,"PC")!=0){
            (*pc)++;
        }
    }

    else{
        log_error(cpu_logger,"INSTRUCCION DESCONOCIDA %s",instruccion->operacion);
        return;
    }
    if(strcmp(instruccion->operacion,"JNZ")!=0){
        (*pc)++;
    }
}

void check_interrupt(int fd_kernel_interrupt, int fd_memoria, t_contextoEjecucion* contexto) {
    int interrup_signal;  // Variable para almacenar la senial de interrupcion

    // Leer si hay interrupcion desde el CPU
    int bytes_recibidos=recv(fd_kernel_interrupt,&interrup_signal,sizeof(int),MSG_DONTWAIT);//evitar bloqueo si es que no hay una interrupcion
    if (bytes_recibidos>0)
    {
        log_info(cpu_logger,"Interrupcion detectada desde el kernel");
        actualizar_contexto(fd_memoria,contexto);

        int respuesta=1;
        if (send(fd_kernel_interrupt,&respuesta,sizeof(int),0)==-1)
        {
            log_error(cpu_logger,"Error al enviar la confimaracion al Kernel");

        }else{
            log_info(cpu_logger,"Confirmacion de interrupcion");
        }
    }
}