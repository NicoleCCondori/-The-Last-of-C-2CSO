#include <utils/utils.h>

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

// <<<<<<< HEAD
	send(fd_servidor, &handshake, sizeof(int32_t), 0);
// =======
	send(fd_servidor, &handshake, sizeof(int32_t), 0); //tira error cuando conectamos kernel ---> falta liberar sockets?
// >>>>>>> check1V2
	recv(fd_servidor, &result, sizeof(int32_t), 0);

	if (result == 0)
		printf("Handshake Success\n");
	else
		printf("Handshake Failure\n");
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
//<<<<<<< HEAD
//=======
	log_info(nuevo_logger, "Se creo exitosamente, %s\n",nombre_log);
//>>>>>>> check1V2
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
//////////////////////////////////////////////
typedef struct
{
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
	uint32_t EX;
	uint32_t FX;
	uint32_t GX;
	uint32_t HX;
} RegistrosCPU;

typedef enum{
	NEW,
	BLOCKED,
	READY,
	EXIT,
	EXEC
} estado_proceso_hilo;

typedef struct 
{
	uint32_t pid; //Identificador del proceso
	t_list* tid; //Lista de los identificadores de los hilos asociados al proceso
	t_list* mutex; //Lista de los mutex creados para el proceso a lo largo de la ejecución de sus hilos, ¿'que se debe guardar exactamente?
	uint32_t pc; //Program Counter, indica la próxima instrucción a ejecutar
	estado_proceso_hilo estado; //para saber en que estado se encuntra el proceso/hilo
	int tam_proceso;
} PCB;

typedef struct 
{
	uint32_t pid; //Identificador del proceso al que pertenece
	uint32_t tid; //Identificador del hilo
	int prioridad;//0 maxima prioridad
	RegistrosCPU* registro;
	
} TCB;

////////////////////////////////////////////////////////

void agregar_buffer_char* (t_buffer buffer,char)

//PARA SERIALIZAR
void agregar_buffer_int(t_buffer buffer, int entero){
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(int));
    buffer->size += sizeof(int);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(int));
    buffer->offset += sizeof(int);
}

void agregar_buffer_Uint32(t_buffer buffer, uint32_t entero){
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
    buffer->size += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

void agregar_buffer_Uint8(t_buffer *buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}

void agregar_buffer_string(t_buffer* buffer, char* args){
    uint32_t tamanio = strlen(args) +1;
    agregar_buffer_Uint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->offset, args, tamanio);
    buffer->offset += tamanio;
    buffer->size += tamanio;
}

//PARA DESERIALIZAR
uint32_t leer_buffer_int(t_buffer* buffer)
{
    int entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(int));
    buffer->offset += sizeof(int);
    return entero;
}

uint32_t leer_buffer_Uint32(t_buffer* buffer)
{
    uint32_t entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    return entero;
}

uint8_t leer_buffer_Uint8(t_buffer* buffer)
{
    uint8_t entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
    return entero;
}

char *leer_buffer_string(tipo_buffer* buffer)
{
    char *cadena;
    uint32_t tamanio;

    tamanio = leer_buffer_enteroUint32(buffer);
    cadena = malloc((tamanio) + 1);
    memcpy(cadena, buffer->stream + buffer->offset, tamanio);
    buffer->offset += tamanio;

    *(cadena + tamanio) = '\0';

    return cadena;
}


void *serializar_paquete(t_paquete *paquete, int bytes){
	void *magic = malloc(bytes); // Reserva memoria del tamaño del paquete completo
	int offset = 0;

	memcpy(magic + offset, &(paquete->codigo_operacion), sizeof(int));
	offset += sizeof(int);

	memcpy(magic + offset, &(paquete->buffer->size), sizeof(int));
	offset += sizeof(int);

	memcpy(magic + offset, paquete->buffer->stream, paquete->buffer->size);
	offset += paquete->buffer->size;

	return magic;
}
