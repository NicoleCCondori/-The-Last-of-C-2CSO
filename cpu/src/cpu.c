#include <utils/utils.h>
#include <cpu.h>
pthread_t hilo_CPU_Cliente
int main(int argc, char* argv[]) {

   



















   
    
    IP_MEMORIA=config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA=config_get_string_value(cpu_config,"PUERTO_MEMORIA")
    PUERTO_ESCUCHA_DISPATCH=config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT=config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT");

    fd_cpu_dispatch=iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,cpu_logger,"Servidor CPU Dispatch iniciado");
    fd_cpu_interrupt=iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,cpu_logger,"Servidor CPU Interrupt iniciado");
    
    
    fd_memoria=crear_conexion(IP_MEMORIA,PUERTO_MEMORIA,"Conexion con MEMORIA exitosa");
    
    
    return 0;
}
