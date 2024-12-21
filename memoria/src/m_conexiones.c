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

void* memoria;
t_list* lista_particiones;
t_list* lista_contextos;
int tamanio_memoria;

pthread_mutex_t mutex_lista_particiones;
pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_contextos;

pthread_t hilo_FS;
pthread_t hilo_cpu;
pthread_t hilo_kernel;
void* memoria;

int tamanio_memoria;

t_list* lista_particiones;
t_list* lista_contextos;

int inicializar_memoria(){
    memoria_logger = iniciar_logger(".//memoria.log","logs_memoria");
    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_obligatorios_memoria");
	configurar_memoria();

	tamanio_memoria = atoi(valores_config_memoria->tam_memoria);
	memoria = malloc(tamanio_memoria);
    if (!memoria) {
        log_error(memoria_logger, "Error al asignar memoria principal");
        return -1;
    }

    lista_contextos = list_create();
    if (!lista_contextos) {
        log_error(memoria_logger, "Error al crear la lista de contextos de ejecución");
        return -1;
    }

    configurar_particiones();

    return 0;
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
    valores_config_memoria->particiones = config_get_array_value(valores_config_memoria->config,"PARTICIONES");
    valores_config_memoria->log_level = config_get_string_value(valores_config_memoria->config,"LOG_LEVEL");
}

void configurar_particiones() {
    lista_particiones = list_create();

    if (strcmp(valores_config_memoria->esquema, "FIJAS") == 0) {
        size_t inicio = 0;
        for (char** particion = valores_config_memoria->particiones; *particion != NULL; ++particion) {
            pthread_mutex_lock(&mutex_lista_particiones);
            Particion* nueva_particion = malloc(sizeof(Particion));
            if (!nueva_particion) {
                log_error(memoria_logger, "Error al asignar memoria para partición fija");
                pthread_mutex_unlock(&mutex_lista_particiones);
                continue;
            }
            nueva_particion->base = inicio;
            nueva_particion->limite = inicio + atoi(*particion) - 1;
            nueva_particion->tamanio = atoi(*particion);
            nueva_particion->libre = true;
            list_add(lista_particiones, nueva_particion);
            log_info(memoria_logger, "Partición fija creada: base=%u, limite=%u", 
                     nueva_particion->base, nueva_particion->limite);
            pthread_mutex_unlock(&mutex_lista_particiones);
            inicio += nueva_particion->tamanio;
        }

    } else if (strcmp(valores_config_memoria->esquema, "DINAMICAS") == 0) {
        pthread_mutex_lock(&mutex_lista_particiones);
        Particion* particion_unica = malloc(sizeof(Particion));
        if (!particion_unica) {
            log_error(memoria_logger, "Error al asignar memoria para partición dinámica");
            pthread_mutex_unlock(&mutex_lista_particiones);
            return;
        }
        particion_unica->base = 0;
        particion_unica->limite = atoi(valores_config_memoria->tam_memoria) - 1;
        particion_unica->tamanio = atoi(valores_config_memoria->tam_memoria);
        particion_unica->libre = true;
        list_add(lista_particiones, particion_unica);
        log_info(memoria_logger, "Partición fija creada: base=%u, limite=%u", 
                     particion_unica->base, particion_unica->limite);
        pthread_mutex_unlock(&mutex_lista_particiones);
    }
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
			log_error(memoria_logger, "Error creando conexion con kernel");
		}
		log_info(memoria_log_obligatorios, "## Kernel Conectado - FD del socket: %d", fd_kernel);
		handshakeServer(fd_kernel);
        
		pthread_create(&hilo_kernel,NULL,(void*)escuchar_kernel,NULL);
		pthread_detach(hilo_kernel);
	}
}

void liberar_recursos(){
    finalizar_modulo(memoria_logger,memoria_log_obligatorios,valores_config_memoria->config);
    
    //finalizar las conexiones
    if (fd_memoria >= 0) close(fd_memoria);
    if (fd_FS >= 0) close(fd_FS);
    if (fd_kernel >= 0) close(fd_kernel);
    if (fd_cpu >= 0) close(fd_cpu);
    
    if (valores_config_memoria) free(valores_config_memoria);
    if (memoria) free(memoria);
    if (lista_particiones) list_destroy_and_destroy_elements(lista_particiones, free);
    if (lista_contextos) list_destroy_and_destroy_elements(lista_contextos, free);
    
    pthread_mutex_destroy(&mutex_lista_particiones);
    pthread_mutex_destroy(&mutex_memoria);
    pthread_mutex_destroy(&mutex_contextos);

    log_info(memoria_logger, "Todos los recursos han sido liberados correctamente.");
}