#include <utils/utils.h>
#include <cpu.h>

t_config_cpu* valores_config_cpu;

int main(int argc, char* argv[]) {

    inicializar_cpu();

    valores_config_cpu = configurar_cpu();

    crearHilos();

    //liberar los logs y config
    return 0;
}

void inicializar_cpu(){
    cpu_logger = iniciar_logger(".//cpu.log", "log_CPU");
    
    cpu_logs_obligatorios = iniciar_logger(".//cpu_logs_obligatorios.log", "log_CPU");
}

t_config_cpu* configurar_cpu(){

    t_config_cpu* config = malloc(sizeof(t_config_cpu));
    config->config = iniciar_configs("src/cpu.config");

    config->ip_memoria = config_get_string_value(config->config,"IP_MEMORIA");
    config->puerto_memoria = config_get_string_value (config->config, "PUERTO_MEMORIA" );
    config->puerto_escucha_dispatch = config_get_string_value(config->config, "PUERTO_ESCUCHA_DISPATCH");
    config->puerto_escucha_interrupt = config_get_string_value(config->config, "PUERTO_ESCUCHA_INTERRUPT");
    config->log_level = config_get_string_value(config->config, "LOG_LEVEL");

    return config;
}

void crearHilos(){

    //Cliente CPU a Memoria
    fd_memoria = crear_conexion(valores_config_cpu->ip_memoria,valores_config_cpu->puerto_memoria,"MEMORIA",cpu_logger);
    handshakeClient(fd_memoria,1);
    //printf("fd_kernel: %d\n", fd_memoria);

    //Hilo para enviar mensajes a memoria
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escuchar_memoria,NULL);
    pthread_detach(hilo_memoria);

    //Servidor CPU - dispatch
    fd_cpu_dispatch = iniciar_servidor(valores_config_cpu->puerto_escucha_dispatch, cpu_logger, "CPU - Dispatch");
    
    //espera la conexion del kernel
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "Kernel - dispatch");
    handshakeServer(fd_kernel_dispatch);
    //printf("fd_kernel: %d\n", fd_kernel_dispatch);

    //se crea un hilo para escuchar msj de Kernel - dispatch
    pthread_create(&hilo_kernel_dispatch, NULL, (void*)cpu_escuchar_kernel_dispatch, NULL);
    pthread_detach(hilo_kernel_dispatch);

    //Servidor CPU - interrupt
    fd_cpu_interrupt = iniciar_servidor(valores_config_cpu->puerto_escucha_interrupt, cpu_logger, "CPU - Interrupt");

    //espera la conexion del kernel
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "Kernel - Interrupt");
    handshakeServer(fd_kernel_interrupt);
    //printf("fd_kernel: %d\n", fd_kernel_interrupt);

    //se crea un hilo para escuchar msj de Kernel - interrupt
    pthread_create(&hilo_kernel_interrupt, NULL, (void*)cpu_escuchar_kernel_interrupt, NULL);
    pthread_join(hilo_kernel_interrupt,NULL);

}