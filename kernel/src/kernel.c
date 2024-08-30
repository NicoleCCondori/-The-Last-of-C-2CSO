#include <utils/hello.h>
#include <kernel.h>

int main(int argc, char* argv[]) {
    //saludar("kernel");
    kernel_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
    if(kernel_logger == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    kernel_logs_obligatorios = log_create(".//kernel_logs_obligatorios.log", "logs", true, LOG_LEVEL_INFO);
    if(kernel_logs_obligatorios == NULL){
        perror("Algo paso con el log. No se pudo crear.");
        exit(EXIT_FAILURE);
    }

    kernel_config = config_create("src/kernel.config");
    if(kernel_config == NULL){
        perror("Error al cargar el archivo.");
        exit(EXIT_FAILURE);
    }

    IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA");
    PUERTO_ESCUCHA = config_get_string_value (kernel_config , "PUERTO_ESCUCHA" );
    PUERTO_MEMORIA = config_get_string_value (kernel_config , "PUERTO_MEMORIA" );
	IP_CPU = config_get_string_value (kernel_config , "IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value (kernel_config , "PUERTO_CPU_DISPATCH" );
    PUERTO_CPU_INTERRUPT = config_get_string_value (kernel_config , "PUERTO_CPU_INTERRUPT" );
    ALGORITMO_PLANIFICACION = config_get_string_value (kernel_config , "ALGORITMO_PLANIFICACION");
    QUANTUM = config_get_string_value (kernel_config , "QUANTUM");
    LOG_LEVEL = config_get_string_value(kernel_config, "LOG_LEVEL");
    
    return 0;
}
