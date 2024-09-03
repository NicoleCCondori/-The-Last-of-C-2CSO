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
    
    kernel_config = iniciar_configs("src/kernel.config");
    

    IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA");
    PUERTO_ESCUCHA = config_get_string_value (kernel_config , "PUERTO_ESCUCHA" );
    PUERTO_MEMORIA = config_get_string_value (kernel_config , "PUERTO_MEMORIA" );
	IP_CPU = config_get_string_value (kernel_config , "IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value (kernel_config , "PUERTO_CPU_DISPATCH" );
    PUERTO_CPU_INTERRUPT = config_get_string_value (kernel_config , "PUERTO_CPU_INTERRUPT" );
    ALGORITMO_PLANIFICACION = config_get_string_value (kernel_config , "ALGORITMO_PLANIFICACION");
    QUANTUM = config_get_string_value (kernel_config , "QUANTUM");
    LOG_LEVEL = config_get_string_value(kernel_config, "LOG_LEVEL");

    log_info(kernel_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(kernel_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    log_info(kernel_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(kernel_logger, "IP_CPU: %s", IP_CPU);
    log_info(kernel_logger, "PUERTO_CPU_DISPATCH: %s", PUERTO_CPU_DISPATCH);
    log_info(kernel_logger, "PUERTO_CPU_INTERRUPT: %s", PUERTO_CPU_INTERRUPT);

}

void crearHilos(){ //analizar si utilizo 3 hilos o 5 ------------

	//Cliente KERNEL a CPU-dispatch
	fd_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH, "CPU - Dispatch",kernel_logger);
	handshakeClient(fd_cpu_dispatch,2);
    
	//hilo para conectarse con cpu Dispatch / atender
    pthread_create(&hilo_cpu_dispatch, NULL, (void*)conexion_cpu_dispatch,NULL);
    pthread_detach(hilo_cpu_dispatch);

	
	//Cliente KERNEL a CPU-interrupt
	fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT, "CPU - Interrupt",kernel_logger);
	handshakeClient(fd_cpu_interrupt,2);

    //hilo para conectarse con cpu - Interrupt / atender
    pthread_create(&hilo_cpu_interrupt, NULL, (void*)conexion_cpu_interrupt,NULL);
    pthread_join(hilo_cpu_interrupt,NULL);
    
    //hilo para conectarse con memoria / atender
   // pthread_create(&hilo_memoria, NULL, (void*)conexion_memoria,NULL);
   // pthread_detach(hilo_memoria); 
}


/*void conexion_memoria(){
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, "MEMORIA");

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


