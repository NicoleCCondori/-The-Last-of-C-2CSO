
















































#include <utils/utils.h>
#include <memoria.h>

int main(int argc, char* argv[]) {
    
    logger_memoria = iniciar_logger("memoria");
    levantar_config_memoria("memoria");
    inicializar_memoria();
}

void inicializar_memoria(){
    conexion = crear_conexion(ip, puerto);
}
