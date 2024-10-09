#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();

    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();
    
    //liberar los logs y config
    free(valores_config_cpu);
    return 0;
}
void ciclo_de_instruccion(){
    fetch(); //Actualizamos la instruccionActual que se  esta trabajando  check
    t_instruccion* instruccionDecodificada = decode(instruccionActual); //Verificar lectura de la instruccion
    execute(instruccionDecodificada);
    //check_interrupt();


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
bool esSysscall(char* instruccion){
    const char* syscall[]={"DUMP_MEMORY","IO","PROCESS_CREATE","THREAD_CREATE","THREAD_JOIN","THREAD_JOIN",
    "THREAH_CANCEL","MUTEX_CREATE","MUTEX_LOCK","MUTEX_UNLOCK","THREAD_EXIT","PROCESS_EXIT"};
    for (int i = 0; syscall[i]!=NULL; i++)
    {
        if(strcmp(instruccion,syscall[i],strlen(syscall[i]))==0)
        {
            return true;
        }
        
    }
    return false;
}

void DUMP_MEMORY{
    
}
void IO (Tiempo){
    
}
void PROCESS_CREATE (Archivo de instrucciones, Tamaño, Prioridad del TID 0){
    
}
void THREAD_CREATE (Archivo de instrucciones, Prioridad){
    
}
void THREAD_JOIN (TID){
    
}
void THREAD_CANCEL (TID){
    
}
void MUTEX_CREATE (Recurso){
    
}
void MUTEX_LOCK (Recurso){
    
}
void MUTEX_UNLOCK (Recurso){
    
}
void THREAD_EXIT{
    
}
void PROCESS_EXIT{
    
}


void execute_syscall(t_instruccion* instruccion, int fd_kernel) {
    // Ejemplo para la syscall PROCESS_CREATE
    if (strcmp(instruccion->operacion, "PROCESS_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando proceso con archivo %s, tamanio %d, prioridad %d",
                 instruccion->archivo, instruccion->tamanio, instruccion->prioridad);

        // Aqui se envia la informacion al Kernel mediante el fd_kernel (simulando una interrupcion)
        // Estructura o mensaje que contiene la informacion de la syscall
        t_syscall_mensaje mensaje;
        mensaje.operacion = PROCESS_CREATE;
        mensaje.archivo = instruccion->archivo;
        mensaje.tamanio = instruccion->tamanio;
        mensaje.prioridad = instruccion->prioridad;

        // Enviar mensaje al Kernel usando fd_kernel
        if (send(fd_kernel, &mensaje, sizeof(mensaje), 0) < 0) {
            log_error(cpu_logger, "Error enviando syscall a Kernel");
        }
    }
    else if (strcmp(instruccion->operacion, "IO") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);

        // Enviar mensaje de IO al Kernel
        t_syscall_mensaje mensaje;
        mensaje.operacion = IO;
        mensaje.tiempo = instruccion->tiempo;

        if (send(fd_kernel, &mensaje, sizeof(mensaje), 0) < 0) {
            log_error(cpu_logger, "Error enviando syscall IO al Kernel");
        }
    }
    /////////////////////////////////////////////
    else if (strcmp(instruccion->operacion, "THREAD_JOIN") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);

        // Enviar mensaje de IO al Kernel
        t_syscall_mensaje mensaje;
        mensaje.operacion = IO;
        mensaje.tiempo = instruccion->tiempo;

        if (send(fd_kernel, &mensaje, sizeof(mensaje), 0) < 0) {
            log_error(cpu_logger, "Error enviando syscall IO al Kernel");
        }
    }
    else if (strcmp(instruccion->operacion, "THREAD_CANCEL") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);

        // Enviar mensaje de IO al Kernel
        t_syscall_mensaje mensaje;
        mensaje.operacion = IO;
        mensaje.tiempo = instruccion->tiempo;

        if (send(fd_kernel, &mensaje, sizeof(mensaje), 0) < 0) {
            log_error(cpu_logger, "Error enviando syscall IO al Kernel");
        }
    }

}

void enviar_syscall_a_kernel(int fd_kernel, char* syscall, ...) {
    // Preparar los datos para enviar (por ejemplo, usar una estructura para empaquetar la syscall)
    va_list args;
    va_start(args, syscall);

    // Empaquetar los datos de la syscall
    syscall_request_t request;
    request.tipo = syscall;
    // Dependiendo de la syscall, llenar los argumentos (ejemplo con PROCESS_CREATE)
    if (strcmp(syscall, "PROCESS_CREATE") == 0) {
        request.archivo = va_arg(args, char);
        request.tamano = va_arg(args, char);
        request.prioridad = va_arg(args, char*);
    }

    va_end(args);

    // Enviar la syscall al Kernel
    int resultado_envio = send(fd_kernel, &request, sizeof(syscall_request_t), 0);
    if (resultado_envio == -1) {
        log_error(cpu_logger, "Error al enviar syscall al Kernel");
        exit(EXIT_FAILURE);
    }

    log_info(cpu_logger, "Syscall enviada al Kernel: %s", syscall);

    // Pausar CPU y esperar respuesta del Kernel (usualmente esperar en un socket)
    char respuesta[128];
    int bytes_recibidos = recv(fd_kernel, respuesta, 128, 0);
    if (bytes_recibidos <= 0) {
        log_error(cpu_logger, "Error al recibir respuesta del Kernel");
        exit(EXIT_FAILURE);
    }

    log_info(cpu_logger, "Respuesta del Kernel: %s", respuesta);
}