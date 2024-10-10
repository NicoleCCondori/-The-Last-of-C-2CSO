#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();
    inicializar_semaforo_syscall();


    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();
    while (true)
    {
        ciclo_de_instruccion();
    }
    
    
    //liberar los logs y config
    destruir_semaforo_syscall();
    free(valores_config_cpu);
    return 0;
}
void ciclo_de_instruccion(){
    //Obtenemos la insstruccion de memoria y la llamamos instruccion actual
    fetch(); 
    //INSTRUCCION ACTUAL ES LA INSTRUCCION QUE RECIBIMOS DE MEMORIA, LA CUAL DECODIFICADA Y GUARDA EN UNA NUEVA VARIABLE
    t_instruccion* instruccionDecodificada = decode(instruccionActual); 
    //Revisa si es syscall,de serlo envia la syscall a kernel, en caso de no serlo ejecuta segun sus operandos
    execute(instruccionDecodificada,fd_memoria,fd_kernel_dispatch,PCB->PC);
    check_interrupt();


}

t_instruccion* decode(char* instruccion){

    t_instruccion* instruccionDecodificada= malloc(sizeof(t_instruccion));
    instruccionDecodificada -> operacion = strtok(instruccion," ");
    instruccionDecodificada -> operando1 = strtok(NULL," ");
    instruccionDecodificada -> operando2 = strtok(NULL," ");
    //inicializo variables
    instruccionDecodificada->es_syscall=false;
    instruccionDecodificada->archivo=NULL;
    instruccionDecodificada->tamanio=0;
    instruccionDecodificada->prioridad=0;
    instruccionDecodificada->tiempo=0;
    instruccionDecodificada->recurso=0;
    instruccionDecodificada->tid=0;
    if(strcmp(instruccionDecodificada->operacion,"PROCESS_CREATE")==0 ||
    strcmp(    instruccionDecodificada->operacion,"TRHEAD_CREATE")==0)
    {
            instruccionDecodificada->es_syscall=true;
            instruccionDecodificada->archivo=strtok(NULL," ");
            instruccionDecodificada->tamanio=atoi(strtok(NULL," "));
            instruccionDecodificada->prioridad=atoi(strtok(NULL," "));
    }
    else if(strcmp(instruccionDecodificada->operacion,"IO")==0)
    {
        instruccionDecodificada->es_syscall=true;
        instruccionDecodificada->tiempo=atoi(instruccionDecodificada->operando1);
    }
    else if (strcmp(instruccionDecodificada->operacion,"MUTEX_CREATE")==0 ||
    strcmp(    instruccionDecodificada->operacion,"MUTEX_LOCK")==0 ||
    strcmp(instruccionDecodificada->operacion,"MUTEX_UNLOCK")==0){
        instruccionDecodificada->es_syscall=true;
        instruccionDecodificada->recurso=atoi(instruccionDecodificada->operando1);
    }
    else if(strcmp(instruccionDecodificada->operacion,"THREAD_JOIN")==0 ||
    strcmp(    instruccionDecodificada->operacion,"TRHEAD_CANCEL")==0){
        instruccionDecodificada->es_syscall=true;
        instruccionDecodificada->tid=atoi(instruccionDecodificada->operando1);
    }
        
    return instruccionDecodificada;
}
void inicializar_particion_de_memoria(uint32_t base,uint32_t limite){
    parteActual.base=base;
    parteActual.limite=limite;
    log_info(cpu_logger,"Particion de memoria inicializada: Base=%d , Limite=%d",base,limite);
}
uint32_t MMU(uint32_t direccion_logica){
    uint32_t direccion_fisica=parteActual.base+ direccion_logica;

    if(direccion_fisica>parteActual.base+ parteActual.limite){
        log_error(cpu_logger,"Segmentation Fault: Direccion %d fuera de los limites",direccion_logica);
        //notificar a kernel;
        exit(EXIT_FAILURE);
    }
    return direccion_fisica;
}



int enviar_pc_a_memoria(int fd_memoria,uint32_t PC){
    int flags= (send(fd_memoria,&PC,sizeof(uint32_t),0));
    if (flags==-1)
    {
        log_error(cpu_logger,"Error al enviar PC a MEMORIA");
        return -1;
    }
    return 0;
}

char* recibir_instruccion_de_memoria(int fd_memoria){
    char* instruccion=malloc(128);//Buffer para recibir instruccion, suponemos el tamanio maximo de la instruccion
    memset(instruccion,0,128);// utilizo memset para evitar datos basura

    int bytes_recibidos=recv(fd_memoria,instruccion,128,0);
    if (bytes_recibidos<=0)
    {
        log_error(cpu_logger,"Error al recibir la instruccion desde memoria");
        free(instruccion);
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"Instruccion recibida desde memoria %s",instruccion);
    return instruccion;
}

void fetch(){

    // int fd_memoria   uint32_t tid,uint32_t* PC
    log_info(cpu_logger,"## TID: %d - FETCH - Program Cunter: %d",PCB->tid,PCB->pc);
    
    enviar_pc_a_memoria(fd_memoria,PCB->pc);
    // Busca la nueva inscruccion
    log_info(cpu_logger,"## TID: <%d> - Solicito contexto Ejecucion",PCB->tid);
    instruccionActual =recibir_instruccion_de_memoria(fd_memoria);
    if(instruccionActual==NULL){
        log_error(cpu_logger,"No se pudo recibir la instruccion desde memoria");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"Instruccion recibida: %s",instruccionActual);

}




void execute(t_instruccion* instruccion,int fd_memoria,int fd_kernel,uint32_t* PC{
    if(instruccion->es_syscall){
        log_info(cpu_logger,"Ejecutando syscall: %s",instruccion->operacion);
        execute_syscall(instruccion,fd_kernel);
        return;
    }
    if(strcmp(instruccion->operacion,"SET")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SET - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);
        set_registro(instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"SUM")==0){
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUM - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);

        sum_registro(instruccion->operando1,instruccion->operando2);
    }
    else if (strcmp(instruccion->operacion,"SUB")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUB - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);
        sub_registro(instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"JNZ")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: JNZ - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);

        jnz_registro(instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"LOG")==0)
    {
        log_registro(instruccion->operando1);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: LOG - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"WRITE_MEM")==0){
        write_mem(instruccion->operando1,instruccion->operando2);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: WRITE_MEM - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"READ_MEM")==0){
        read_mem(instruccion->operando1,instruccion->operando2);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: READ_MEM - <%s> <%s>",PCB->tid,instruccion->operando1,instruccion->operando2);
    }

    else{
        log_error(cpu_logger,"INSTRUCCION DESCONOCIDA %s",instruccion->operacion);
        
    }
    if(strcmp(instruccion->operacion,"JNZ")!=0){
        (*PC)++;
    }
    

}

void check_interrupt(t_instruccion instruc, int fd_kernel,int fd_memoria){
    // Verificar el TID
    





    if (instruc->es_syscall == 1)
    {
        log_info(cpu_logger,"SE A DETECTADO UNA INTERRUPCION %s", instruc->operacion);
        execute_syscall(instruc,fd_kernel);
        //mandar señal a memoria
        t_buffer buContexExecut;
        buContexExecut.stream = NULL;
        buContexExecut.size = 0;
        buContexExecut.offset = 0;
        agregar_buffer_string(&buContexExecut,instruc->operacion);
        if(send(fd_memoria,buContexExecut.stream,buContexExecut.size,0)==-1){
            log_error(cpu_logger, "Error enviando el buffer a memoria para el Contexto de Ejecucion");
        }
        free(buContexExecut.stream);

    }else{
        log_info(cpu_logger,"NO SE A DETECTADO UNA INTERRUPCION");
        
    }
}






///////////////////INSTRUCCIONES ////////////////////

void set_registro(char* registro,char* valor){
    uint32_t aux = atoi(valor);

    uint32_t* reg=obtenerRegistro(registro);
    if(reg!=NULL){
        *reg = aux;
    }

}

void read_mem(char* datos, char* direccion){
    uint32_t* reg_datos = obtenerRegistro(datos);
    uint32_t* reg_dirreccion = obtenerRegistro(direccion);
    if(datos != NULL && dirreccion != NULL){
        uint32_t direccion_fisica = MMU(*reg_dirreccion);
        *reg_datos = leer_desde_memoria(fd_memoria,direccion_fisica)
    }
}
void leer_desde_memoria(int fd_memoria,uint32_t direccion_fisica){
    uint32_t dato;
    int resultado_envio = send(fd_memoria, &direccion_fisica, sizeof(uint32_t));
    if(resultado_envio == -1){
        log_error(cpu_logger,"Errror al enviar direccion de lectura a memoria");
        exit(EXIT_FAILURE)
    }
    int bytes_recibidos =recv(fd_memoria, &dato,sizeof(uint32_t),0);
    if (bytes_recibidos <= 0){
        log_error(cpu_logger,"Error al recibir dato desde memoria");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"## TID: <TID> - Acción: <LEER > - Dirección Física: <DIRECCION_FISICA>")
    return dato;
}

void write_mem(char* registro_direccion, char* registro_datos){
    uint32_t* reg_direccion=obtenerRegistro(registro_direccion);
    uint32_t* reg_datos=obtenerRegistro(registro_datos);
    if(reg_direccion!= NULL && reg_datos!= NULL){
        uint32_t direccion_fisica=MMU(*reg_direccion);

        escribir_en_memoria(fd_memoria,direccion_fisica,*reg_datos);

    }
}
void escribir_en_memoria(int fd_memoria,uint32_t direccion_fisica,uint32_t dato){
    uint32_t buffer[2];
    buffer[0]=direccion_fisica;
    buffer[1]=dato;

    int resultado_envio=send(fd_memoria,buffer,sizeof(buffer),0);
    if(resultado_envio==-1){
        log_error(cpu_logger,"Error al enviar datos de escritura a memoria");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"## TID: %d - Accion: Escribir - Dirrecion Fisica;%d",tid,direccion_fisica);
}

void sum_registro(char* destino, char* origen){
    uint32_t* reg_des=obtenerRegistro(destino);
    uin32_t* reg_ori=obtenerRegistro(origen);
    if(reg_des!=NULL && reg_ori!=NULL){
        *reg_des+=*reg_orig;
    }
}

void sub_registro(char* destino, char* origen){
    destino -= origen;
}

void jnz_registro(char* registro, char* instrucción){
    if(registro != 0){
        programCounterCpu = instrucción;
    }
}

void log_registro(char* registro){
    log_info(cpu_logger,);
}
// OBTENGO EL REGISTRO COMPARANDO 
uint32_t obtenerRegistro(char* registro){
    if(strcmp(registro,"AX")==0){
       return RegistrosCPU->AX 
    }
    else if (strcmp(registro,"BX")==0){
       return RegistrosCPU->BX 
    }
    else if(strcmp(registro,"CX")==0){
        return RegistrosCPU->CX 
    }
     else if(strcmp(registro,"DX")==0){
        return RegistrosCPU->DX
    }
    else if(strcmp(registro,"EX")==0){
       return  RegistrosCPU->EX 
    }
    else if(strcmp(registro,"FX")==0){
        return RegistrosCPU->FX 
    }
    else if(strcmp(registro,"GX")==0){
       return RegistrosCPU->GX
    }
    else if(strcmp(registro,"HX")==0){
        return RegistrosCPU->HX 
    }
    else{
        log_error(cpu_logger,"Registro desconocido %s",registro);
        return NULL
    }
}

/////////// System Calls //////////

//Primero compara a que Syscall corresponde y le manda el mensaje a kernel. Mensaje es serializado y empaquetado.
void execute_syscall(t_instruccion* instruccion, int fd_kernel_interrupt) {
    t_syscall_mensaje mensaje;

    if (strcmp(instruccion->operacion, "PROCESS_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando proceso con archivo %s, tamanio %d, prioridad %d",
        instruccion->archivo, instruccion->tamanio, instruccion->prioridad);

        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.archivo_length=strlen(instruccion->archivo)+1;
        mensaje.archivo = instruccion->archivo;
        mensaje.tamanio = instruccion->tamanio;
        mensaje.prioridad = instruccion->prioridad;

      enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
      sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);

    }
    else if (strcmp(instruccion->operacion, "IO") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);

        // Enviar mensaje de IO al Kernel
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion
        mensaje.tiempo = instruccion->tiempo;

        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);



    }
    else if (strcmp(instruccion->operacion, "THREAD_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando hilo con archivo %s, prioridad %d",
        instruccion->archivo, instruccion->prioridad);

        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.archivo = instruccion->archivo;
        mensaje.prioridad = instruccion->prioridad;

        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);



     
    }
    else if (strcmp(instruccion->operacion, "THREAD_JOIN") == 0) {
        log_info(cpu_logger, "Syscall: THREAD_JOIN a con tid: %d",instruccion->tid);
            

   
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.tid=instruccion->tid;

       
        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "THREAD_CANCEL") == 0) {
       log_info(cpu_logger, "Syscall: THREAD_CANCEL con tid: %d",instruccion->tid);


        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.tid=instruccion->tid;


        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);



    }
    else if (strcmp(instruccion->operacion, "MUTEX_CREATE") == 0) {
         log_info(cpu_logger, "Syscall: MUTEX_CREATE con recurso: %d",instruccion->recurso);


        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.recursoinstruccion->recurso;

        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
               
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "MUTEX_LOCK") == 0) {
        log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %d",instruccion->recurso);


      
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.recursoinstruccion->recurso;

        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "MUTEX_UNLOCK") == 0) {
    log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %d",instruccion->recurso);


      
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
        mensaje.recursoinstruccion->recurso;

        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "DUMP_MEMORY") == 0) {
    log_info(cpu_logger, "SYSCALL: DUMP_MEMORY");


      
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
 
        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "THREAD_EXIT") == 0) {
    log_info(cpu_logger, "SYSCALL: THREAD_EXIT");


      
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
 
        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }
    else if (strcmp(instruccion->operacion, "PROCESS_EXIT") == 0) {
    log_info(cpu_logger, "SYSCALL: PROCESS_EXIT");


      
        mensaje.operacion_length=strlen(instruccion->operacion)+1;
        mensaje.operacion = instruccion->operacion;
 
        enviar_syscall_a_kernel(&mensaje,fd_kernel_interrupt);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_interrupt);


    }

}
//SERIALIZO LAS SYSCALL
char* serializar_syscall(t_syscall_mensaje* mensaje,int* bytes_serializados){
   
    int size_total=sizeof(uint32_t)*2
    +sizeof(int)*5
    +mensaje->operacion_length
    +mensaje->archivo_length;
    
    char* buffer=malloc(size_total);
    int offset=0;
    

    memcpy(stream + offset,&mensaje.tamanio,sizeof(int));
    offset+=sizeof(int);

    memcpy(stream + offset,&mensaje.prioridad,sizeof(int));
    offset+=sizeof(int);

    memcpy(stream + offset,&mensaje.tiempo,sizeof(int));
    offset+=sizeof(int);

    memcpy(stream + offset,&mensaje.recurso,sizeof(int));
    offset+=sizeof(int);

    memcpy(stream + offset,&mensaje.tid,sizeof(int));
    offset+=sizeof(int);

    memcpy(stream + offset,&(mensaje->operacion_length),sizeof(uint32_t));
    buffer->offset+=sizeof(uint32_t);
    
    memcpy(stream + offset,&mensaje->operacion,mensaje->operacion_length);
    offset+=mensaje->operacion_length;

  
    memcpy(stream + offset,(&mensaje->archivo_length),sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(stream + offset,&mensaje->archivo,mensaje->archivo_length);
    offset+=mensaje.archivo_length;

    *bytes_serializados=size_total;

    return buffer;

}
//ENVIA
void enviar_sysscall_a_kernel(t_syscall_mensaje* mensaje,int fd_kernel_interrupt){
    int bytes_serializados;
    char* buffer_serializado=serializar_syscall(mensaje,&bytes_serializados);

    int bytes_enviados=send(fd_kernel_interrupt,buffer_serializado,bytes_serializados,0);

    if(bytes_eviados<0){
        log_error(cpu_logger,"Error enviando syscall al KERNEL");

    }else{
        log_info(cpu_logger,"SYSCALL enviada correctamente al kernel");
    }

    free(buffer_serializado);
}
sem_t sem_syscall;
void inicializar_semaforo_syscall(){
    sem_init(&sem_syscall,0,0)
}
void recibir_respuesta_kernel(int fd_kernel_interrupt){
    char respuesta[256];
    int bytes_recibidos=recv(fd_kernel_interrupt,respuesta,sizeof(respuesta),0)
    if (bytes_recibidos>0)
    {
        sem_post(&sem_syscall);
    }   
}
void destruir_semaforo_syscall(){
    sem_destroy(&sem_syscall);
}