#include <escuchar_kernel.h>


void escuchar_kernel(){
    printf("Ejecuto escuchar_kernel.c \n");

    while (1){
		t_paquete* paquete_kernel = recibir_paquete(fd_kernel);
        op_code codigo_operacion = paquete_kernel->codigo_operacion;
		switch (codigo_operacion)
		{
		case ASIGNAR_MEMORIA:
            //crear_proceso(paquete_kernel);
            break;

		case HILO_READY:
		    // crear_hilo(paquete_kernel);
			break;
        
        case DUMP_MEMORY:
		     //envio_datos_a_FS(paquete_kernel);
			break;

		default:
			log_warning(memoria_logger, "Operacion desconocida de KERNEL");
			break;
		}
		eliminar_paquete(paquete_kernel);
	}
}

/*
2. Particiones Dinámicas: la misma se va a ir subdividiendo a medida que lleguen los pedidos 
de creación de los procesos, es por esto que la lista será dinámica.

En ambos esquemas, el hueco a asignar y/o fraccionar se deberá elegir utilizando alguna de las siguientes estrategias:
●	First Fit
●	Best Fit
●	Worst Fit
En caso de encontrar un hueco libre, se le asignará la partición, se creará la estructura necesaria para administrar la Memoria de Sistema, y se responderá como OK al Kernel.
En caso de no encontrar un hueco libre, se le responderá al Kernel que el proceso no pudo ser inicializado. Para este trabajo práctico, no debe realizarse el proceso de compactación bajo ningún caso.

*/
/*

void crear_proceso(t_paquete* paquete_kernel){
    if (strcmp(valores_config_memoria->esquema, "FIJAS")==0){
        asignar_particiones_fijas();
    }
    else if (strcmp(valores_config_memoria->esquema, "DINAMICAS")==0){
        asignar_particiones_dinamicas();
    }
}

void asignar_particiones_fijas(uint32_t tamanio_proceso, uint32_t pid){
    t_list* particiones = list_create();
      for (int i = 0; valores_config_memoria->particiones[i] != NULL; i++) {

        uint32_t tamanio = atoi(valores_config_memoria->particiones[i]);

        particion_t* particion = malloc(sizeof(particion_t));
        particion->id = i;
        particion->tamanio = tamanio;
        particion->ocupada = false;
        particion->pid = 0;

        list_add(particiones, particion);
    }

    particion_t* particion_asignada = NULL;

    if (strcmp(valores_config_memoria->algoritmo_busqueda, "FIRST") == 0) {
        particion_asignada = algoritmo_first_fit(tamanio_proceso);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "BEST") == 0) {
        particion_asignada = algoritmo_best_fit(tamanio_proceso);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "WORST") == 0) {
        particion_asignada = algoritmo_worst_fit(tamanio_proceso);
    }

    if (particion_asignada != NULL) {
        particion_asignada->ocupada = true;
        particion_asignada->pid = pid;

	    send(fd_kernel, &valor_a_enviar, sizeof(int), 0);}
    
    else send(fd_kernel, &valor_a_enviar, sizeof(int), -1);
}

void asignar_particiones_dinamicas(uint32_t tamanio_proceso, uint32_t pid){
    particion_t* particion_asignada = NULL;

    if (strcmp(valores_config_memoria->algoritmo_busqueda, "FIRST") == 0) {
        particion_asignada = algoritmo_first_fit(tamanio_proceso);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "BEST") == 0) {
        particion_asignada = algoritmo_best_fit(tamanio_proceso);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "WORST") == 0) {
        particion_asignada = algoritmo_worst_fit(tamanio_proceso);
    }

    if (particion_asignada != NULL) {
        particion_asignada->ocupada = true;
        particion_asignada->pid = pid;

        if (particion_asignada->tamanio > tamanio_proceso) {
            dividir_particion(particion_asignada, tamanio_proceso);
            send(fd_kernel, &valor_a_enviar, sizeof(int), 0);}
    
    else send(fd_kernel, &valor_a_enviar, sizeof(int), -1);
}
}
void dividir_particion(particion_t* particion, uint32_t tamanio_proceso) {
    uint32_t espacio_restante = particion->tamanio - tamanio_proceso;
    particion->tamanio = tamanio_proceso;

    particion_t* nueva_particion_libre = malloc(sizeof(particion_t));
    nueva_particion_libre->tamanio = espacio_restante;
    nueva_particion_libre->ocupada = false;
    nueva_particion_libre->pid = 0;
    nueva_particion_libre->direccion = particion->direccion + tamanio_proceso;

    list_add(particiones_libres, nueva_particion_libre);
}


particion_t* algoritmo_first_fit(uint32_t tamanio_proceso) {
    for (int i = 0; i < list_size(particiones_libres); i++) {
        particion_t* particion = list_get(particiones_libres, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso) {
            return particion;
        }
    }
    return NULL;
}

particion_t* algoritmo_best_fit(uint32_t tamanio_proceso) {
    particion_t* particion_elegida = NULL;

    for (int i = 0; i < list_size(particiones_libres); i++) {
        particion_t* particion = list_get(particiones_libres, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso) {
            if (particion_elegida == NULL || particion->tamanio < particion_elegida->tamanio) {
                particion_elegida = particion;
            }
        }
    }
    return particion_elegida;
}

particion_t* algoritmo_worst_fit(uint32_t tamanio_proceso) {
    particion_t* particion_elegida = NULL;

    for (int i = 0; i < list_size(particiones_libres); i++) {
        particion_t* particion = list_get(particiones_libres, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso) {
            if (particion_elegida == NULL || particion->tamanio > particion_elegida->tamanio) {
                particion_elegida = particion;
            }
        }
    }
    return particion_elegida;
}

void crear_hilo(t_paquete* paquete_kernel){
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
*/