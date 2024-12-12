#include <escuchar_kernel_dispatch.h>
#include <c_conexiones.h>



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
			t_paquete* paquete_aux = recibir_paquete(fd_kernel_dispatch);

            //deserializo llamando a la función "deserializar_enviar_contexto"
			    
            t_enviar_contexto* contexto = deserializar_enviar_contexto(paquete_aux);
            //uint32_t pidHilo= leer_buffer_Uint32(paquete_aux->buffer);
		    //uint32_t tidHilo = leer_buffer_Uint32(paquete_aux->buffer);
            PidHilo = contexto->PID;
            TidHilo = contexto->TID;
            //Debo liberar la memoria
            //free(contexto); no sé si liberaba aca por el tema del malloc
            destruir_buffer_paquete(paquete_aux);
			obtener_contexto(fd_memoria, PidHilo, TidHilo); //se envia a memoria el tid y pid para obtener el contexto de ejecucion

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


void obtener_contexto(int fd_memoria,uint32_t pid, uint32_t tidHilo){
    log_info(cpu_logger,"## TID: <%d> - Solicito contexto Ejecucion",tidHilo);
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion=OBTENER_CONTEXTO;
    paquete->buffer=malloc(sizeof(t_buffer));

    paquete->buffer->size=sizeof(uint32_t)*2;
    paquete->buffer->stream=malloc(paquete->buffer->size);
    int offset=0;

    memcpy(paquete->buffer->stream,&pid,sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(paquete->buffer->stream,&tidHilo,sizeof(uint32_t));

    int bytes=sizeof(op_code)+sizeof(int)+paquete->buffer->size;
    void* a_enviar=malloc(bytes);
    
    memcpy(a_enviar+offset,&(paquete->codigo_operacion),sizeof(op_code));
    offset+=sizeof(op_code);
    
    memcpy(a_enviar+offset,&(paquete->buffer->size),sizeof(int));
    offset=sizeof(int);

    memcpy(a_enviar+offset,&(paquete->buffer->stream),paquete->buffer->size);

    int flags= send(fd_memoria,a_enviar,bytes,0);
    if (flags==-1)
    {
        log_error(cpu_logger,"Error al enviar PID y TID");
    }
    
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);    
}