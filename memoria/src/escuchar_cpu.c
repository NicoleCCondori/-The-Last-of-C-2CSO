#include <escuchar_cpu.h>
#include <m_conexiones.h>

void escuchar_cpu(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto memoria_escucha_cpu.c \n");
    bool control_key = 1;
    while (control_key){
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op)
		{
		case MENSAJE:

		case PAQUETE:

			break;
		case -1:
			log_error(memoria_logger, "Desconexion de CPU");
			exit(EXIT_FAILURE);
		default:
			log_warning(memoria_logger, "Operacion desconocida de CPU");
			break;
		}
	}

}


void devolver_contexto_ejecucion (int identificador)
{
	

}
void actualizar_contexto_ejecucion(char* archivoPesudocodigo)
{

}

void obtener_instruccion(char* archivoPesudocodigo)
{

}

//Por cada TID del sistema, se deberá leer su archivo de pseudocódigo 
//y guardar de forma estructurada las instrucciones del mismo para poder 
//devolverlas una a una a pedido de la CPU. Queda a criterio del grupo 
//utilizar la estructura que crea conveniente para este caso de uso.

void READ_MEM(char* archivoPesudocodigo)
{

}

void WRITE_MEM(char* archivoPesudocodigo)
{

}