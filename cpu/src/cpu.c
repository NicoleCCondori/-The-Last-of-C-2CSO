#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();

    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();
    
    //liberar los logs y config
    free(valores_config_cpu);
    return 0;
}

void inicializar_cpu(){
    cpu_logger = iniciar_logger(".//cpu.log", "log_CPU");
    
    cpu_logs_obligatorios = iniciar_logger(".//cpu_logs_obligatorios.log", "log_CPU");
   
    configurar_cpu();
}

void configurar_cpu(){

    valores_config_cpu = malloc(sizeof(t_config_cpu));
    valores_config_cpu->config = iniciar_configs("src/cpu.config");

    valores_config_cpu->ip_memoria = config_get_string_value(valores_config_cpu->config,"IP_MEMORIA");
    valores_config_cpu->puerto_memoria = config_get_string_value (valores_config_cpu->config, "PUERTO_MEMORIA" );
    valores_config_cpu->puerto_escucha_dispatch = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_DISPATCH");
    valores_config_cpu->puerto_escucha_interrupt = config_get_string_value(valores_config_cpu->config, "PUERTO_ESCUCHA_INTERRUPT");
    valores_config_cpu->log_level = config_get_string_value(valores_config_cpu->config, "LOG_LEVEL");

    //valores_config_cpu = config;
    printf("dsp %s \n",valores_config_cpu->ip_memoria); //se ASIGNA BIEN
	//free(config);
}



void conectar_kernel_dispatch(){
     //Servidor CPU - dispatch
    fd_cpu_dispatch = iniciar_servidor(valores_config_cpu->puerto_escucha_dispatch, cpu_logger, "CPU - Dispatch");
    
    //espera la conexion del kernel
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "Kernel - dispatch");
    handshakeServer(fd_kernel_dispatch);
    //printf("fd_kernel: %d\n", fd_kernel_dispatch);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_dispatch, NULL, (void*)escuchar_kernel_dispatch, NULL);
    pthread_detach(hilo_kernel_dispatch);
}
void conectar_kernel_interrupt(){
    //Servidor CPU - interrupt
    fd_cpu_interrupt = iniciar_servidor(valores_config_cpu->puerto_escucha_interrupt, cpu_logger, "CPU - Interrupt");

    //espera la conexion del kernel
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "Kernel - Interrupt");
    handshakeServer(fd_kernel_interrupt);
    //printf("fd_kernel: %d\n", fd_kernel_interrupt);

    //se crea un hilo para escuchar msj de Kernel - interrupt
    pthread_create(&hilo_kernel_interrupt, NULL, (void*)escuchar_kernel_interrupt, NULL);
    pthread_join(hilo_kernel_interrupt,NULL);
}
void conectar_memoria(){
    //Conexion como cliente a MEMORIA
    fd_memoria = crear_conexion(valores_config_cpu->ip_memoria,valores_config_cpu->puerto_memoria,"MEMORIA",cpu_logger);
    handshakeClient(fd_memoria,1);
    
    //Hilo para enviar mensajes a memoria
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escucha_memoria,NULL);
    pthread_detach(hilo_memoria);
}


void cpu_escucha_memoria(){
    bool control_key = 1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_memoria);
		switch (cod_op)
		{
		case MENSAJE:
		
		case PAQUETE:
		
			break;
		case -1:
			log_error(cpu_logger, "Desconexion de MEMORIA");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de MEMORIA");
			break;
		}
	}
}
void escuchar_kernel_dispatch(){
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
			log_error(cpu_logger, "Desconexion de KERNEL - Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL -Interrupt");
			break;
		}
	}    
}


void escuchar_kernel_interrupt(){
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
			log_error(cpu_logger, "Desconexion de KERNEL-Interrupt");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL-Interrupt");
			break;
		}
	}
}