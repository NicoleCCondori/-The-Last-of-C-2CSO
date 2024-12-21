#include <escuchar_kernel.h>

void escuchar_kernel(){
    printf("Ejecuto escuchar_kernel.c \n");

    while (1){
		t_paquete* paquete_kernel = recibir_paquete(fd_kernel);//aca
        if(!paquete_kernel){
            log_error(memoria_logger,"ERROR en recibir paquete linea 9 escucha memoria");
            return;
        }
        op_code codigo_operacion = paquete_kernel->codigo_operacion;
        log_info(memoria_logger, "Me llega el siguiente op code %u", codigo_operacion);

		switch (codigo_operacion)
		{
		case ASIGNAR_MEMORIA:
            pthread_t thread_asignar_memoria;
            pthread_create(&thread_asignar_memoria, NULL, (void*)crear_proceso, (void*)paquete_kernel);
            pthread_join(thread_asignar_memoria, NULL);
            break;

        case FINALIZAR_PROCESO:
            pthread_t thread_finalizar_proceso;
            pthread_create(&thread_finalizar_proceso, NULL, (void*)finalizar_proceso, (void*)paquete_kernel);
            pthread_join(thread_finalizar_proceso, NULL);
			break;

		case HILO_READY:
            pthread_t thread_ready;
            pthread_create(&thread_ready, NULL, (void*)crear_hilo, (void*)paquete_kernel);
            pthread_join(thread_ready,NULL);

			break;
        
        case DUMP_MEMORY:
            pthread_t thread_dump_memory;
            pthread_create(&thread_dump_memory, NULL, (void*)envio_datos_a_FS, (void*)paquete_kernel);
            pthread_join(thread_dump_memory,NULL);

			break;

        case FINALIZAR_HILO:
            pthread_t thread_finalizar_hilo;
            pthread_create(&thread_finalizar_hilo, NULL, (void*)finalizar_hilo, (void*)paquete_kernel);
            pthread_join(thread_finalizar_hilo,NULL);

            break;

		default:
			log_warning(memoria_logger, "Operacion desconocida de KERNEL");
			break;
		}
        if(paquete_kernel){
          eliminar_paquete(paquete_kernel);  
        }
         // Cerrar la conexión con el kernel

	}
}

/* CREACION DE UN PROCESO - ASIGNO MEMORIA*/
void crear_proceso(void* arg){
    t_paquete* paquete_kernel = (t_paquete*)arg;
    t_asignar_memoria* datos_asignar_memoria = deserializar_asignar_memoria((paquete_kernel));
    if (strcmp(valores_config_memoria->esquema, "FIJAS")==0){
        log_info(memoria_logger, "Asignando memoria con particiones fijas");
        asignar_particiones_fijas(datos_asignar_memoria);
    }
    else if (strcmp(valores_config_memoria->esquema, "DINAMICAS")==0){
        log_info(memoria_logger, "Asignando memoria con particiones dinamicas");
        asignar_particiones_dinamicas(datos_asignar_memoria);
    }
    free(datos_asignar_memoria);
    pthread_exit(NULL);

}

void asignar_particiones_fijas(t_asignar_memoria* datos_asignar_memoria){
    uint32_t bit_confirmacion = -1;
    Particion* particion_asignada = evaluarParticion(datos_asignar_memoria->tam_proceso);

    if (particion_asignada != NULL) {
        particion_asignada->libre = false;
        particion_asignada->pid = datos_asignar_memoria->pid;
        //log_info(memoria_logger,"El pid a asiganar particiones fijas es: %u", datos_asignar_memoria->pid);
	    bit_confirmacion = 1;
        log_info(memoria_log_obligatorios, "## Proceso creado: PID=%u, Tamanio=%u",
        particion_asignada->pid, particion_asignada->tamanio);  
    }
        
        log_info(memoria_logger, "## Proceso creado: base=%u, limite=%u",
        particion_asignada->base, particion_asignada->limite);

       /*  control_key_kernel_memoria = -1;
       send(fd_kernel, &bit_confirmacion, sizeof(uint32_t), 0);*/
       
        t_paquete* paquete_memoria = crear_paquete(CONFIRMAR_ESPACIO_PROCESO);
        serializar_proceso_memoria(paquete_memoria, datos_asignar_memoria->pid, bit_confirmacion);
        enviar_paquete(paquete_memoria, fd_kernel);
        log_info(memoria_logger, "Ya se envio el paquete a kernel, con PID:%u",datos_asignar_memoria->pid );
        eliminar_paquete(paquete_memoria);
        log_info(memoria_logger, "Ya se elimino el paquete de kernel");
}

void asignar_particiones_dinamicas(t_asignar_memoria* datos_asignar_memoria){
    uint32_t bit_confirmacion = -1;
    Particion* particion_asignada = evaluarParticion(datos_asignar_memoria->tam_proceso);

    if (particion_asignada != NULL) {
        particion_asignada->libre = false;
        particion_asignada->pid = datos_asignar_memoria->pid;

        if (particion_asignada->tamanio > datos_asignar_memoria->tam_proceso) {
            dividir_particion(particion_asignada, datos_asignar_memoria->tam_proceso);
            bit_confirmacion = 1;
            log_info(memoria_log_obligatorios, "## Proceso creado: PID=%u, Tamanio=%u",
            particion_asignada->pid, particion_asignada->tamanio);
            }
    //log_info(memoria_logger,"El PID es %u",particion_asignada->pid);
    t_paquete* paquete = crear_paquete(CONFIRMAR_ESPACIO_PROCESO);
    serializar_proceso_memoria(paquete, datos_asignar_memoria->pid, bit_confirmacion);
    printf("Paquete serializado: bit_confirmacion=%d, pid=%u, tamanio=%d\n", bit_confirmacion, datos_asignar_memoria->pid, paquete->buffer->size);
     enviar_paquete(paquete, fd_kernel);
     log_info(memoria_logger, "Ya se envio el paquete a kernel, con PID:%u",datos_asignar_memoria->pid );
     eliminar_paquete(paquete);
     log_info(memoria_logger, "Ya se elimino el paquete de kernel");
}
}

void dividir_particion(Particion* particion, uint32_t tamanio_proceso) {
    uint32_t espacio_restante = particion->tamanio - tamanio_proceso;
    particion->tamanio = tamanio_proceso;
    particion->libre = false;
    particion->limite = particion->base + tamanio_proceso - 1;

    Particion* nueva_particion_libre = malloc(sizeof(Particion));
    nueva_particion_libre->tamanio = espacio_restante;
    nueva_particion_libre->libre = true;
    nueva_particion_libre->pid = 0;
    nueva_particion_libre->base = particion->base + tamanio_proceso;
    nueva_particion_libre->limite = nueva_particion_libre->base + espacio_restante - 1;

    list_add(lista_particiones, nueva_particion_libre);
    log_info(memoria_logger, "Partición dividida: base=%u, limite=%u (original), base=%u, limite=%u (nueva)",
             particion->base, particion->limite, nueva_particion_libre->base, nueva_particion_libre->limite);
}

Particion* evaluarParticion(int tamanio){
    Particion* particion_asignada = NULL;

    if (strcmp(valores_config_memoria->algoritmo_busqueda, "FIRST") == 0) {
        particion_asignada = algoritmo_first_fit(tamanio);
        return particion_asignada;

    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "BEST") == 0) {
        particion_asignada = algoritmo_best_fit(tamanio);
        return particion_asignada;
        
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "WORST") == 0) {
        particion_asignada = algoritmo_worst_fit(tamanio);
        return particion_asignada;
    }

    return particion_asignada;
}

Particion* algoritmo_first_fit(uint32_t tamanio_proceso) {
    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);
        log_info(memoria_logger, "Partición %d - Libre: %d, Tamaño: %u\n,Base: %u\n,Limite: %u\n ", i, particion->libre, particion->tamanio, particion->base, particion->limite);
        if (particion->libre && particion->tamanio >= tamanio_proceso) {
            log_info(memoria_logger, "Retorna Partición -> Base:%d y Limite:%d " ,particion->base,particion->limite);
            return particion;
        }
    }
    return NULL;
}

Particion* algoritmo_best_fit(uint32_t tamanio_proceso) {
    Particion* particion_elegida = NULL;

    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);
        if (particion->libre && particion->tamanio >= tamanio_proceso) {
            if (particion_elegida == NULL || particion->tamanio < particion_elegida->tamanio) {
                particion_elegida = particion;
            }
        }
    }
    return particion_elegida;
}

Particion* algoritmo_worst_fit(uint32_t tamanio_proceso) {
    Particion* particion_elegida = NULL;

    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);
        if (particion->libre && particion->tamanio >= tamanio_proceso) {
            if (particion_elegida == NULL || particion->tamanio > particion_elegida->tamanio) {
                particion_elegida = particion;
            }
        }
    }
    return particion_elegida;
}

/*FINALIZAR PROCESO*/

void finalizar_proceso(void* arg){
    t_paquete* paquete_kernel = (t_paquete*)arg;
    t_enviar_contexto* finalizar_proceso = deserializar_enviar_contexto(paquete_kernel);

    bool encontrado = false;
    int bit_confirmacion = 1;

    // Recorremos la lista de particiones
    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);

        if (particion->pid == finalizar_proceso->PID) {
            particion->libre = true;
            particion->pid = 0; //no está asociado a ningun proceso
            encontrado = true;

            if (strcmp(valores_config_memoria->esquema, "DINAMICAS")==0){
                consolidar_particiones_libres(i);
            }

            log_info(memoria_log_obligatorios, "## Proceso destruido: PID=%u, Tamanio=%u",
            finalizar_proceso->PID, particion->tamanio);
            break;
        }
    }

    if (!encontrado) {
        log_warning(memoria_logger, "No se encontró partición para PID %d", finalizar_proceso->PID);
        bit_confirmacion = -1;
    }

   /* t_paquete* paquete_memoria = crear_paquete(CONFIRMAR_FINALIZAR_PROCESO);
    serializar_proceso_memoria(paquete_memoria, finalizar_proceso->PID, bit_confirmacion);
    enviar_paquete(paquete_memoria, fd_kernel);
    log_info(memoria_logger, "Ya se envio el paquete a kernel");
    eliminar_paquete(paquete_memoria);
    log_info(memoria_logger, "Ya se elimino el paquete de kernel");*/
    control_key_kernel_memoria = -1;
    send(fd_kernel, &bit_confirmacion, sizeof(uint32_t), 0);
    pthread_exit(NULL);

}

Particion* buscar_particion_por_pid(uint32_t pid_buscado) {
    pthread_mutex_lock(&mutex_lista_particiones);
    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion_actual = list_get(lista_particiones, i);
        if (particion_actual->pid == pid_buscado) {
            pthread_mutex_unlock(&mutex_lista_particiones);
            return particion_actual;
        }
    }
    pthread_mutex_unlock(&mutex_lista_particiones);
    return NULL;
}


void consolidar_particiones_libres(int indice) {
    Particion* actual = list_get(lista_particiones, indice);

    // Valido la particion siguiente
    if (indice + 1 < list_size(lista_particiones)) {
        Particion* siguiente = list_get(lista_particiones, indice + 1);
        if (siguiente->libre) {
            actual->tamanio += siguiente->tamanio;  // Sumo el tamaño
            list_remove(lista_particiones, indice + 1);  // Elimino la partición siguiente
            free(siguiente);
        }
    }

    // Valido la particion anterior
    if (indice > 0) {
        Particion* anterior = list_get(lista_particiones, indice - 1);
        if (anterior->libre) {
            anterior->tamanio += actual->tamanio;  // Sumo el tamaño
            list_remove(lista_particiones, indice);  // Elimino la partición actual
            free(actual);
        }
    }

}

/* CREACION DE UN HILO - ASIGNO MEMORIA*/
void crear_hilo(void* arg){
    t_paquete* paquete_kernel = (t_paquete*)arg;
    uint32_t bit_confirmacion = -1;
    t_crear_hilo* datos_hilo = deserializar_crear_hilo(paquete_kernel);

    log_info(memoria_logger, "Estoy creando hilo:%u para PID:%u", datos_hilo->TID, datos_hilo->PID);

    ContextoEjecucion* nuevo_contexto = malloc(sizeof(ContextoEjecucion));
    nuevo_contexto->pid = datos_hilo->PID;
    nuevo_contexto->tid = datos_hilo->TID;

    Particion* particion = buscar_particion_por_pid(datos_hilo->PID);

    nuevo_contexto->base = particion->base;
    nuevo_contexto->limite = particion->limite;
    memset(&(nuevo_contexto->registros), 0, sizeof(RegistrosCPU));
    nuevo_contexto->pc = 0;
    nuevo_contexto-> instrucciones = strdup(datos_hilo->path);
    nuevo_contexto-> prioridad = datos_hilo->prioridad;

    list_add(lista_contextos, nuevo_contexto);
	bit_confirmacion = 1;
    log_info(memoria_log_obligatorios, "## Hilo creado: PID=%u, TID=%u",
            nuevo_contexto->pid, nuevo_contexto->tid);

    t_paquete* paquete = crear_paquete(CONFIRMAR_CREACION_HILO);
    serializar_hilo_memoria(paquete, bit_confirmacion, nuevo_contexto->pid, nuevo_contexto->tid);
    enviar_paquete(paquete, fd_kernel);
    log_info(memoria_logger, "Se envio el paquete a Kernel");
    eliminar_paquete(paquete);
    log_info(memoria_logger,"Se elimino el paquete a Kernel");
    free(datos_hilo->path);  // Liberar la cadena 'path'
    free(datos_hilo); 
    pthread_exit(NULL);       // Liberar la estructura
}

/*Finalización de hilos*/
void finalizar_hilo(void* arg){
    t_paquete* paquete_kernel = (t_paquete*)arg;
    int bit_confirmacion = -1;  // Inicializamos con -1, que significa "no encontrado"
    t_enviar_contexto* datos_finalizar_hilo = deserializar_enviar_contexto(paquete_kernel);

    uint32_t tid_a_eliminar = datos_finalizar_hilo->TID;
    uint32_t pid_a_eliminar = datos_finalizar_hilo->PID;

    log_info(memoria_logger, "Recibe para eliminar el tid %u, con pid %u", tid_a_eliminar, pid_a_eliminar);
    ContextoEjecucion* contexto_eliminado = NULL;
    
    // Recorrer la lista de contextos
    for (int i = 0; i < list_size(lista_contextos); i++) {
        ContextoEjecucion* contexto = list_get(lista_contextos, i);
        log_info(memoria_logger,"TID buscado %u, TID actual %u", tid_a_eliminar, contexto->tid);

        if (contexto->tid == tid_a_eliminar && contexto->pid == pid_a_eliminar) {
            // Eliminar el contexto de la lista
            contexto_eliminado = list_remove(lista_contextos, i);

            log_info(memoria_logger, "Contexto con TID %u eliminado exitosamente.", tid_a_eliminar);
            log_info(memoria_log_obligatorios, "## Hilo destruido: PID=%u, TID=%u", pid_a_eliminar, tid_a_eliminar);
            
            bit_confirmacion = 1;  // Confirmación positiva de que se encontró y eliminó el contexto
            break;  // Salir del ciclo después de eliminar el contexto
        }
    }

    // Verificación si el contexto no fue encontrado
    if (bit_confirmacion == -1) {
        log_info(memoria_logger, "No se encontró el contexto con TID %u y PID %u para eliminar.", tid_a_eliminar, pid_a_eliminar);
    }

    // Liberar los recursos
    free(datos_finalizar_hilo);
    if (contexto_eliminado != NULL) {
        free(contexto_eliminado);  // Solo liberar si el contexto fue eliminado de la lista
    }

    // Enviar la confirmación al kernel
    control_key_kernel_memoria = -1;
    send(fd_kernel, &bit_confirmacion, sizeof(uint32_t), 0);
    log_info(memoria_logger, "Ya envie bit de confirmacion %u", bit_confirmacion);
    
    pthread_exit(NULL);
}

/*DUMP_MEMORY
En caso de que el FileSystem responda con error, se devolverá el mismo mensaje al Kernel, en caso positivo, se responde como OK.
*/

t_datos_esenciales* deserializar_datos_dump_memory(t_paquete* paq_dump_memory){
	t_datos_esenciales* datos_dm = malloc(sizeof(t_datos_esenciales));

	datos_dm->pid_inv = leer_buffer_Uint32(paq_dump_memory->buffer);
	datos_dm->tid_inv = leer_buffer_Uint32(paq_dump_memory->buffer);
	return datos_dm;
}

void envio_datos_a_FS(void* arg){
    t_paquete* paquete_kernel = (t_paquete*)arg;
    int valor_a_enviar;
    t_datos_esenciales* datos_dm = deserializar_datos_dump_memory(paquete_kernel);

    Particion* particion= buscar_particion_por_pid(datos_dm->pid_inv);
    uint32_t base = particion->base;
    uint32_t tamanio = particion->tamanio;

    void* datos = malloc(tamanio);

    pthread_mutex_lock(&mutex_memoria);
    memcpy(datos, (char*)memoria + base, tamanio);
    pthread_mutex_unlock(&mutex_memoria);

    char nombre[256];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    snprintf(nombre, sizeof(nombre), "%d-%d-%04d-%02d-%02d_%02d-%02d-%02d.dmp", 
            datos_dm->pid_inv, datos_dm->tid_inv, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    log_info(memoria_logger, "Nombre de archivo: %s\n", nombre);

    t_paquete* paquete_dump_memory = crear_paquete(DUMP_MEMORY);
    //serializar_enviar_DUMP_MEMORY(paquete_dump_memory, datos, tamanio, nombre);
    enviar_paquete(paquete_dump_memory, fd_FS);
    eliminar_paquete(paquete_dump_memory);

   /*En caso de que el FileSystem responda con error, se devolverá el mismo mensaje al Kernel,
    en caso positivo, se responde como OK.*/
      
    recv(fd_FS, &valor_a_enviar, sizeof(int), 0);
    send(fd_kernel, &valor_a_enviar, sizeof(valor_a_enviar), 0);

    free(datos);
    pthread_exit(NULL);
}