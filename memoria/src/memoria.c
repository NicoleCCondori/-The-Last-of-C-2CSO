#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    memoria_logger=iniciar_logger(".//tp.log","log_MEMORIA");
    memoria_log_obligatorios=iniciar_logger(".//tp.logs_olbigatorios.log","logs_MEMORIA");
    memoria_config=iniciar_configs("src/memoria.config");
    
    PUERTO_ESCUCHA=config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    
    fd_memoria=iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria iniciada ");
    printf("fd_memoria: %d\n", fd_memoria);
    //Esperar conexion CPU

    log_info(memoria_logger,"Esperando CPU...");
    fd_cpu=esperar_cliente(fd_memoria,memoria_logger,"CPU");
    handshakeServer(fd_cpu);
    printf("fd_cpu: %d\n", fd_cpu);

    //mensajes de CPU
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu,NULL,(void*)memoria_escuchar_cpu,NULL);
    pthread_detach (hilo_cpu);

    return 0;
}