#include <utils/hello.h>
#include <cpu.h>
pthread_t hilo_CPU_Cliente
int main(int argc, char* argv[]) {
    //Inicializar cpu
   
    inicializar_cpu();

    //Conectarnos como cliente a memoria
    
    fd_memoria=crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    log_info(cpu_logger,"Conexion memoria exitosa");

    //saludar("cpu");
    return 0;
}
