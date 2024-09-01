#include <utils/hello.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    // saludar("memoria");

    memoryLogger = log_create("tp1.kernelClient.logs", "log.kernelClient", true, LOG_LEVEL_INFO);
    if (memoryLogger == NULL){
		perror("No se pudo encontrar o crear el log");
		exit(EXIT_FAILURE);
	}

	memoryLoggerObs = log_create("tp1.kernelClient.logsObs", "log.kernelClientObs", true, LOG_LEVEL_INFO);
    if (memoryLoggerObs == NULL){
		perror("No se pudo encontrar o crear el log");
		exit(EXIT_FAILURE);
	}

    int serverMemoryActu = inciaServidor();
    log_info(memoryLogger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(serverMemoryActu);

    t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int inciaServidor(void){
    int socketMemory;

	struct addrinfo hints, *servMemoryInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PUERTO, &hints, &servMemoryInfo);

	socketMemory = socket(servMemoryInfo->ai_family,
                        servMemoryInfo->ai_socktype,
                        servMemoryInfo->ai_protocol);

	bind(socketMemory, servMemoryInfo->ai_addr, servMemoryInfo->ai_addrlen);

	listen(socketMemory, SOMAXCONN);

	freeaddrinfo(servMemoryInfo);
	log_trace(memoryLogger, "Listo para escuchar al cliente");

	return socketMemory;
}
int esperar_cliente(int socket_servidor)
{
	int socket_cliente;
	log_info(memoryLogger, "Se conecto un cliente!");

	return socket_cliente;
}