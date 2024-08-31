#include <utils/utils.h>

/*void saludar(char* quien) {
    printf("Hola desde %s!!\n", quien);
}*/

int crear_conexion(char *ip, char *puerto, char *name_server)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

    //
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    //
	int resultado = getaddrinfo(ip, puerto, &hints, &server_info);
	if (resultado != 0)
	{
		printf("Error en getaddrinfo: %s", gai_strerror(resultado));
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
		printf("El cliente se conecto al servidor correctamente a %s.\n", name_server);
	}
	else
	{
		printf("Error al conectar servidor %s\n", name_server); //ACA FALTABA EL PARENTESIS
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

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



t_log *iniciar_logger(char *modulo)
{
	t_log *nuevo_logger = log_create(modulo, modulo, 1, LOG_LEVEL_INFO);
	if (nuevo_logger == NULL)
	{
		printf("Error al crear %s\n",modulo);
       	exit(2);
	};
	return nuevo_logger;
}

t_config* iniciar_configs(char* modulo){
    t_config* nuevo_config=config_create(modulo);
    if (nuevo_config == NULL) {
        printf("Error al crear %s\n",modulo);
        exit(2);
    }
    return nuevo_config;
}
