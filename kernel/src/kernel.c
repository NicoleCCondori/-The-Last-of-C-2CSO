#include <utils/utils.h>
#include <kernel.h>

int main(int argc, char* argv[]) {
    //saludar("kernel");
    inicializar_kernel();
    crearHilos();

    //liberar los logs y config

    return 0;
}

void inicializar_kernel(){
    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    config = iniciar_configs("src/kernel.config");

    t_config_kernel* kernel_config;

    kernel_config->ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    kernel_config->puerto_escucha= config_get_string_value (config , "PUERTO_ESCUCHA" );
    kernel_config->puerto_memoria = config_get_string_value (config , "PUERTO_MEMORIA" );
	kernel_config->ip_cpu = config_get_string_value (config , "IP_CPU");
	kernel_config->puerto_cpu_dispatch = config_get_string_value (config , "PUERTO_CPU_DISPATCH" );
    kernel_config->puerto_cpu_interrupt = config_get_string_value (config , "PUERTO_CPU_INTERRUPT" );
    kernel_config->algoritmo_planificacion = config_get_string_value (config , "ALGORITMO_PLANIFICACION");
    kernel_config->quantum = config_get_string_value (config , "QUANTUM");
    kernel_config->log_level = config_get_string_value(config, "LOG_LEVEL");

    log_info(kernel_logger, "IP_MEMORIA: %s", kernel_config->ip_memoria);
    log_info(kernel_logger, "PUERTO_ESCUCHA: %s", kernel_config->puerto_escucha);
    log_info(kernel_logger, "PUERTO_MEMORIA: %s", kernel_config->puerto_memoria);
    log_info(kernel_logger, "IP_CPU: %s", kernel_config->ip_cpu);
    log_info(kernel_logger, "PUERTO_CPU_DISPATCH: %s", kernel_config->puerto_cpu_dispatch);
    log_info(kernel_logger, "PUERTO_CPU_INTERRUPT: %s", kernel_config->puerto_cpu_interrupt);

}

void crearHilos(){ //analizar si utilizo 3 hilos o 5 ------------

	//Cliente KERNEL a CPU-dispatch
	fd_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch, "CPU - Dispatch",kernel_logger);
	handshakeClient(fd_cpu_dispatch,2);
    
	//hilo para conectarse con cpu Dispatch / atender
    pthread_create(&hilo_cpu_dispatch, NULL, (void*)conexion_cpu_dispatch,NULL);
    pthread_detach(hilo_cpu_dispatch);

	
	//Cliente KERNEL a CPU-interrupt
	fd_cpu_interrupt = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt, "CPU - Interrupt",kernel_logger);
	handshakeClient(fd_cpu_interrupt,2);

    //hilo para conectarse con cpu - Interrupt / atender
    pthread_create(&hilo_cpu_interrupt, NULL, (void*)conexion_cpu_interrupt,NULL);
    pthread_join(hilo_cpu_interrupt,NULL);
    
    //hilo para conectarse con memoria / atender
   // pthread_create(&hilo_memoria, NULL, (void*)conexion_memoria,NULL);
   // pthread_detach(hilo_memoria); 
}


/*void conexion_memoria(){
    fd_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria, "MEMORIA");

    //atender los msjs de memoria , otra funcion?
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de MEMORIA");
			return EXIT_FAILURE;
		default:
			log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
	return EXIT_SUCCESS;
}*/


