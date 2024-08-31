#include <utils/utils.h>
#include <cpu.h>

int main(int argc, char* argv[]) {
    //saludar("cpu");
    cpu_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
    if(cpu_logger == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    cpu_logs_obligatorios = log_create(".//cpu_logs_obligatorios.log", "logs", true, LOG_LEVEL_INFO);
    if(cpu_logs_obligatorios == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    cpu_config = config_create("src/cpu.config");
    if(cpu_config == NULL){
        perror("Error al cargar el archivo.");
        exit(EXIT_FAILURE);
    }

    IP_MEMORIA = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value (cpu_config , "PUERTO_MEMORIA" );
	PUERTO_ESCUCHA_DISPATCH = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_DISPATCH" );
	PUERTO_ESCUCHA_INTERRUPT = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_INTERRUPT" );
    LOG_LEVEL = config_get_string_value(cpu_config, "LOG_LEVEL"); 

    fd_cpu_dispatch=iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,cpu_logger,"Servidor CPU Dispatch iniciado");
    fd_cpu_interrupt=iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,cpu_logger,"Servidor CPU Interrupt iniciado");
    
    
    fd_memoria=crear_conexion(IP_MEMORIA,PUERTO_MEMORIA,"Conexion con MEMORIA exitosa");
    
    return 0;
}
