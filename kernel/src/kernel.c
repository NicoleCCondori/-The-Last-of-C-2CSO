#include <kernel.h>


int main(int argc, char* argv[]) {
    //saludar("kernel");
	t_log* kernel_logger=NULL;
t_log* kernel_logs_obligatorios;
t_config_kernel* valores_config_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;

pthread_t hilo_cpu_dispatch;
pthread_t hilo_cpu_interrupt;
pthread_t hilo_memoria;

    inicializar_kernel();

    conectar_cpu_dispatch();
    conectar_cpu_interrupt();
    conectar_memoria();


    
    //liberar los logs y config
    free(valores_config_kernel);

    //falta finalizar las conexiones
    return 0;
}

void inicializar_kernel(){
    kernel_logger = iniciar_logger(".//kernel.log", "log_KERNEL");
   
    kernel_logs_obligatorios = iniciar_logger(".//kernel_logs_obligatorios.log", "logs");
    
    configurar_kernel();

}

void configurar_kernel() {
    valores_config_kernel = malloc(sizeof(t_config_kernel));

    valores_config_kernel->config = iniciar_configs("src/kernel.config");

    valores_config_kernel->ip_memoria = config_get_string_value(valores_config_kernel->config,"IP_MEMORIA");
    valores_config_kernel->puerto_escucha= config_get_string_value (valores_config_kernel->config , "PUERTO_ESCUCHA" );
    valores_config_kernel->puerto_memoria = config_get_string_value (valores_config_kernel->config , "PUERTO_MEMORIA" );
	valores_config_kernel->ip_cpu = config_get_string_value (valores_config_kernel->config , "IP_CPU");
	valores_config_kernel->puerto_cpu_dispatch = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_DISPATCH" );
    valores_config_kernel->puerto_cpu_interrupt = config_get_string_value (valores_config_kernel->config , "PUERTO_CPU_INTERRUPT" );
    valores_config_kernel->algoritmo_planificacion = config_get_string_value (valores_config_kernel->config , "ALGORITMO_PLANIFICACION");
    valores_config_kernel->quantum = config_get_string_value (valores_config_kernel->config , "QUANTUM");
    valores_config_kernel->log_level = config_get_string_value(valores_config_kernel->config, "LOG_LEVEL");

	//valores_config_kernel = config;
    printf("dsp %s \n",valores_config_kernel->puerto_escucha); //se ASIGNA BIEN

	//free(config);
}

//Revisar mas adelante
/*void conectar_cpu_dispatch(){
    //Cliente KERNEL a CPU-dispatch
	fd_cpu_dispatch = crear_conexion(valores_config_kernel->ip_memoria, valores_config_kernel->puerto_cpu_dispatch, "CPU - Dispatch",kernel_logger);
	handshakeClient(fd_cpu_dispatch,2);
    
	//hilo para conectarse con cpu Dispatch / atender
    pthread_create(&hilo_cpu_dispatch, NULL, (void*)kernel_escucha_cpu_dispatch,NULL);
    pthread_detach(hilo_cpu_dispatch);
}*/
void conectar_cpu_interrupt(){
    //Cliente KERNEL a CPU-interrupt
	fd_cpu_interrupt = crear_conexion(valores_config_kernel->ip_cpu, valores_config_kernel->puerto_cpu_interrupt, "CPU - Interrupt",kernel_logger);
	handshakeClient(fd_cpu_interrupt,2);
    
    //hilo para conectarse con cpu - Interrupt / atender
    pthread_create(&hilo_cpu_interrupt, NULL, (void*)kernel_escucha_cpu_interrupt,NULL);
    pthread_detach(hilo_cpu_interrupt);
}
void conectar_memoria(){
    //cliente KERNEL - MEMORIA
    fd_memoria = crear_conexion(valores_config_kernel->ip_cpu,valores_config_kernel->puerto_memoria,"MEMORIA",kernel_logger);
    handshakeClient(fd_memoria,2);

    //hilo para conectarse con memoria / atender
    pthread_create(&hilo_memoria, NULL, (void*)kernel_escucha_memoria,NULL);
    pthread_join(hilo_memoria,NULL);
}

void kernel_escucha_memoria(){
    //atender los msjs de memoria
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
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}

void kernel_escucha_cpu_dispatch(){
    //atender los msjs de cpu-dispatch , otra funcion?
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_cpu_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU-Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de CPU-Dispatch");
			break;
		}
	}
	//close(fd_cpu_dispatch); //liberar_conexion(fd_cpu_dispatch);
}

void kernel_escucha_cpu_interrupt(){

    //atender los msjs de cpu-interrupt , otra funcion?
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_cpu_interrupt);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(kernel_logger, "Operacion desconocida de CPU-Interrupt");
			break;
		}
	}
	//close(fd_cpu_interrupt); //liberar_conexion(fd_cpu_interrupt);
}


