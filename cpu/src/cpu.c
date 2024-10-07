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
/*
void fetch() { 
    int numInstruccionABuscar = contextoEjecucion->programCounter;
    instruccionAEjecutar = list_get(contextoEjecucion->instrucciones,numInstruccionABuscar); 
    contextoEjecucion->programCounter += 1;
}

//////////////// "MMU" //////////
uint32_t mmu(char* direccionLogica, int tamValor){
    int dirFisica;
    int dirLogica = atoi(direccionLogica);
    int tamMaxSegmento = obtenerTamanioMaxSeg();

    nroSegmento = floor(dirLogica/tamMaxSegmento);
    uint32_t desplazamiento = dirLogica % tamMaxSegmento;

    log_debug(logger, "nrosegmento: %d", nroSegmento);
    log_debug(logger, "desplazamiento: %d", desplazamiento);
    log_debug(logger, "tamvalor: %d", tamValor);

    t_segmento* segmento = (t_segmento*)list_get(contextoEjecucion->tablaDeSegmentos, nroSegmento);
    
    uint32_t base = segmento->direccionBase;
    
    if((desplazamiento + tamValor) < (segmento->tamanio)){
        dirFisica = base + desplazamiento;
        return dirFisica;
    }
    
    else{
        log_info(logger, "PID: <%d> - Error SEG_FAULT - Segmento: <%d> - Offset: <%d> - Tamaño: <%d>", contextoEjecucion->pid, nroSegmento, desplazamiento, tamValor);
        char * terminado = string_duplicate ("SEG_FAULT");
        destruirTemporizador(rafagaCPU);
        modificarMotivoDesalojo (EXIT, 1, terminado, "", "");
        enviarContextoActualizado(socketCliente);
        contextoEjecucion->programCounter = contextoEjecucion->instruccionesLength;
        free (terminado);
        return UINT32_MAX; 
    }
}
*/


void execute(t_instruccion* instruccion){
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
    

}
void inicializar_cpu(){
    cpu_logger = iniciar_logger(".//cpu.log", "log_CPU");
    
    cpu_logs_obligatorios = iniciar_logger(".//cpu_logs_obligatorios.log", "log_CPU");

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
///////////////////////////////////////

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

