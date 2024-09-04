#include <utils/hello.h>
#include <kernel.h>

#include <utils/utils.h>

/*
	Objetivos para el kernel

	*Crear los logs y archivos de configuracion
	*Establecer conexion con kernel
	-crear los procesos
		-enviar un mensaje

	--------------*hilos------------------- 
	THREAD_CREATE, esta syscall recibirá como parámetro de la CPU el nombre del archivo de pseudocódigo que deberá ejecutar el hilo a crear y su prioridad. Al momento de crear el nuevo hilo, deberá generar el nuevo TCB con un TID autoincremental y poner al mismo en el estado READY.
	THREAD_JOIN, esta syscall recibe como parámetro un TID, mueve el hilo que la invocó al estado BLOCK hasta que el TID pasado por parámetro finalice. En caso de que el TID pasado por parámetro no exista o ya haya finalizado, esta syscall no hace nada y el hilo que la invocó continuará su ejecución.
	THREAD_CANCEL, esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT. Se deberá indicar a la Memoria la finalización de dicho hilo. En caso de que el TID pasado por parámetro no exista o ya haya finalizado, esta syscall no hace nada. Finalmente, el hilo que la invocó continuará su ejecución.
	THREAD_EXIT, esta syscall finaliza al hilo que lo invocó, pasando el mismo al estado EXIT. Se deberá indicar a la Memoria la finalización de dicho hilo.
*/
int main(int argc, char* argv[]) {
    // saludar("kernel");
	t_log* kernelLogger;
	// t_log* kernelLoggerObs;
	t_config* kernelConfig;
	
	kernelLogger = iniciar_logger("tp1.kernelClient.logs", "log.kernelClient");
	kernelLoggerObs = iniciar_logger("tp1.kernelClient.logsObs", "log.kernelClientObs");
	kernelConfig = iniciar_configs("kernel.config");

	/*
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
	*/

	IP_MEMORIA = config_get_string_value(kernelConfig,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernelConfig,"PUERTO_MEMORIA");
	LOG_LEVEL = config_get_string_value(kernelConfig,"LOG_LEVEL");

	//iniciarCliente();
	
	kernelConexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA,"memoria",kernelLogger);
	
	// pthread_create()
	
	/*
	log_destroy(kernelLogger);
	config_destroy(kernelConfig);
	*/

	printf("//////////////----FIN----//////////////");
    return 0;
}
/*
int iniciarCliente(void){
	IP_MEMORIA = config_get_string_value(kernel.config,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernel.config,"PUERTO_MEMORIA");
	LOG_LEVEL = config_get_string_value(kernel.config,"LOG_LEVEL");

	IP_CPU = config_get_string_value(kernel.config,"IP");
	PUERTO_CPU_DISPATCH = config_get_string_value(config,"IP");
	PUERTO_CPU_INTERRUPT = config_get_string_value(config,"IP");
	ALGORITMOS_PLANIFICACION = config_get_string_value(kernel.config,"IP");
	QUANTUM = config_get_string_value(kernel.config,"IP");
	
}
*/

/*
int crear_conexion1(char *ip, char* puerto)
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
*/

