#include <utils/hello.h>
#include <cpu.h>
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
//int fd_memoria
pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_kernel_dispatch;
pthread_t hilo_kernel_interrupt;


int main(int argc, char* argv[]) {
    //saludar("cpu");
    inicializar_cpu();
    crearHilos();
    
    return 0;
}

void inicializar_cpu(){
    cpu_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
    if(cpu_logger == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    cpu_logs_obligatorios = log_create(".//cpu_logs_obligatorios.log", "logs", true, LOG_LEVEL_INFO);
    if(cpu_logs_obligatorios == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    cpu_config = config_create("src/cpu.config");
    if(cpu_config == NULL){
        perror("Error al cargar el archivo.");
        exit(EXIT_FAILURE);
    }

    IP_MEMORIA = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value (cpu_config , "PUERTO_MEMORIA" );
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_DISPATCH" );
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_INTERRUPT" );
    LOG_LEVEL = config_get_string_value(cpu_config, "LOG_LEVEL");

    mostrar_logs();
}

void crearHilos(){
    //Hilo para servidor de CPU - dispatch
    pthread_create(&hilo_cpu_dispatch, NULL, (void*)servidor_cpu_dispatch, NULL);
    pthread_detach(hilo_cpu_dispatch);
    //Hilo para servidor de CPU - interrupt
    pthread_create(&hilo_cpu_interrupt, NULL, (void*)servidor_cpu_interrupt, NULL);
    pthread_detach(hilo_cpu_interrupt);
    //Hilo para conectarse como cliente a Memoria

}

void servidor_cpu_dispatch(){
    fd_cpu_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_logger, "CPU - Dispatch");

    //espera la conexion del kernel
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "Kernel - dispatch")
    handshakeServer(fd_kernel_dispatch);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_dispatch, NULL, (void*)cpu_escuchar_kernel_dispatch, NULL);
    pthread_detach(hilo_kernel_dispatch);
}

void servidor_cpu_interrupt(){
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_logger, "CPU - Interrupt");

    //espera la conexion del kernel
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "Kernel - Interrupt")
    handshakeServer(fd_kernel_interrupt);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_interrupt, NULL, (void*)cpu_escuchar_kernel_interrupt, NULL);
    pthread_detach(hilo_kernel_interrupt);
}

void cpu_escuchar_kernel_dispatch(){
    //atender los msjs de kernel-dispatch
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de KERNEL - Dispatch");
			return EXIT_FAILURE;
		default:
			log_warning(kernel_logger, "Operacion desconocida de KERNEL -Interrupt");
			break;
		}
	}    
}

void cpu_escuchar_kernel_interrupt(){
    //atender los msjs de kernel-interrupt 
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de KERNEL-Interrupt");
			return EXIT_FAILURE;
		default:
			log_warning(kernel_logger, "Operacion desconocida de KERNEL-Interrupt");
			break;
		}
	}
}