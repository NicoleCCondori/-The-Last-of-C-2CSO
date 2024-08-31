#include <utils/hello.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    saludar("memoria");
    //Iniciar memoria como servidor
    
    fd_memoria=iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria iniciada ");
    
    //Esperar conexion CPU

    log_info(memoria_logger,"Esperando CPU");
    fd_cpu=esperar_cliente(fd_memoria,memoria_logger,"Memoria")

    return 0;
}
