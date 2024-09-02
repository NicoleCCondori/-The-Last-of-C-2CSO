#include <utils/utils.h>
#include <cpu.h>

int main(int argc, char* argv[]) {

    inicializar_cpu();
    crearHilos();

    //liberar los logs y config
    return 0;
}

void inicializar_cpu(){
    cpu_logger = iniciar_logger(".//cpu.log", "log_CPU");
    
    cpu_logs_obligatorios = iniciar_logger(".//cpu_logs_obligatorios.log", "log_CPU");
   
    cpu_config = iniciar_configs("src/cpu.config");
  
    IP_MEMORIA = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value (cpu_config , "PUERTO_MEMORIA" );
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_DISPATCH" );
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_INTERRUPT" );
    LOG_LEVEL = config_get_string_value(cpu_config, "LOG_LEVEL");

    log_info(cpu_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(cpu_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
}

void crearHilos(){

    //Hilo para conectarse como cliente a Memoria
    fd_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA,"MEMORIA",cpu_logger);
    handshakeClient(fd_memoria,1);
    printf("fd_kernel: %d\n", fd_memoria);

    //Mensajes de memoria
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escuchar_memoria,NULL);
    pthread_detach(hilo_memoria);

    //Servidor CPU - dispatch
    fd_cpu_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_logger, "CPU - Dispatch");
    
    //espera la conexion del kernel
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "Kernel - dispatch");
    handshakeServer(fd_kernel_dispatch);
    printf("fd_kernel: %d\n", fd_kernel_dispatch);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_dispatch, NULL, (void*)cpu_escuchar_kernel_dispatch, NULL);
    pthread_detach(hilo_kernel_dispatch);

    //Servidor CPU - interrupt
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_logger, "CPU - Interrupt");

    //espera la conexion del kernel
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "Kernel - Interrupt");
    handshakeServer(fd_kernel_interrupt);
    printf("fd_kernel: %d\n", fd_kernel_interrupt);

    //se crea un hilo para escuchar msj de Kernel - interrupt
    pthread_create(&hilo_kernel_interrupt, NULL, (void*)cpu_escuchar_kernel_interrupt, NULL);
    pthread_detach(hilo_kernel_interrupt);

}