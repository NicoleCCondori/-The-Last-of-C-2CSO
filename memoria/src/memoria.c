#include <utils/hello.h>
#include <memoria.h>

/*
	Objetivos para la memoria

	*Crear los logs y archivos de configuracion
	*Establecer conexion con kernel
	-crear los procesos
		-recibir un mensaje

*/

int main(int argc, char* argv[]) {

	memoryLogger = iniciar_logger("tp.memoria.logs", "log.memoria");
	memoryLoggerObs = iniciar_logger("tp.memoria.logsObs", "log.memoria");
	memoryConfig = iniciar_configs("memoria.config");

    // saludar("memoria");
	/*
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

	memoryConfig = config_create("memoria.config");
    if (memoryConfig == NULL){
		perror("No se pudo cargar la config");
		exit(EXIT_FAILURE);
	}
	*/
	PUERTO_ESCUCHA = config_get_string_value(memoryConfig,"PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(memoryConfig,"IP_FILESYSTEM");
	PUERTO_FILESYSTEM = config_get_string_value(memoryConfig,"PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_string_value(memoryConfig,"TAM_MEMORIA");
	PATH_INSTRUCCIONES = config_get_string_value(memoryConfig,"PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_string_value(memoryConfig,"RETARDO_RESPUESTA");
	ESQUEMA = config_get_string_value(memoryConfig,"ESQUEMA");
	ALGORITMO_BUSQUEDA = config_get_string_value(memoryConfig,"ALGORITMO_BUSQUEDA");
	PARTICIONES = config_get_string_value(memoryConfig,"PARTICIONES");
	LOG_LEVEL = config_get_string_value(memoryConfig,"LOG_LEVEL");


    int serverMemoryActu = iniciar_servidor(PUERTO_ESCUCHA,memoryLogger,"memoria");
    log_info(memoryLogger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(serverMemoryActu,memoryLogger,"> kernel");
	handshakeServer(cliente_fd);

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			
			break;
		case PAQUETE:
			
			break;
		case -1:
			log_error(memoryLogger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(memoryLogger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}
