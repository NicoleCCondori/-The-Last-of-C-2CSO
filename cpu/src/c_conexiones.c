#include <c_conexiones.h>
//#include <escuchar_kernel_dispatch.h>

//en kernel_dispatch.c se inicializa con los argumento pasado por el modulo de kernel 
uint32_t PidHilo = 100;
uint32_t TidHilo= 100;

t_log* cpu_logger;
t_log* cpu_log_debug;
t_log* cpu_logs_obligatorios;

t_config_cpu* valores_config_cpu;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;

pthread_t hilo_kernel_dispatch;
pthread_t hilo_kernel_interrupt;
pthread_t hilo_memoria;
pthread_mutex_t mutex_contextos2;


char* instruccionActual;
particionMemoria parteActual;
sem_t sem_syscall;
sem_t sem_syscallKernel;
sem_t sem_instruccion;
sem_t sem_contexto;

int control_key;
int control_key_interrupt;

void inicializar_cpu(){
    cpu_logger = iniciar_logger(".//cpu.log", "log_CPU");
    
    cpu_logs_obligatorios = iniciar_logger(".//cpu_logs_obligatorios.log", "log_CPU_obligatorio");
   
    configurar_cpu();
}

void configurar_cpu(){

    valores_config_cpu = malloc(sizeof(t_config_cpu));
    valores_config_cpu->config = iniciar_configs("src/cpu.config");

    valores_config_cpu->ip_memoria = config_get_string_value(valores_config_cpu->config,"IP_MEMORIA");
    valores_config_cpu->puerto_memoria = config_get_string_value (valores_config_cpu->config, "PUERTO_MEMORIA" );
    valores_config_cpu->puerto_escucha_dispatch = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_DISPATCH");
    valores_config_cpu->puerto_escucha_interrupt = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_INTERRUPT");
    valores_config_cpu->log_level = config_get_string_value(valores_config_cpu->config, "LOG_LEVEL");

    //valores_config_cpu = config;
    printf("dsp %s \n",valores_config_cpu->ip_memoria); //se ASIGNA BIEN
	//free(config);
}

void conectar_memoria(){
    //Conexion como cliente a MEMORIA
    fd_memoria = crear_conexion(valores_config_cpu->ip_memoria,valores_config_cpu->puerto_memoria,"MEMORIA",cpu_logger);
    handshakeClient(fd_memoria,1);
    
    //Hilo para enviar mensajes a memoria
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escucha_memoria,NULL);
    pthread_detach(hilo_memoria);
}

void conectar_kernel_dispatch(){
     //Servidor CPU - dispatch
    fd_cpu_dispatch = iniciar_servidor(valores_config_cpu->puerto_escucha_dispatch, cpu_logger, "CPU - Dispatch");
    
    //espera la conexion del kernel
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "Kernel - dispatch");
    handshakeServer(fd_kernel_dispatch);
    //printf("fd_kernel: %d\n", fd_kernel_dispatch);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_dispatch, NULL, (void*)escuchar_kernel_dispatch, NULL);
    pthread_detach(hilo_kernel_dispatch);
}

void conectar_kernel_interrupt(){
    //Servidor CPU - interrupt
    fd_cpu_interrupt = iniciar_servidor(valores_config_cpu->puerto_escucha_interrupt, cpu_logger, "CPU - Interrupt");

    //espera la conexion del kernel
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "Kernel - Interrupt");
    handshakeServer(fd_kernel_interrupt);
    //printf("fd_kernel: %d\n", fd_kernel_interrupt);

    //se crea un hilo para escuchar msj de Kernel - interrupt
    pthread_create(&hilo_kernel_interrupt, NULL, (void*)escuchar_kernel_interrupt, NULL);
    pthread_join(hilo_kernel_interrupt,NULL);
}

void empezar_ciclo_de_instruccion(t_contextoEjecucion* contexto){
    pthread_t hilo_ciclo_instruccion;
    pthread_create (&hilo_ciclo_instruccion, NULL, (void*)ciclo_de_instruccion, (void*)contexto);
    pthread_detach (hilo_ciclo_instruccion);
}

void inicializar_particion_de_memoria(uint32_t base, uint32_t limite){
    parteActual.base=base;
    parteActual.limite=limite;
    log_info(cpu_logger,"Particion de memoria inicializada: Base=%d , Limite=%d",base,limite);
}

int enviar_pc_a_memoria(uint32_t PC,uint32_t TID, uint32_t PID){
    t_paquete* paquete = crear_paquete(OBTENER_INSTRUCCION);
    serializar_obtener_instruccion(paquete, PC, TID, PID);
    enviar_paquete(paquete, fd_memoria);
    log_info(cpu_logger,"Enviamos(Obtener instruccion) PC:%u , TID:%u y PID:%u a memoria", PC,TID,PID);
    eliminar_paquete(paquete);

    return 0;
}

void actualizar_contexto(int fd_memoria, t_contextoEjecucion* contexto_ejecucion) {
    log_info(cpu_logger, "Actualizando contexto en memoria: TID = %d, PC = %d", contexto_ejecucion->TID, contexto_ejecucion->PC);

    t_paquete* paquete_contexto = crear_paquete(ACTUALIZAR_CONTEXTO); 

    serializar_enviar_contexto_cpu(paquete_contexto, contexto_ejecucion);
    // Enviar el paquete serializado al socket de memoria 
    enviar_paquete(paquete_contexto, fd_memoria);
    // Liberar la memoria del paquete
    eliminar_paquete(paquete_contexto);  
}

void set_registro(char* registro,char* valor, RegistrosCPU* registros){
    uint32_t aux = atoi(valor);

    uint32_t* reg = obtenerRegistro(registro, registros);
    if(reg!=NULL){
        *reg = aux;
        log_info(cpu_logger, "Registro %s actualizado a %u", registro, aux);
    }else{
        log_info(cpu_logger,"No se pudo encontrar el registro %s\n", registro);
    }

}

void read_mem(char* datos, char* direccion, t_contextoEjecucion* contexto){
    uint32_t* reg_datos = obtenerRegistro(datos,contexto->RegistrosCPU);
    uint32_t* reg_direccion = obtenerRegistro(direccion,contexto->RegistrosCPU);
    
    if(!reg_datos || !reg_direccion){
         log_error(cpu_logger,"Error: Registros no validados para lectura");
        return;
    }

    uint32_t direccion_fisica = MMU(*reg_direccion, contexto);
    *reg_datos = leer_desde_memoria(fd_memoria, direccion_fisica,contexto->TID); 
    
}


uint32_t leer_desde_memoria(int fd_memoria, uint32_t direccion_fisica, uint32_t tidHilo){ 
    uint32_t dato;
    
    t_paquete* paquete_enviar_datos_lectura = crear_paquete(READ_MEM);

    serializar_read_mem(paquete_enviar_datos_lectura, direccion_fisica,PidHilo,tidHilo); 
    enviar_paquete(paquete_enviar_datos_lectura, fd_memoria);
    eliminar_paquete(paquete_enviar_datos_lectura);

    int bytes_recibidos =recv(fd_memoria, &dato,sizeof(uint32_t),0);
    if (bytes_recibidos <= 0){
        log_error(cpu_logger,"Error al recibir dato desde memoria");
        exit(EXIT_FAILURE);
    }
    
    log_info(cpu_logger,"## TID: <%d> - Accion: <LEER > - Direccion Fisica: <%d>",tidHilo,direccion_fisica);
    return dato;
}

void write_mem(char* registro_direccion, char* registro_datos, t_contextoEjecucion* contexto){
    uint32_t* reg_direccion = obtenerRegistro(registro_direccion,contexto->RegistrosCPU);
    uint32_t* reg_datos = obtenerRegistro(registro_datos,contexto->RegistrosCPU);
     if(!reg_direccion || !reg_datos){
        log_error(cpu_logger,"Error: Registros no validados para escritura");
        return;
    }
        uint32_t direccion_fisica=MMU(*reg_direccion,contexto);
        escribir_en_memoria(fd_memoria,direccion_fisica,*reg_datos,contexto->TID);
}

void escribir_en_memoria(int fd_memoria, uint32_t direccion_fisica, uint32_t dato, uint32_t tidHilo){
    t_paquete* paquete_enviar_datos_escritura = crear_paquete(WRITE_MEM);
    log_info(cpu_logger, "dir fis %u,dato %u, tidHilo %u ",direccion_fisica, dato,tidHilo);
    serializar_write_mem(paquete_enviar_datos_escritura, direccion_fisica, dato, PidHilo,tidHilo);
    enviar_paquete(paquete_enviar_datos_escritura, fd_memoria);
    eliminar_paquete(paquete_enviar_datos_escritura);
    
    log_info(cpu_logger,"## TID: %u - Accion: Escribir - Dirrecion Fisica;%u",tidHilo,direccion_fisica);
}

void sum_registro(char* destino, char* origen,RegistrosCPU* registros){
    uint32_t* reg_des = obtenerRegistro(destino,registros);
    uint32_t* reg_ori = obtenerRegistro(origen,registros);
    if(reg_des && reg_ori){
        *reg_des += *reg_ori;
    }
}

void sub_registro(char* destino, char* origen,RegistrosCPU* registros){
	uint32_t* reg_des = obtenerRegistro(destino,registros);
    uint32_t* reg_ori = obtenerRegistro(origen,registros);
     if(reg_des && reg_ori){
        *reg_des -= *reg_ori;
    }
}

void jnz_registro(char* registro, char* instruccion,RegistrosCPU* registros, uint32_t *pc){
    uint32_t* reg = obtenerRegistro(registro,registros);
    uint32_t aux_pc = atoi(instruccion);
    if(reg!=NULL){
        *pc = aux_pc;
    }
}

void log_registro(char* registro,RegistrosCPU* registros){
	uint32_t* reg = obtenerRegistro(registro,registros);
    log_info(cpu_logger,"Registro: %d",*reg);
}

// OBTENGO EL REGISTRO COMPARANDO 
uint32_t* obtenerRegistro(char* registro, RegistrosCPU* registros){
  
    if(strcmp(registro,"AX")==0){

       return &registros->AX;
    }
    else if (strcmp(registro,"BX")==0){
       return &registros->BX;
    }
    else if(strcmp(registro,"CX")==0){
        return &registros->CX;
    }
     else if(strcmp(registro,"DX")==0){
        return &registros->DX;
    }
    else if(strcmp(registro,"EX")==0){
       return &registros->EX;
    }
    else if(strcmp(registro,"FX")==0){
        return &registros->FX; 
    }
    else if(strcmp(registro,"GX")==0){
       return &registros->GX;
    }
    else if(strcmp(registro,"HX")==0){
        return &registros->HX; 
    }
    else{
        log_error(cpu_logger,"Registro desconocido %s",registro);
        return NULL;
    }

}

uint32_t MMU(uint32_t direccion_logica,t_contextoEjecucion* contexto){
    uint32_t direccion_fisica=parteActual.base+ direccion_logica;

    if( direccion_fisica < parteActual.base || direccion_fisica > (parteActual.base + parteActual.limite)){
        log_error(cpu_logger,"Segmentation Fault: Direccion %d fuera de los limites",direccion_logica);
        actualizar_contexto(fd_memoria,contexto);
        enviar_a_kernel_PROCESS_EXIT(fd_kernel_dispatch,contexto->pid,contexto->TID);        
        //notificar a kernel;
        exit(EXIT_FAILURE);
    }
    return direccion_fisica;
}
void execute_syscall(t_instruccion* instruccion, int fd_kernel_dispatch) {   
    if (strcmp(instruccion->operacion, "PROCESS_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando proceso con archivo %s, tamanio %d, prioridad %d",
        instruccion->archivo, instruccion->tamanio, instruccion->prioridad);
        enviar_a_kernel_PROCESS_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->archivo,instruccion->tamanio,instruccion->prioridad);
    }
    else if (strcmp(instruccion->operacion, "IO") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);
        enviar_a_kernel_IO(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tiempo);
    }
    
    else if (strcmp(instruccion->operacion, "THREAD_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando hilo con archivo %s, prioridad %d",
        instruccion->archivo, instruccion->prioridad);
        enviar_a_kernel_THREAD_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->archivo,instruccion->prioridad);
    }
    
    else if (strcmp(instruccion->operacion, "THREAD_JOIN") == 0) 
    {
        log_info(cpu_logger, "Syscall: THREAD_JOIN a con tid: %d",instruccion->tid);
        enviar_a_kernel_THREAD_JOIN(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tid);
    }
    else if (strcmp(instruccion->operacion, "THREAD_CANCEL") == 0) {
       log_info(cpu_logger, "Syscall: THREAD_CANCEL con tid: %d",instruccion->tid);
        enviar_a_kernel_THREAD_CANCEL(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tid);
    }
    else if (strcmp(instruccion->operacion, "MUTEX_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: MUTEX_CREATE con recurso: %s",instruccion->recurso);
        enviar_a_kernel_MUTEX_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
        
    }
    else if (strcmp(instruccion->operacion, "MUTEX_LOCK") == 0) {
        log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %s",instruccion->recurso);
   
        enviar_a_kernel_MUTEX_LOCK(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
       
    }
    else if (strcmp(instruccion->operacion, "MUTEX_UNLOCK") == 0) {
    log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %s",instruccion->recurso);
        enviar_a_kernel_MUTEX_UNLOCK(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
 
    }
    
    else if (strcmp(instruccion->operacion, "DUMP_MEMORY") == 0) {
    log_info(cpu_logger, "SYSCALL: DUMP_MEMORY");
        enviar_a_kernel_DUMP_MEMORY(fd_kernel_dispatch,instruccion->PID,instruccion->TID);
    }

    else if (strcmp(instruccion->operacion, "THREAD_EXIT") == 0)
    {
        log_info(cpu_logger, "SYSCALL: THREAD_EXIT");
        enviar_a_kernel_THREAD_EXIT(fd_kernel_dispatch,instruccion->PID,instruccion->TID);

    }
    else if (strcmp(instruccion->operacion, "PROCESS_EXIT") == 0) 
    {
        log_info(cpu_logger, "SYSCALL: PROCESS_EXIT");
        enviar_a_kernel_PROCESS_EXIT(fd_kernel_dispatch,instruccion->PID,instruccion->TID);
    
    }

}

void recibir_respuesta_kernel(int fd_kernel_interrupt){
    char respuesta[256];
    int bytes_recibidos=recv(fd_kernel_interrupt,respuesta,sizeof(respuesta),0);
    if (bytes_recibidos>0)
    {
        sem_post(&sem_syscall);
    }   
}
void destruir_semaforo_syscall(){
    sem_destroy(&sem_syscall);
}

//Creo una función que envie el mensaje
//CONSULTAR CON EL GRUPO: si creen que es mejor sacar la variable operacion ya que se sabe por medio del op_code
void serializar_datos_esenciales(t_paquete* paquete,uint32_t PID, uint32_t TID){
    agregar_buffer_Uint32(paquete->buffer, PID);
    agregar_buffer_Uint32(paquete->buffer, TID);
}

void enviar_a_kernel_PROCESS_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID, char* archivo,uint32_t tamanio,uint32_t prioridad){
    t_paquete* paquete_process_create = crear_paquete(PROCESS_CREATE);
    serializar_process_create(paquete_process_create,PID, TID, archivo,tamanio,prioridad);
    enviar_paquete(paquete_process_create, fd_kernel_dispatch);
    eliminar_paquete(paquete_process_create);
}

void serializar_process_create(t_paquete* paquete_process_create, uint32_t PID, uint32_t TID, char* archivo,uint32_t tamanio,uint32_t prioridad){
    agregar_buffer_Uint32(paquete_process_create->buffer,PID);
    agregar_buffer_Uint32(paquete_process_create->buffer,TID);
    agregar_buffer_string(paquete_process_create->buffer,archivo);
    agregar_buffer_Uint32(paquete_process_create->buffer,tamanio);
    agregar_buffer_Uint32(paquete_process_create->buffer,prioridad);
}

void enviar_a_kernel_IO(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,int tiempo){
    t_paquete* paquete_IO = crear_paquete(IO);
    serializar_IO(paquete_IO, PID, TID, tiempo);
    enviar_paquete(paquete_IO,fd_kernel_dispatch);
    eliminar_paquete(paquete_IO);
}

void serializar_IO(t_paquete* paquete_IO, uint32_t PID, uint32_t TID, int tiempo) {
    size_t total_size = sizeof(uint32_t) * 2 + sizeof(int);

    // Asignar memoria al buffer
    paquete_IO->buffer = malloc(sizeof(t_buffer));
    if (!paquete_IO->buffer) {
        perror("Error al asignar memoria para el buffer");
        exit(EXIT_FAILURE);
    }

    paquete_IO->buffer->size = total_size;
    paquete_IO->buffer->stream = malloc(total_size);
    if (!paquete_IO->buffer->stream) {
        perror("Error al asignar memoria para el stream");
        exit(EXIT_FAILURE);
    }

    // Serializar los datos en el buffer
    size_t offset = 0;

    // Serializar PID
    memcpy(paquete_IO->buffer->stream + offset, &PID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serializar TID
    memcpy(paquete_IO->buffer->stream + offset, &TID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serializar tiempo
    memcpy(paquete_IO->buffer->stream + offset, &tiempo, sizeof(int));
}

void enviar_a_kernel_THREAD_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* archivo,uint32_t prioridad){
    t_paquete* paquete_thread_create = crear_paquete(THREAD_CREATE);
    
    serializar_thread_create(paquete_thread_create,PID, TID, archivo, prioridad);
    
    log_info(cpu_logger,"Se esta serializando el pid:%u, el tid:%u, prioridad:%u y path:%s", PID,TID,prioridad,archivo);
    if(paquete_thread_create->buffer == NULL){
        log_info(cpu_logger, "EL BUFFER ESTA VACIO!");
    }
    else log_info(cpu_logger, "EL BUFFER NO ESTA VACIO!");
    enviar_paquete(paquete_thread_create, fd_kernel_dispatch);
    log_info(cpu_logger,"Se envia el paquete a kernel (thread_create)");
    eliminar_paquete(paquete_thread_create);
    log_info(cpu_logger,"Se elimina el paquete");
}

void serializar_thread_create(t_paquete* paquete_thread_create, uint32_t PID, uint32_t TID, char* archivo, uint32_t prioridad) {
    size_t size_archivo = strlen(archivo) + 1;
    size_t total_size = sizeof(uint32_t) * 3 + sizeof(size_t) + size_archivo; // Agregar el tamaño del archivo

    // Asignar memoria al buffer
    paquete_thread_create->buffer = malloc(sizeof(t_buffer));
    if (!paquete_thread_create->buffer) {
        perror("Error al asignar memoria para el buffer");
        exit(EXIT_FAILURE);
    }

    paquete_thread_create->buffer->size = total_size;
    paquete_thread_create->buffer->stream = malloc(total_size);
    if (!paquete_thread_create->buffer->stream) {
        perror("Error al asignar memoria para el stream");
        exit(EXIT_FAILURE);
    }

    // Serializar los datos en el buffer
    size_t offset = 0;

    // Serializar PID
    memcpy(paquete_thread_create->buffer->stream + offset, &PID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serializar TID
    memcpy(paquete_thread_create->buffer->stream + offset, &TID, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Serializar tamaño del archivo
    memcpy(paquete_thread_create->buffer->stream + offset, &size_archivo, sizeof(size_t));
    offset += sizeof(size_t);

    // Serializar archivo
    memcpy(paquete_thread_create->buffer->stream + offset, archivo, size_archivo);
    offset += size_archivo;

    // Serializar prioridad
    memcpy(paquete_thread_create->buffer->stream + offset, &prioridad, sizeof(uint32_t));
}

void enviar_a_kernel_THREAD_JOIN(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid){
    t_paquete* paquete_thread_join = crear_paquete(THREAD_JOIN);
    serializar_thread_join_y_cancel(paquete_thread_join,PID,TID, tid);
    enviar_paquete(paquete_thread_join,fd_kernel_dispatch);
    eliminar_paquete(paquete_thread_join);
}

void serializar_thread_join_y_cancel(t_paquete* paquete_thread_join_y_cancel,uint32_t PID,uint32_t TID,uint32_t tid){
    agregar_buffer_Uint32(paquete_thread_join_y_cancel->buffer,PID);
    agregar_buffer_Uint32(paquete_thread_join_y_cancel->buffer,TID);
    agregar_buffer_Uint32(paquete_thread_join_y_cancel->buffer,tid);
}

void enviar_a_kernel_THREAD_CANCEL(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid){
    t_paquete* paquete_thread_cancel = crear_paquete(THREAD_CANCEL);
    serializar_thread_join_y_cancel(paquete_thread_cancel,PID,TID, tid);
    enviar_paquete(paquete_thread_cancel,fd_kernel_dispatch);
    eliminar_paquete(paquete_thread_cancel);
}

void enviar_a_kernel_MUTEX_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_create = crear_paquete(MUTEX_CREATE);
    serializar_mutex(paquete_mutex_create, PID, TID, recurso);
    enviar_paquete(paquete_mutex_create,fd_kernel_dispatch);
    eliminar_paquete(paquete_mutex_create);
}

void serializar_mutex(t_paquete* paquete_mutex, uint32_t PID, uint32_t TID, char* recurso){
    agregar_buffer_Uint32(paquete_mutex->buffer,PID);
    agregar_buffer_Uint32(paquete_mutex->buffer,TID);
    agregar_buffer_string(paquete_mutex->buffer,recurso);  
}

void enviar_a_kernel_MUTEX_LOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_lock = crear_paquete(MUTEX_LOCK);
    serializar_mutex(paquete_mutex_lock, PID, TID, recurso);
    enviar_paquete(paquete_mutex_lock,fd_kernel_dispatch);
    eliminar_paquete(paquete_mutex_lock);
}

void enviar_a_kernel_MUTEX_UNLOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_unlock = crear_paquete(MUTEX_UNLOCK);
    serializar_mutex(paquete_mutex_unlock, PID,TID, recurso);
    enviar_paquete(paquete_mutex_unlock,fd_kernel_dispatch);
    eliminar_paquete(paquete_mutex_unlock);
}

void enviar_a_kernel_DUMP_MEMORY(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_dump_memory = crear_paquete(DUMP_MEMORY);
    serializar_hilo_cpu(paquete_dump_memory,PID,TID);
    enviar_paquete(paquete_dump_memory,fd_kernel_dispatch);
    eliminar_paquete(paquete_dump_memory);
}

void enviar_a_kernel_THREAD_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_thread_exit = crear_paquete(THREAD_EXIT);
    log_info(cpu_logger,"LE ENVIO PID:%u y TID:%u en THREAD_EXIT ",PID,TID);
    serializar_hilo_cpu(paquete_thread_exit,PID,TID);
    enviar_paquete(paquete_thread_exit,fd_kernel_dispatch);
    eliminar_paquete(paquete_thread_exit);
}


void enviar_a_kernel_PROCESS_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_process_exit = crear_paquete(PROCESS_EXIT);
    log_info(cpu_logger,"LE ENVIO PID:%u y TID:%u en PROCESS EXIT",PID,TID);
    serializar_hilo_cpu(paquete_process_exit,PID,TID);
    log_info(cpu_logger,"envio paquete");
    enviar_paquete(paquete_process_exit, fd_kernel_dispatch);
    eliminar_paquete(paquete_process_exit);
}




