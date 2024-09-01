#include <utils/hello.h>
#include <kernel.h>

int main(int argc, char* argv[]) {
    // saludar("kernel");
	t_log* kernelLogger;
	t_log* kernelLoggerObs;
	t_config* kernelConfig;

	kernelLogger = log_create("tp1.kernelClient.logs", "log.kernelClient", true, LOG_LEVEL_INFO);
    if (kernelLogger == NULL){
		perror("No se pudo encontrar o crear el log");
		exit(EXIT_FAILURE);
	}

	kernelLoggerObs = log_create("tp1.kernelClient.logsObs", "log.kernelClientObs", true, LOG_LEVEL_INFO);
    if (kernelLoggerObs == NULL){
		perror("No se pudo encontrar o crear el log");
		exit(EXIT_FAILURE);
	}
    
	kernelConfig = config_create("kernel.config");
    if (kernelConfig == NULL){
		perror("No se pudo cargar la config");
		exit(EXIT_FAILURE);
	}

	iniciarCliente();
	conexionA = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
	enviar_mensaje();
	paquete(conexion);
	
	log_destroy(kernelLogger);
	config_destroy(kernelConfig);

	printf("//////////////----FIN----//////////////");
    return 0;
}

int iniciarCliente(void){
	IP_MEMORIA = config_get_string_value(kernel.config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernel.config,"PUERTO_MEMORIA");
	/*
	IP_CPU = config_get_string_value(kernel.config,"IP");
	PUERTO_CPU_DISPATCH = config_get_string_value(config,"IP");
	PUERTO_CPU_INTERRUPT = config_get_string_value(config,"IP");
	ALGORITMOS_PLANIFICACION = config_get_string_value(kernel.config,"IP");
	QUANTUM = config_get_string_value(kernel.config,"IP");
	*/
	LOG_LEVEL = config_get_string_value(kernel.config,"LOG_LEVEL");
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}
void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);


	send(socket_cliente,a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

