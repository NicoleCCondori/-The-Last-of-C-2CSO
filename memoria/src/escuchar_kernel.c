#include <escuchar_kernel.h>
#include <m_conexiones.h>

void escuchar_kernel(){
    printf("Ejecuto escuchar_kernel.c \n");

    
    while (1){
		t_paquete* paquete_kernel = recibir_paquete(fd_kernel);
        op_code codigo_operacion = paquete_kernel->codigo_operacion;
		switch (codigo_operacion)
		{
		case ASIGNAR_MEMORIA:
		asignar_memoria();
		break;

		case HILO_READY:
		     crear_hilo();
			break;


		default:
			log_warning(memoria_logger, "Operacion desconocida de KERNEL");
			break;
		}
		eliminar_paquete(paquete_kernel);
	}

}

void asignar_memoria(){
	int valor_a_enviar = 0;
	send(fd_kernel, &valor_a_enviar, sizeof(valor_a_enviar), 0);
}

void crear_hilo(){
	t_paquete* enviar_contexto = recibir_paquete(fd_cpu);
    t_crear_hilo* datos_hilo = deserializar_crear_hilo(enviar_contexto);

	TCB* nuevo_tcb = malloc(sizeof(TCB));
    if (!nuevo_tcb) {
        log_error(memoria_logger, "Error al crear el TCB");
        exit(EXIT_FAILURE);
    }

    nuevo_tcb->pid = datos_hilo->PID;
    nuevo_tcb->tid = datos_hilo->TID;
    nuevo_tcb->prioridad = datos_hilo->prioridad;
    nuevo_tcb->path = strdup(datos_hilo->path);
	nuevo_tcb->pc = 0;

    nuevo_tcb->registro = malloc(sizeof(RegistrosCPU));
    if (!nuevo_tcb->registro) {
        log_error(memoria_logger, "Error al inicializar los registros de CPU");
        exit(EXIT_FAILURE);
    }

    nuevo_tcb->registro->AX = 0;
    nuevo_tcb->registro->BX = 0;
    nuevo_tcb->registro->CX = 0;
    nuevo_tcb->registro->DX = 0;
    nuevo_tcb->registro->EX = 0;
    nuevo_tcb->registro->FX = 0;
    nuevo_tcb->registro->GX = 0;
    nuevo_tcb->registro->HX = 0;


	list_add(lista_tcb, nuevo_tcb);
    log_info(memoria_logger, "TCB agregado a la lista global de TCBs: PID %d, TID %d", nuevo_tcb->pid, nuevo_tcb->tid);

	int valor_a_enviar = 0;
	send(fd_kernel, &valor_a_enviar, sizeof(valor_a_enviar), 0);

	//free(enviar_contexto);
    destruir_buffer_paquete(enviar_contexto);
}
