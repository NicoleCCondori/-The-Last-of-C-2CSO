#include <utils/hello.h>
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

    //Mensajes de memoria
    //preguntar si esta bien esto, no se que onda el tp anterior, por que esucharia a la memoria?
    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escuchar_memoria,NULL);
    pthread_join (hilo_memoria,NULL);

    
    
    return 0;
}
