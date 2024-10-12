#include <utils/utils.h>

t_list* lista_tcb;

PCB* buscar_proceso(t_list* lista, uint32_t pid){
	for(int i=0; i<list_size(lista);i++)
	{
		PCB* proceso_actual = list_get(lista, i);
		if (proceso_actual->pid == pid) {
            return proceso_actual;
        }

	}
	return NULL;
}

//establece una conexión TCP con un servidor dado su dirección IP y puerto
int crear_conexion(char *ip, char *puerto, char *name_server,t_log* logger)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

    //
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    //Obtiene la información sobre el servidor y la almacena en server_info. 
	int resultado = getaddrinfo(ip, puerto, &hints, &server_info);
	if (resultado != 0)
	{
		log_error(logger, "Error en getaddrinfo: %s", gai_strerror(resultado));
		//printf("Error en getaddrinfo: %s", gai_strerror(resultado));
		exit(-1);
	}

	// Creamos un socket
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Conectamos el socket
	int connect_resultado = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	if (connect_resultado == 0)
	{
		log_info(logger, "El cliente se conecto al servidor correctamente a %s.\n", name_server);
		//printf("El cliente se conecto al servidor correctamente a %s.\n", name_server);

	}
	else
	{
		log_info(logger, "Error al conectar servidor %s\n", name_server);
		//printf("Error al conectar servidor %s\n", name_server);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void handshakeClient(int fd_servidor, int32_t handshake)
{
	int result;

	send(fd_servidor, &handshake, sizeof(int32_t), 0); //tira error cuando conectamos kernel ---> falta liberar sockets?
	recv(fd_servidor, &result, sizeof(int32_t), 0);

	if (result == 0)
		printf("¡Handshake realizado con exito!\n");
	else
		printf("Error Handshake\n");
}

//configura y pone en marcha un servidor TCP
int iniciar_servidor(char *puerto, t_log *logger, char *msj_server)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int resultado = getaddrinfo(NULL, puerto, &hints, &servinfo); //fijar si es necesario pasar por parametro la ip
	if (resultado != 0)
	{
		log_error(logger, "Error en getaddrinfo: %s.\n", gai_strerror(resultado));
		exit(-1);
	}

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	int bind_resultado = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (bind_resultado != 0)
	{
		herror("Fallo el bind\n");
		exit(-3);
	}

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	log_info(logger, "Server: %s:", msj_server);

	return socket_servidor;
}


int esperar_cliente(int socket_servidor, t_log *logger, char *name_cliente)//podriamos agregar el nombre del servidor
{
	int socket_cliente;

	socket_cliente = accept(socket_servidor, NULL, NULL);// es necesario indicar estos parametros " (struct sockaddr *)&cliente_addr, &addr_size"
	if (socket_cliente == -1)
    {
        perror("accept failed");
        return -1;
    }

    log_info(logger, "Se conecto el cliente: %s", name_cliente);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void handshakeServer(int fd_client)
{
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	recv(fd_client, &handshake, sizeof(int32_t), MSG_WAITALL);
	switch (handshake)
	{
	case 1: // CPU
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	case 2: // Kernel
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	case 3: // Memoria
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	default: // ERROR
		send(fd_client, &resultError, sizeof(int32_t), 0);
		break;
	}
}


t_log *iniciar_logger(char *path_log, char *nombre_log)
{
	t_log *nuevo_logger = log_create(path_log, nombre_log, 1, LOG_LEVEL_INFO);
	if (nuevo_logger == NULL)
	{
		log_error(nuevo_logger, "Error al crear %s\n",nombre_log);
		//printf("Error al crear %s\n",nombre_log);
       	exit(2);
	};
	log_info(nuevo_logger, "Se creo exitosamente, %s\n",nombre_log);
	return nuevo_logger;
}

t_config* iniciar_configs(char* path_config)
{
    t_config* nuevo_config=config_create(path_config);
    if (nuevo_config == NULL) {
		perror("Error al cargar el archivo.");
        exit(EXIT_FAILURE);
        //printf("Error al crear %s\n",path_config);
        //exit(2);
    }
    return nuevo_config;
}


void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete_lista(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}


/**
void finalizar_conexiones(int num_sockets, ...) {
  va_list args;
  va_start(args, num_sockets);

  for (int i = 0; i < num_sockets; i++) {
    int socket_fd = va_arg(args, int);
    close(socket_fd);
  }

  va_end(args);
}*/

void finalizar_modulo(t_log* logger,t_log* logger_obligatorio, t_config* config){
	
	if (logger) {
		log_destroy(logger);
	}
	if (logger_obligatorio){
		log_destroy(logger_obligatorio);
	}
	if (config){
		config_destroy(config);
	}
}