#include <escuchar_kernel_dispatch.h>


void escuchar_kernel_dispatch(){
    //atender los msjs de kernel-dispatch
    
    while (1)
	{

        t_paquete* paquete_kernel = recibir_paquete(fd_kernel_dispatch);
        if (paquete_kernel == NULL) {
            log_error(cpu_logger, "Error: paquete_kernel es NULL");
            return;
        }
        if (paquete_kernel->buffer == NULL) {
            log_error(cpu_logger, "Error: buffer de paquete_kernel es NULL");
            return;
        }

        op_code cod_op = paquete_kernel->codigo_operacion;
        log_info(cpu_logger,"Codigo de operacion: %d", cod_op);
        
		switch (cod_op)
		{
            case MENSAJE:
                break;

            case RECIBIR_TID:
            
                log_info(cpu_logger,"recibiendo paquete de kernel TID!!!\n");
                pthread_mutex_lock(&mutex_contextos2);
                t_enviar_contexto* contexto = deserializar_enviar_contexto(paquete_kernel);
                pthread_mutex_unlock(&mutex_contextos2);

                log_info(cpu_logger,"PID: %u\n", contexto->PID);
                log_info(cpu_logger,"TID: %u\n", contexto->TID);
                PidHilo = contexto->PID;
                TidHilo = contexto->TID;

                log_info(cpu_logger,"Recibo de kernel pid <%u> y tid <%u>", PidHilo, TidHilo);
                
                //Debo liberar la memoria
                //free(contexto); no sé si liberaba aca por el tema del malloc
                //sem_post(&sem_syscallKernel);

                pthread_mutex_lock(&mutex_contextos2);
                obtener_contexto(fd_memoria, PidHilo, TidHilo); //se envia a memoria el tid y pid para obtener el contexto de ejecucion
                pthread_mutex_unlock(&mutex_contextos2);

                break;

            case -1:
                log_error(cpu_logger, "Desconexion de KERNEL - Dispatch");
                exit(EXIT_FAILURE);
            default:
                log_warning(cpu_logger, "Operacion desconocida de KERNEL -Dispatch");
                break;
            }
            if(paquete_kernel!=NULL){
                eliminar_paquete(paquete_kernel);
            }
	}    
}

void obtener_contexto(int fd_memoria,uint32_t pid, uint32_t tidHilo){
    log_info(cpu_logger,"Enviamos(Obtener contexto) El pid <%d> y tid <%d> a memoria", pid, tidHilo);
    t_paquete* paquete_obtener_contexto = crear_paquete(OBTENER_CONTEXTO);
    serializar_hilo_cpu(paquete_obtener_contexto, pid, tidHilo);
    enviar_paquete(paquete_obtener_contexto, fd_memoria);
    log_info(cpu_logger,"Se acaba de pedir el contexto a memoria!! \n");
    eliminar_paquete(paquete_obtener_contexto);
}