#include <ciclo_de_instruccion.h>


void* ciclo_de_instruccion(void* arg){
    //sem_wait(&sem_syscallKernel);
    
    while(control_key == 1 ){ 
        t_contextoEjecucion* contexto = (t_contextoEjecucion*)arg;
        log_info(cpu_logger, "Fetch");
        log_info(cpu_logger, "El pid:%u  ,el tid:%u y el pc es:%u ",contexto->pid, contexto->TID, contexto->PC);
        fetch(contexto->PC, contexto->TID, contexto->pid); //Obtenemos la instruccion de memoria y la llamamos instruccion actual
        
        sem_wait(&sem_instruccion);

        t_instruccion* instruccionDecodificada = decode(/*instruccionActual*/); //INSTRUCCION ACTUAL ES LA INSTRUCCION QUE RECIBIMOS DE MEMORIA, LA CUAL DECODIFICADA Y GUARDA EN UNA NUEVA VARIABLE
        
        //Revisa si es syscall,de serlo envia la syscall a kernel, en caso de no serlo ejecuta segun sus operandos
        execute(instruccionDecodificada, contexto);

        log_info(cpu_logger, "Datos actuales pc %u, tid %u, pid %u", contexto->PC, contexto->TID, contexto->pid);

        free(instruccionDecodificada);
        
        if(control_key_interrupt==1){

            check_interrupt(contexto);
            control_key_interrupt=0;
        }

        if (control_key == 0) {
            log_info(cpu_logger, "Ciclo de instrucción detenido por syscall.");
            break;
        }
    }
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
    
    token = strtok(NULL, " ");
    instruccionDecodificada->operando3 = token ? strdup(token) : strdup("");
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

    if(strcmp(instruccionDecodificada->operacion,"THREAD_CREATE")==0)
    {
            instruccionDecodificada->es_syscall = true;
            instruccionDecodificada->archivo = instruccionDecodificada->operando1;
            instruccionDecodificada->prioridad = atoi(instruccionDecodificada->operando2);
    }
    else if(strcmp(instruccionDecodificada->operacion,"PROCESS_CREATE")==0){
        instruccionDecodificada->es_syscall=true;
        instruccionDecodificada->archivo=instruccionDecodificada->operando1;
        instruccionDecodificada->tamanio=atoi(instruccionDecodificada->operando2);
        instruccionDecodificada->prioridad=atoi(instruccionDecodificada->operando3);
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
        instruccionDecodificada->recurso = instruccionDecodificada->operando1;
    }

    else if(strcmp(instruccionDecodificada->operacion,"THREAD_JOIN")==0 ||
    strcmp(instruccionDecodificada->operacion,"TRHEAD_CANCEL")==0){
        instruccionDecodificada->es_syscall = true;
        instruccionDecodificada->tid = atoi(instruccionDecodificada->operando1);
    }
    else if(strcmp(instruccionDecodificada->operacion,"PROCESS_EXIT")==0 || strcmp(instruccionDecodificada->operacion,"THREAD_EXIT")==0 ){
        instruccionDecodificada->es_syscall=true;
    }

    free(instruccionCopia);
    //free(token);
    return instruccionDecodificada;
}

void execute(t_instruccion* instruccion, t_contextoEjecucion* contexto){
    if(instruccion->es_syscall){
        log_info(cpu_logger,"Ejecutando syscall: %s",instruccion->operacion);
        contexto->PC++;
        actualizar_contexto(fd_memoria,contexto);
        sem_wait(&sem_syscall);
        log_info(cpu_logger,"Valor semaforo en execute syscall");
        //mostrar_valor_semaforo(&sem_syscall);
        execute_syscall(instruccion,fd_kernel_dispatch);

        control_key = 0;
        return;
    }else 
    if(strcmp(instruccion->operacion,"SET")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SET - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);
        set_registro(instruccion->operando1, instruccion->operando2, contexto->RegistrosCPU);
        if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else if(strcmp(instruccion->operacion,"SUM")==0){
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUM - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);

        sum_registro(instruccion->operando1,instruccion->operando2,contexto->RegistrosCPU);
         if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else if (strcmp(instruccion->operacion,"SUB")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUB - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);
        sub_registro(instruccion->operando1,instruccion->operando2,contexto->RegistrosCPU);
         if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else if(strcmp(instruccion->operacion,"JNZ")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: JNZ - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);

        jnz_registro(instruccion->operando1,instruccion->operando2,contexto->RegistrosCPU,contexto->PC);

    }
    else if(strcmp(instruccion->operacion,"LOG")==0)
    {
        log_registro(instruccion->operando1,contexto->RegistrosCPU);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: LOG - <%s> ",contexto->TID,instruccion->operando1);
         if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else if(strcmp(instruccion->operacion,"WRITE_MEM")==0){
        write_mem(instruccion->operando1,instruccion->operando2,contexto);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: WRITE_MEM - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);
         if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else if(strcmp(instruccion->operacion,"READ_MEM")==0){
        read_mem(instruccion->operando1,instruccion->operando2,contexto);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: READ_MEM - <%s> <%s>",contexto->TID,instruccion->operando1,instruccion->operando2);
        if(strcmp(instruccion->operando2,"PC")!=0){
            contexto->PC++;
        }
    }
    else{
        log_error(cpu_logger,"INSTRUCCION DESCONOCIDA %s",instruccion->operacion);
        return;
    }
    
}

void check_interrupt(t_contextoEjecucion* contexto) {

        log_info(cpu_logger,"Interrupcion detectada desde el kernel");
        actualizar_contexto(fd_memoria,contexto);
        t_paquete* paquete=crear_paquete(CONFIRMAR_INTERRUPCION);
        log_info(cpu_logger,"Serializo PID: %u Y TID: %U ",contexto->pid,contexto->TID);
        serializar_hilo_cpu(paquete,contexto->pid,contexto->TID);
        log_info(cpu_logger,"ENVIANDO PAQUETE");
        enviar_paquete(paquete,fd_kernel_interrupt);
        log_info(cpu_logger,"PAQUETE ENVIADO");
        eliminar_paquete(paquete);
        log_info(cpu_logger,"Se elimino el paquete");
}

