#include <memoria.h>

int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    conectar_con_FS();
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_cpu();

    conectar_kernel();

    //liberar los logs y config

    finalizar_modulo(memoria_logger,memoria_log_obligatorios,valores_config_memoria->config);
    free(valores_config_memoria);
    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);
    close(fd_kernel);
    close(fd_cpu);


    return 0;
}

void inicializar_memoria(){
    memoria_logger = iniciar_logger(".//memoria.log","log_MEMORIA");

    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_MEMORIA");

    configurar_memoria();

	memoria = malloc(valores_config_memoria->tam_memoria);

	inicializar_lista_tcb();

}

void configurar_memoria(){
    valores_config_memoria = malloc(sizeof(t_config_memoria));
    valores_config_memoria->config = iniciar_configs("src/memoria.config");
       
    valores_config_memoria->puerto_escucha = config_get_string_value(valores_config_memoria->config,"PUERTO_ESCUCHA");
    valores_config_memoria->ip_filesystem = config_get_string_value(valores_config_memoria->config,"IP_FILESYSTEM");
    valores_config_memoria->puerto_filesystem = config_get_string_value(valores_config_memoria->config,"PUERTO_FILESYSTEM");
    valores_config_memoria->tam_memoria = config_get_string_value(valores_config_memoria->config,"TAM_MEMORIA");
    valores_config_memoria->path_instrucciones = config_get_string_value(valores_config_memoria->config,"PATH_INSTRUCCIONES");
    valores_config_memoria->retardo_respuesta = config_get_string_value(valores_config_memoria->config,"RETARDO_RESPUESTA");
    valores_config_memoria->esquema = config_get_string_value(valores_config_memoria->config,"ESQUEMA");
    valores_config_memoria->algoritmo_busqueda = config_get_string_value(valores_config_memoria->config,"ALGORITMO_BUSQUEDA");
    /**valores_config_memoria->particiones = config_get_array_value(config->config,"PARTICIONES");*/
    valores_config_memoria->log_level = config_get_string_value(valores_config_memoria->config,"LOG_LEVEL");
}

void inicializar_lista_tcb() {
    lista_tcb = list_create();
    if (lista_tcb == NULL) {
        log_error(memoria_logger, "Error al crear la lista de TCBs");
        exit(EXIT_FAILURE);
    }
    log_info(memoria_logger, "Lista global de TCBs inicializada correctamente");
}

void conectar_con_FS(){
    //Cliente MEMORIA a Filesystem
    fd_FS = crear_conexion(valores_config_memoria->ip_filesystem, valores_config_memoria->puerto_filesystem, "FILESYSTEM",memoria_logger);
    handshakeClient(fd_FS,3);

	//se crea un hilo para escuchar mensajes de FS
    pthread_create(&hilo_FS, NULL, (void*)memoria_escucha_FS,NULL);
    pthread_detach(hilo_FS);
}


void conectar_cpu(){
   
    //Esperar conexion CPU
    log_info(memoria_logger,"Esperando CPU...");
    fd_cpu = esperar_cliente(fd_memoria,memoria_logger,"CPU");
    handshakeServer(fd_cpu);

    //se crea un hilo para escuchar mensajes de CPU
    pthread_create(&hilo_cpu, NULL, (void*)escuchar_cpu, NULL);
    pthread_detach(hilo_cpu);
}

//Conexión con multihilos
void conectar_kernel(){
    //Esperar conexion kernel
    log_info(memoria_logger, "Esperando kernel...");
	while (true) //siempre está esperando
	{
		fd_kernel = esperar_cliente(fd_memoria, memoria_logger,"kernel");
		if(fd_kernel == -1){
			log_error(memoria_logger, "Error creando conexión con kernel");
		}
		log_info(memoria_logger, "Conexión exitosa con kernel");

		handshakeServer(fd_kernel);

		pthread_t hilo_kernel;
        int* fd_nueva_conexion_ptr = malloc(sizeof(int));
        *fd_nueva_conexion_ptr = fd_kernel;
		pthread_create(&hilo_kernel,NULL,(void*)escuchar_kernel,fd_nueva_conexion_ptr);
		pthread_detach(hilo_kernel);
	}
}

void memoria_escucha_cpu(){
    bool control_key=1;
   while (control_key)
	{
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op)
		{
		case MENSAJE:
			//
			break;
		case PAQUETE:
		//
			break;
		case -1:
			log_error(memoria_logger, "Desconexion con CPU");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida con CPU");
			break;
		}
	}	
} 

void memoria_escucha_kernel(void* arg){
	int fd_kernel = *(int*)arg;
    free(arg);

    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op)
		{
		case MENSAJE:
			//
			break;
		case PAQUETE:
		//
			break;
		case -1:
			log_error(memoria_logger, "Desconexion con KERNEL");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida con KERNEL");
			break;
		}
	}
	log_info(memoria_logger, "Cerrando conexión con kernel");
    close(fd_kernel);
    pthread_exit(NULL);
}

void memoria_escucha_FS(){
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_FS);
		switch (cod_op)
		{
		case MENSAJE:
			//
			break;
		case PAQUETE:
		//
			break;
		case -1:
			log_error(memoria_logger, "Desconexion con FILESYSTEM");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida con FILESYSTEM");
			break;
		}
	}
}
