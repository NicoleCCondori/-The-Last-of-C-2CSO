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
    memoria_logger = iniciar_logger(".//memoria.log","logs_memoria");
    memoria_log_obligatorios = iniciar_logger(".//memoria_logs_olbigatorios.log","logs_obligatorios_memoria");

	configurar_memoria();

	int tamanio_memoria = atoi(valores_config_memoria->tam_memoria);
	memoria = malloc(tamanio_memoria);

    conifgurar_particiones();

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
}



/*Particiones Fijas: En este esquema la lista de particiones vendrá dada por archivo de configuración 
y la misma no se podrá alterar a lo largo de la ejecución.*/

void configurar_particiones() {
    lista_particiones = list_create();

    if (strcmp(valores_config_memoria->esquema, "FIJAS") == 0) {
        size_t inicio = 0;
        for (char** particion = valores_config_memoria->particiones; *particion != NULL; ++particion) {
            Particion* nueva_particion = malloc(sizeof(Particion));
            nueva_particion->inicio = inicio;
            nueva_particion->tamanio = atoi(*particion) - 1;
            nueva_particion->libre = true;
            list_add(lista_particiones, nueva_particion);
            inicio += nueva_particion->tamanio;
        }

    
    } else if (strcmp(valores_config_memoria->esquema, "DINAMICAS") == 0) {
        Particion* particion_unica = malloc(sizeof(Particion));
        particion_unica->inicio = 0;
        particion_unica->tamanio = atoi(valores_config_memoria->tam_memoria);
        particion_unica->libre = true;
        list_add(lista_particiones, particion_unica);
    }
}

void inicializar_memoria_sistema() {
    memoria_sistema.contextos = list_create();

    for (int i = 0; i < list_size(lista_particiones); ++i) {
        Particion* particion = list_get(lista_particiones, i);
        ContextoEjecucion* contexto = malloc(sizeof(ContextoEjecucion));
        contexto->base = particion->inicio;
        contexto->limite = particion->tamanio;
        list_add(memoria_sistema.contextos, contexto);
    }
}

void liberar_memoria() {
    list_destroy_and_destroy_elements(lista_particiones, free);
    list_destroy_and_destroy_elements(memoria_sistema.contextos, free);
    config_destroy(valores_config_memoria->config);
    free(valores_config_memoria);
    log_destroy(memoria_logger);
    log_destroy(memoria_log_obligatorios);
}

void imprimir_particiones() {
    printf("Particiones:\n");
    for (int i = 0; i < list_size(lista_particiones); ++i) {
        Particion* particion = list_get(lista_particiones, i);
        printf("Particion %d: Inicio = %zu, Tamaño = %zu, Libre = %s\n",
               i, particion->inicio, particion->tamanio,
               particion->libre ? "Si" : "No");
    }
}

void imprimir_contextos() {
    printf("Contextos de Ejecución:\n");
    for (int i = 0; i < list_size(memoria_sistema.contextos); ++i) {
        ContextoEjecucion* contexto = list_get(memoria_sistema.contextos, i);
        printf("Contexto %d: Base = %zu, Límite = %zu\n",
               i, contexto->base, contexto->limite);
    }
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
        
		pthread_create(&hilo_kernel,NULL,(void*)escuchar_kernel,NULL);
		pthread_detach(hilo_kernel);
	}
}