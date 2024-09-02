#include <utils/utils.h>
#include <cpu.h>

int main(int argc, char* argv[]) {
    






    

    //fd_cpu_dispatch=iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,cpu_logger,"CPU Dispatch");
    //fd_cpu_interrupt=iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,cpu_logger,"CPU Interrupt");
    
    

    fd_memoria=crear_conexion(IP_MEMORIA,PUERTO_MEMORIA,"MEMORIA");//agregar el parametro de cpu_logger

    //Mensajes de memoria
    //preguntar si esta bien esto, no se que onda el tp anterior, por que esucharia a la memoria?
    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria,NULL,(void*)cpu_escuchar_memoria,NULL);
    pthread_join (hilo_memoria,NULL);


    return 0;
}
