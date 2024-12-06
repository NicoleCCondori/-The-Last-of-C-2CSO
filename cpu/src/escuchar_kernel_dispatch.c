#include <escuchar_kernel_dispatch.h>
#include <c_conexiones.h>

uint32_t pidHilo;
uint32_t tidHilo;

void escuchar_kernel_dispatch(){
    //atender los msjs de kernel-dispatch
    bool control_key=1;
    while (control_key)
	{
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op)
		{
		case MENSAJE:

		case RECIBIR_TID:
<<<<<<< HEAD
		t_paquete* paquete_aux=recibir_paquete(fd_kernel_dispatch);
		tidHilo=leer_buffer_uint32(paquete_aux->buffer);
=======
			t_paquete* paquete_aux = recibir_paquete(fd_kernel_dispatch);

            //deserializo llamando a la función "deserializar_enviar_contexto"
			    
            t_enviar_contexto* contexto = deserializar_enviar_contexto(paquete_aux);
            //uint32_t pidHilo= leer_buffer_Uint32(paquete_aux->buffer);
		    //uint32_t tidHilo = leer_buffer_Uint32(paquete_aux->buffer);
            pidHilo = contexto->PID;
            tidHilo = contexto->TID;
            //Debo liberar la memoria
            //free(contexto); no sé si liberaba aca por el tema del malloc
            destruir_buffer_paquete(paquete_aux);
			obtener_contexto(fd_memoria, pidHilo, tidHilo); //se envia a memoria el tid y pid para obtener el contexto de ejecucion
>>>>>>> origin/checkpoint3

		case PAQUETE:

			break;
		case -1:
			log_error(cpu_logger, "Desconexion de KERNEL - Dispatch");
			exit(EXIT_FAILURE);
		default:
			log_warning(cpu_logger, "Operacion desconocida de KERNEL -Interrupt");
			break;
		}
	}    
}