#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    conectar_con_FS();
    
    fd_memoria= iniciar_servidor(valores_config_memoria->puerto_escucha ,memoria_logger,"MEMORIA");
    log_info(memoria_logger, "MEMORIA lista para recibir clientes");

    conectar_cpu();

    conectar_kernel();

    //falta finalizar las conexiones

    free(valores_config_memoria);

    return 0;
}

void inicializar_memoria(){
    memoria_logger = iniciar_logger(".//memoria.log","log_MEMORIA");

    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_MEMORIA");

    configurar_memoria();

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
    pthread_create(&hilo_cpu, NULL, (void*)memoria_escucha_cpu, NULL);
    pthread_detach(hilo_cpu);
}

void conectar_kernel(){
    //Esperar conexion kernel
    log_info(memoria_logger, "Esperando kernel...");
	fd_kernel = esperar_cliente(fd_memoria, memoria_logger,"kernel");
	handshakeServer(fd_kernel);

    //se crea un hilo para escuchar mensajes de kernel // ->>>> cambiar como multihilo
    pthread_create(&hilo_kernel,NULL,(void*)memoria_escucha_kernel,NULL);
    pthread_join(hilo_kernel,NULL);
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

void memoria_escucha_kernel(){
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