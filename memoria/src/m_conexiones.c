#include <m_conexiones.h>

t_log* memoria_logger;
t_log* memoria_log_obligatorios;

t_config_memoria* valores_config_memoria;

int fd_memoria;
int fd_FS;
int fd_cpu;
int fd_kernel;

pthread_t hilo_FS;
pthread_t hilo_cpu;
pthread_t hilo_kernel;
//t_list* lista_tcb;
void* memoria;

void inicializar_memoria(){
    memoria_logger = iniciar_logger(".//memoria.log","log_MEMORIA");

    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_MEMORIA");

	configurar_memoria();

	int tamanio_memoria = atoi(valores_config_memoria->tam_memoria);
	memoria = malloc(sizeof(tamanio_memoria));

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
    valores_config_memoria->particiones = config_get_array_value(config->config,"PARTICIONES");
    valores_config_memoria->log_level = config_get_string_value(valores_config_memoria->config,"LOG_LEVEL");

     printf("dsp %s \n",valores_config_memoria->puerto_escucha);

}

void inicializar_lista_tcb() {
    lista_tcb = list_create();
    if (lista_tcb == NULL) {
        log_error(memoria_logger, "Error al crear la lista de TCBs");
        exit(EXIT_FAILURE);
    }
    log_info(memoria_logger, "Lista global de TCBs inicializada correctamente");
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

void conectar_con_FS(){
    //Cliente MEMORIA a Filesystem
    fd_FS = crear_conexion(valores_config_memoria->ip_filesystem, valores_config_memoria->puerto_filesystem, "FILESYSTEM",memoria_logger);
    handshakeClient(fd_FS,3);

	//se crea un hilo para escuchar mensajes de FS
    pthread_create(&hilo_FS, NULL, (void*)memoria_escucha_FS,NULL);
    pthread_detach(hilo_FS);
}

//Conexión con multihilos
void conectar_kernel(){
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
        
		pthread_create(&hilo_kernel,NULL,(void*)escuchar_kernel,NULL);
		pthread_detach(hilo_kernel);
	}
}