#include <c_conexiones.h>
<<<<<<< HEAD
=======
#include <escuchar_kernel_dispatch.h>

>>>>>>> origin/checkpoint3
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

<<<<<<< HEAD
=======
//Vienen de Kernel "RECIBIR_TID"
//uint32_t pidHilo; 
//uint32_t tidHilo; //usé extern y estan definidas en escuchar_kernel_dispatch

char* instruccionActual;
particionMemoria parteActual;
sem_t sem_syscall;

>>>>>>> origin/checkpoint3
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
void conectar_memoria(){
    //Conexion como cliente a MEMORIA
    fd_memoria = crear_conexion(valores_config_cpu->ip_memoria,valores_config_cpu->puerto_memoria,"MEMORIA",cpu_logger);
    handshakeClient(fd_memoria,1);
    
    //Hilo para enviar mensajes a memoria
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escucha_memoria,NULL);
    pthread_detach(hilo_memoria);
}

<<<<<<< HEAD
=======
void ciclo_de_instruccion(t_contextoEjecucion* contexto){
    //Obtenemos la instruccion de memoria y la llamamos instruccion actual
    
    fetch(contexto->TID , contexto->PC); 
    
    //INSTRUCCION ACTUAL ES LA INSTRUCCION QUE RECIBIMOS DE MEMORIA, LA CUAL DECODIFICADA Y GUARDA EN UNA NUEVA VARIABLE
    
    t_instruccion* instruccionDecodificada = decode(instruccionActual); 
    
    //Revisa si es syscall,de serlo envia la syscall a kernel, en caso de no serlo ejecuta segun sus operandos
    
    execute(instruccionDecodificada, contexto->RegistrosCPU, &contexto->PC, contexto->TID);
    check_interrupt(fd_kernel_interrupt,fd_memoria, contexto);

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
    instruccionDecodificada->PID = pidHilo;
    instruccionDecodificada->TID = tidHilo;
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
    strcmp(    instruccionDecodificada->operacion,"TRHEAD_CANCEL")==0){
        instruccionDecodificada->es_syscall = true;
        instruccionDecodificada->tid = atoi(instruccionDecodificada->operando1);
    }
        
    return instruccionDecodificada;
}

char* recibir_instruccion_de_memoria() {
    // Recibir el paquete desde memoria
    t_paquete* paquete = recibir_paquete(fd_memoria);
    
    if (paquete == NULL) {
        log_error(cpu_logger, "Error al recibir paquete instruccion desde memoria");
        return NULL;
    }

    // Leer la instruccion del buffer del paquete
    char* instruccion = leer_buffer_string(paquete->buffer);
    
    if (instruccion == NULL) {
        log_error(cpu_logger, "Error al leer la instruccion del buffer");
        free(paquete);
        return NULL;
    }

    // Loggear la instruccion recibida
    log_info(cpu_logger, "Instruccion recibida desde memoria: %s", instruccion);

    // Liberar el paquete (ya que ya se obtuvo la instruccion)
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    
    return instruccion;
}

void fetch(uint32_t tidHilo, uint32_t pc){

    // int fd_memoria   uint32_t tid,uint32_t* PC
    log_info(cpu_logger,"## TID: %d - FETCH - Program Cunter: %d",tidHilo,pc);
    if (enviar_pc_a_memoria(pc, tidHilo)==-1)
    {
        log_error(cpu_logger,"ERROR al enviar PC a MEMORIA");
        exit(EXIT_FAILURE);
    }
    // Busca la nueva inscruccion
    instruccionActual = recibir_instruccion_de_memoria(fd_memoria);
    if(instruccionActual==NULL){
        log_error(cpu_logger,"No se pudo recibir la instruccion desde memoria");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"Instruccion recibida: %s",instruccionActual);
}

int enviar_pc_a_memoria(uint32_t PC,uint32_t TID){
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion=OBTENER_INSTRUCCION;
    paquete->buffer=malloc(sizeof(t_buffer));

    paquete->buffer->size=sizeof(uint32_t)*2;
    paquete->buffer->stream=malloc(paquete->buffer->size);
    int offset=0;

    memcpy(paquete->buffer->stream,&PC,sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(paquete->buffer->stream,&TID,sizeof(uint32_t));


    int bytes=sizeof(op_code)+sizeof(int)+paquete->buffer->size;
    void* a_enviar=malloc(bytes);
    //int offset=0;

    memcpy(a_enviar+offset,&(paquete->codigo_operacion),sizeof(op_code));
    offset+=sizeof(op_code);

    memcpy(a_enviar+offset,&(paquete->buffer->size),sizeof(int));
    offset+=sizeof(int);

     memcpy(a_enviar+offset,&(paquete->buffer->stream),paquete->buffer->size);

     int flags=send(fd_memoria,a_enviar,bytes,0);
     if(flags==-1){
        log_error(cpu_logger,"Error al enviar PC y TID a MEMORIA");
        return -1;
     }

    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);

    return 0;
}

void execute(t_instruccion* instruccion, RegistrosCPU* registros, uint32_t* pc,uint32_t tidHilo){
    if(instruccion->es_syscall){
        log_info(cpu_logger,"Ejecutando syscall: %s",instruccion->operacion);
        execute_syscall(instruccion,fd_kernel_dispatch);
       
    }
    if(strcmp(instruccion->operacion,"SET")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SET - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
        set_registro(instruccion->operando1, instruccion->operando2, registros);
    }
    else if(strcmp(instruccion->operacion,"SUM")==0){
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);

        sum_registro(instruccion->operando1,instruccion->operando2,registros);
    }
    else if (strcmp(instruccion->operacion,"SUB")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: SUB - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
        sub_registro(instruccion->operando1,instruccion->operando2,registros);
    }
    else if(strcmp(instruccion->operacion,"JNZ")==0)
    {
        log_info(cpu_logger,"## TID <%d> - Ejecutando: JNZ - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);

        jnz_registro(instruccion->operando1,instruccion->operando2,registros,pc);
    }
    else if(strcmp(instruccion->operacion,"LOG")==0)
    {
        log_registro(instruccion->operando1,registros);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: LOG - <%s> ",tidHilo,instruccion->operando1);
    }
    else if(strcmp(instruccion->operacion,"WRITE_MEM")==0){
        write_mem(instruccion->operando1,instruccion->operando2,registros,tidHilo);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: WRITE_MEM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
    }
    else if(strcmp(instruccion->operacion,"READ_MEM")==0){
        read_mem(instruccion->operando1,instruccion->operando2,registros,tidHilo);
        log_info(cpu_logger,"## TID <%d> - Ejecutando: READ_MEM - <%s> <%s>",tidHilo,instruccion->operando1,instruccion->operando2);
    }

    else{
        log_error(cpu_logger,"INSTRUCCION DESCONOCIDA %s",instruccion->operacion);
        
    }
    if(strcmp(instruccion->operacion,"JNZ")!=0){
        pc++;
    }
}

void check_interrupt(int fd_kernel_interrupt, int fd_memoria, t_contextoEjecucion* contexto) {
    int interrup_signal;  // Variable para almacenar la senial de interrupcion

    // Leer si hay interrupcion desde el CPU
    if (recv(fd_kernel_interrupt, &interrup_signal, sizeof(int), 0) > 0) {
        log_info(cpu_logger, "Interrupcion detectada desde el Kernel");

        // Actualizar el contexto de ejecucion: Registros, PC y TID
        actualizar_contexto(fd_memoria, contexto);

        // Enviar una confirmacion al Kernel si es necesario
        int respuesta = 1;  // Por ejemplo, enviar un valor que represente exito
        if (send(fd_kernel_interrupt, &respuesta, sizeof(int), 0) == -1) {
            log_error(cpu_logger, "Error al enviar la confirmacion de manejo de interrupcion al Kernel.");
        } else {
            log_info(cpu_logger, "Confirmacion de interrupcion enviada al Kernel.");
        }

    } else {
        log_info(cpu_logger, "No se ha detectado una interrupcion. Continuando ejecucion normal.");
    }
}
void actualizar_contexto(int fd_memoria, t_contextoEjecucion* contexto_ejecucion) {
    log_info(cpu_logger, "Actualizando contexto en memoria: TID = %d, PC = %d", contexto_ejecucion->TID, contexto_ejecucion->PC);

    
    t_paquete* paquete_contexto = crear_paquete(ACTUALIZAR_CONTEXTO); 

    serializar_enviar_contexto(paquete_contexto, contexto_ejecucion);

    // Enviar el paquete serializado al socket de memoria 
    enviar_paquete(paquete_contexto, fd_memoria);

    // Liberar la memoria del paquete
    eliminar_paquete(paquete_contexto);  
}
>>>>>>> origin/checkpoint3
/*
void cpu_escucha_memoria(){
    bool control_key = 1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:
		
		case PAQUETE:
		
			break;
		case -1:
			log_error(cpu_logger, "Desconexion de MEMORIA");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}
void escuchar_kernel_dispatch(){
    //atender los msjs de kernel-dispatch
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(cpu_logger, "Desconexion de KERNEL - Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL -Interrupt");
			break;
		}
	}    
}


void escuchar_kernel_interrupt(){
    //atender los msjs de kernel-interrupt 
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(cpu_logger, "Desconexion de KERNEL-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL-Interrupt");
			break;
		}
	}
}
<<<<<<< HEAD
*/
=======

void write_mem(char* registro_direccion, char* registro_datos, RegistrosCPU* registros,uint32_t tidHilo){
    uint32_t reg_direccion = obtenerRegistro(registro_direccion,registros);
    uint32_t reg_datos = obtenerRegistro(registro_datos,registros);
    if(reg_direccion != 0 && reg_datos!= 0){
        uint32_t direccion_fisica=MMU(reg_direccion);

        escribir_en_memoria(fd_memoria,direccion_fisica, reg_datos,tidHilo);

    }
}
void escribir_en_memoria(int fd_memoria, uint32_t direccion_fisica, uint32_t dato,uint32_t tidHilo){
    uint32_t buffer[2];
    buffer[0]=direccion_fisica;
    buffer[1]=dato;

    int resultado_envio=send(fd_memoria,buffer,sizeof(buffer),0);
    if(resultado_envio==-1){
        log_error(cpu_logger,"Error al enviar datos de escritura a memoria");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger,"## TID: %u - Accion: Escribir - Dirrecion Fisica;%u",tidHilo,direccion_fisica);
}

void sum_registro(char* destino, char* origen,RegistrosCPU* registros){
    uint32_t reg_des = obtenerRegistro(destino,registros);
    uint32_t reg_ori = obtenerRegistro(origen,registros);
    if(reg_des!=0 && reg_ori!=0){
        reg_des += reg_ori;
    }
}

void sub_registro(char* destino, char* origen,RegistrosCPU* registros){
	uint32_t reg_des = obtenerRegistro(destino,registros);
    uint32_t reg_ori = obtenerRegistro(origen,registros);
    reg_des -= reg_ori;
}

void jnz_registro(char* registro, char* instruccion,RegistrosCPU* registros, uint32_t *pc){
    uint32_t reg = obtenerRegistro(registro,registros);
    uint32_t aux_pc = atoi(instruccion);
    if(reg != 0){
        *pc = aux_pc;
    }
}

void log_registro(char* registro,RegistrosCPU* registros){
	uint32_t reg = obtenerRegistro(registro,registros);
    log_info(cpu_logger,"Registro: %d",reg);
}

// OBTENGO EL REGISTRO COMPARANDO 
uint32_t obtenerRegistro(char* registro, RegistrosCPU* registros){
    if(strcmp(registro,"AX")==0){

       return registros->AX;
    }
    else if (strcmp(registro,"BX")==0){
       return registros->BX;
    }
    else if(strcmp(registro,"CX")==0){
        return registros->CX;
    }
     else if(strcmp(registro,"DX")==0){
        return registros->DX;
    }
    else if(strcmp(registro,"EX")==0){
       return registros->EX;
    }
    else if(strcmp(registro,"FX")==0){
        return registros->FX; 
    }
    else if(strcmp(registro,"GX")==0){
       return registros->GX;
    }
    else if(strcmp(registro,"HX")==0){
        return registros->HX; 
    }
    else{
        log_error(cpu_logger,"Registro desconocido %s",registro);
        return 0;
    }
    return 0;
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
void execute_syscall(t_instruccion* instruccion, int fd_kernel_dispatch) {

    //t_syscall_mensaje* mensaje = malloc(sizeof(t_syscall_mensaje)) ; //debemos LIBERARLOOOO
    //lo necesitan todas las syscall
    //mensaje->PID = instruccion->PID;
    //mensaje->TID = instruccion->TID;
    //mensaje->operacion_length = strlen(instruccion->operacion) + 1;
    //mensaje->operacion = instruccion->operacion;
    
    if (strcmp(instruccion->operacion, "PROCESS_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando proceso con archivo %s, tamanio %d, prioridad %d",
        instruccion->archivo, instruccion->tamanio, instruccion->prioridad);

        //PRIMERO DEBO ASIGNARLE A CADA TIPO DE DATO LO QUE CORRESPONDE DE t_syscall_mensaje
        //mensaje->archivo = instruccion->archivo;
        //mensaje->archivo_length = strlen(instruccion->archivo) + 1;
        //mensaje->tamanio = instruccion->tamanio;
        //mensaje->prioridad = instruccion->prioridad;

        //t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_PROCESS_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->archivo,instruccion->tamanio,instruccion->prioridad);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);
    }
    else if (strcmp(instruccion->operacion, "IO") == 0) {
        log_info(cpu_logger, "Syscall: Ejecutando IO por %d segundos", instruccion->tiempo);

        // Enviar mensaje de IO al Kernel
        //mensaje->tiempo = instruccion->tiempo;

        //t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_IO(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tiempo);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);

    }
    else if (strcmp(instruccion->operacion, "THREAD_CREATE") == 0) {
        log_info(cpu_logger, "Syscall: Creando hilo con archivo %s, prioridad %d",
        instruccion->archivo, instruccion->prioridad);
        
        //mensaje->archivo_length = strlen(instruccion->archivo) + 1;
        //mensaje->archivo = instruccion->archivo;
        //mensaje->prioridad = instruccion->prioridad;

        //t_paquete* paquete = serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_THREAD_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->archivo,instruccion->prioridad);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);
     
    }
    else if (strcmp(instruccion->operacion, "THREAD_JOIN") == 0) 
    {
        log_info(cpu_logger, "Syscall: THREAD_JOIN a con tid: %d",instruccion->tid);
   
        //mensaje->tid=instruccion->tid;

		//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_THREAD_JOIN(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tid);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);

    }
    else if (strcmp(instruccion->operacion, "THREAD_CANCEL") == 0) {
       log_info(cpu_logger, "Syscall: THREAD_CANCEL con tid: %d",instruccion->tid);

        //mensaje->tid = instruccion->tid;

        //t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_THREAD_CANCEL(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->tid);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);



    }
    else if (strcmp(instruccion->operacion, "MUTEX_CREATE") == 0) {
         log_info(cpu_logger, "Syscall: MUTEX_CREATE con recurso: %s",instruccion->recurso);
        //mensaje->recurso_length = strlen(instruccion->recurso) + 1;
        //mensaje->recurso = instruccion->recurso;

        //t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_MUTEX_CREATE(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);


    }
    else if (strcmp(instruccion->operacion, "MUTEX_LOCK") == 0) {
        log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %s",instruccion->recurso);
        //mensaje->recurso_length = strlen(instruccion->recurso) + 1;
        //mensaje->recurso = instruccion->recurso;
        
      	//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_MUTEX_LOCK(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);


    }
    else if (strcmp(instruccion->operacion, "MUTEX_UNLOCK") == 0) {
    log_info(cpu_logger, "Syscall: MUTEX_LOCK con recurso: %s",instruccion->recurso);
        //mensaje->recurso_length = strlen(instruccion->recurso) + 1;
        //mensaje->recurso = instruccion->recurso;

      	//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_MUTEX_UNLOCK(fd_kernel_dispatch,instruccion->PID,instruccion->TID,instruccion->recurso);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);


    }
    else if (strcmp(instruccion->operacion, "DUMP_MEMORY") == 0) {
    log_info(cpu_logger, "SYSCALL: DUMP_MEMORY");
 
 		//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_DUMP_MEMORY(fd_kernel_dispatch,instruccion->PID,instruccion->TID);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);


    }
    else if (strcmp(instruccion->operacion, "THREAD_EXIT") == 0)
    {
        log_info(cpu_logger, "SYSCALL: THREAD_EXIT");
 
   		//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_THREAD_EXIT(fd_kernel_dispatch,instruccion->PID,instruccion->TID);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);

    }
    else if (strcmp(instruccion->operacion, "PROCESS_EXIT") == 0) 
    {
        log_info(cpu_logger, "SYSCALL: PROCESS_EXIT");
 
    	//t_paquete* paquete=serializar_syscall(mensaje);
        //paquete->codigo_operacion=PAQUETE;
        //enviar_syscall_a_kernel(paquete,fd_kernel_dispatch);
        enviar_a_kernel_PROCESS_EXIT(fd_kernel_dispatch,instruccion->PID,instruccion->TID);
        sem_wait(&sem_syscall);
        recibir_respuesta_kernel(fd_kernel_dispatch);
    }

}

//SERIALIZO LAS SYSCALL
/*t_paquete* serializar_syscall(t_syscall_mensaje* mensaje){
    
    t_buffer* buffer=malloc(sizeof(t_buffer));

    buffer->size=sizeof(uint32_t)*4
    +sizeof(int)*5
    +mensaje->operacion_length
    +mensaje->archivo_length;
    
    buffer->stream=malloc(buffer->size);
    buffer->offset=0;
    
    //tamanio
    memcpy(buffer->stream + buffer->offset,&mensaje->tamanio,sizeof(int));
    buffer->offset+=sizeof(int);
    //prioridad
     memcpy(buffer->stream + buffer->offset,&mensaje->prioridad,sizeof(int));
    buffer->offset+=sizeof(int);
    //tiempo
    memcpy(buffer->stream + buffer->offset,&mensaje->tiempo,sizeof(int));
    buffer->offset+=sizeof(int);
    //recurso
    memcpy(buffer->stream + buffer->offset,&mensaje->recurso,sizeof(int));
    buffer->offset+=sizeof(int);
    //tid
    memcpy(buffer->stream + buffer->offset,&mensaje->tid,sizeof(int));
    buffer->offset+=sizeof(int);
    //TID IDENTIFICADOR DEL PROCESO
    memcpy(buffer->stream + buffer->offset,&mensaje->TID,sizeof(uint32_t));
    buffer->offset+=sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset,&mensaje->PID,sizeof(uint32_t));
    buffer->offset+=sizeof(uint32_t);
   
   
    //operacion_lenght
    memcpy(buffer->stream + buffer->offset,&(mensaje->operacion_length),sizeof(uint32_t));
    buffer->offset+=sizeof(uint32_t);
    //operacion
    memcpy(buffer->stream + buffer->offset,&mensaje->operacion,mensaje->operacion_length);
    buffer->offset+=mensaje->operacion_length;

    //archivo
    memcpy(buffer->stream + buffer->offset,(&mensaje->archivo_length),sizeof(uint32_t));
    buffer->offset+=sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset,&mensaje->archivo,mensaje->archivo_length);
    buffer->offset+=mensaje->archivo_length;
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SYSCALL;
    paquete->buffer=buffer;

    return paquete;
}

//ENVIA
void enviar_syscall_a_kernel(t_paquete* paquete,int fd_kernel_dispatch){

    int total_size=sizeof(op_code)+ sizeof(uint32_t)+ paquete->buffer->size;

    void* a_enviar= malloc(total_size);
    int offset=0;

    memcpy(a_enviar+offset,&(paquete->codigo_operacion),sizeof(op_code));
    offset+=sizeof(op_code);

    memcpy(a_enviar+offset,&(paquete->buffer->size),sizeof(uint32_t));
    offset+=sizeof(uint32_t);

    memcpy(a_enviar+offset,&paquete->buffer->stream,paquete->buffer->size);

    send(fd_kernel_dispatch,a_enviar,total_size,0);

    free(a_enviar);
   
}*/

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
void inicializar_particion_de_memoria(uint32_t base, uint32_t limite){
    parteActual.base=base;
    parteActual.limite=limite;
    log_info(cpu_logger,"Particion de memoria inicializada: Base=%d , Limite=%d",base,limite);
}

//Creo una función que envie el mensaje
//CONSULTAR CON EL GRUPO: si creen que es mejor sacar la variable operacion ya que se sabe por medio del op_code
void serializar_datos_esenciales(t_paquete* paquete,uint32_t PID, uint32_t TID){
    agregar_buffer_Uint32(paquete->buffer, PID);
    agregar_buffer_Uint32(paquete->buffer, TID);
}

void enviar_a_kernel_PROCESS_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID, char* archivo,uint32_t tamanio,uint32_t prioridad){
    t_paquete* paquete_process_create = crear_paquete(PROCESS_CREATE);
    serializar_datos_esenciales(paquete_process_create,PID,TID);
    serializar_process_create(paquete_process_create,archivo,tamanio,prioridad);

    enviar_paquete(paquete_process_create, fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_process_create);
}
void serializar_process_create(t_paquete* paquete_process_create, char* archivo,uint32_t tamanio,uint32_t prioridad){
    agregar_buffer_string(paquete_process_create->buffer,archivo);
    agregar_buffer_Uint32(paquete_process_create->buffer,tamanio);
    agregar_buffer_Uint32(paquete_process_create->buffer,prioridad);
}

void enviar_a_kernel_IO(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,int tiempo){
    t_paquete* paquete_IO = crear_paquete(IO);
    serializar_datos_esenciales(paquete_IO,PID,TID);
    serializar_IO(paquete_IO, tiempo);

    enviar_paquete(paquete_IO,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_IO);
}
void serializar_IO(t_paquete* paquete_IO, int tiempo){
    agregar_buffer_int(paquete_IO->buffer, tiempo);
}

void enviar_a_kernel_THREAD_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* archivo,uint32_t prioridad){
    t_paquete* paquete_thread_create = crear_paquete(THREAD_CREATE);
    serializar_datos_esenciales(paquete_thread_create,PID,TID);
    serializar_thread_create(paquete_thread_create, archivo,prioridad);

    enviar_paquete(paquete_thread_create,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_thread_create);
}
void serializar_thread_create(t_paquete* paquete_thread_create,char* archivo, uint32_t prioridad){
    agregar_buffer_string(paquete_thread_create->buffer,archivo);
    agregar_buffer_Uint32(paquete_thread_create->buffer,prioridad);
}

void enviar_a_kernel_THREAD_JOIN(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid){
    t_paquete* paquete_thread_join = crear_paquete(THREAD_JOIN);
    serializar_datos_esenciales(paquete_thread_join,PID,TID);
    serializar_thread_join_y_cancel(paquete_thread_join, tid);

    enviar_paquete(paquete_thread_join,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_thread_join);
}
void serializar_thread_join_y_cancel(t_paquete* paquete_thread_join_y_cancel,uint32_t tid){
    agregar_buffer_Uint32(paquete_thread_join_y_cancel->buffer,tid);
}

void enviar_a_kernel_THREAD_CANCEL(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,uint32_t tid){
    t_paquete* paquete_thread_cancel = crear_paquete(THREAD_CANCEL);
    serializar_datos_esenciales(paquete_thread_cancel,PID,TID);
    serializar_thread_join_y_cancel(paquete_thread_cancel, tid);

    enviar_paquete(paquete_thread_cancel,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_thread_cancel);
}


void enviar_a_kernel_MUTEX_CREATE(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_create = crear_paquete(MUTEX_CREATE);
    serializar_datos_esenciales(paquete_mutex_create,PID,TID);
    serializar_mutex(paquete_mutex_create, recurso);

    enviar_paquete(paquete_mutex_create,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_mutex_create);
}
void serializar_mutex(t_paquete* paquete_mutex,char* recurso){
    agregar_buffer_string(paquete_mutex->buffer,recurso);  
}

void enviar_a_kernel_MUTEX_LOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_lock = crear_paquete(MUTEX_LOCK);
    serializar_datos_esenciales(paquete_mutex_lock,PID,TID);
    serializar_mutex(paquete_mutex_lock, recurso);

    enviar_paquete(paquete_mutex_lock,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_mutex_lock);
}

void enviar_a_kernel_MUTEX_UNLOCK(int fd_kernel_dispatch,uint32_t PID,uint32_t TID,char* recurso){
    t_paquete* paquete_mutex_unlock = crear_paquete(MUTEX_UNLOCK);
    serializar_datos_esenciales(paquete_mutex_unlock,PID,TID);
    serializar_mutex(paquete_mutex_unlock, recurso);
    enviar_paquete(paquete_mutex_unlock,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_mutex_unlock);
}

void enviar_a_kernel_DUMP_MEMORY(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_dump_memory = crear_paquete(DUMP_MEMORY);
    serializar_datos_esenciales(paquete_dump_memory,PID,TID);
    enviar_paquete(paquete_dump_memory,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_dump_memory);
}

void enviar_a_kernel_THREAD_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_thread_exit = crear_paquete(THREAD_EXIT);
    serializar_datos_esenciales(paquete_thread_exit,PID,TID);
    enviar_paquete(paquete_thread_exit,fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_thread_exit);
}


void enviar_a_kernel_PROCESS_EXIT(int fd_kernel_dispatch,uint32_t PID,uint32_t TID){
    t_paquete* paquete_process_exit = crear_paquete(PROCESS_EXIT);
    serializar_datos_esenciales(paquete_process_exit,PID,TID);
    enviar_paquete(paquete_process_exit, fd_kernel_dispatch);
    destruir_buffer_paquete(paquete_process_exit);
}




>>>>>>> origin/checkpoint3
