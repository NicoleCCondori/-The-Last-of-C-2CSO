#include <escuchar_kernel.h>


void escuchar_kernel(){
    printf("Ejecuto escuchar_kernel.c \n");

    while (1){
		t_paquete* paquete_kernel = recibir_paquete(fd_kernel);
        op_code codigo_operacion = paquete_kernel->codigo_operacion;
		switch (codigo_operacion)
		{
		case ASIGNAR_MEMORIA:
            crear_proceso(paquete_kernel);
            break;

        case FINALIZAR_PROCESO:
		    finalizar_proceso(paquete_kernel);
			break;

		case HILO_READY:
		    crear_hilo(paquete_kernel);
			break;
        
        case DUMP_MEMORY:
		    envio_datos_a_FS(paquete_kernel);
			break;

		default:
			log_warning(memoria_logger, "Operacion desconocida de KERNEL");
			break;
		}
		eliminar_paquete(paquete_kernel);
	}
}

/* CREACION DE UN PROCESO - ASIGNO MEMORIA*/
void crear_proceso(t_paquete* paquete_kernel){
    t_asignar_memoria* datos_asignar_memoria = deserializar_asignar_memoria((paquete_kernel));
    if (strcmp(valores_config_memoria->esquema, "FIJAS")==0){
        asignar_particiones_fijas(datos_asignar_memoria);
    }
    else if (strcmp(valores_config_memoria->esquema, "DINAMICAS")==0){
        asignar_particiones_dinamicas(datos_asignar_memoria);
    }
}

void asignar_particiones_fijas(t_asignar_memoria* datos_asignar_memoria){
    int valor_a_enviar = -1;
    Particion* particion_asignada = evaluarParticion(datos_asignar_memoria->tam_proceso);

    if (particion_asignada != NULL) {
        particion_asignada->libre = false;
        particion_asignada->pid = datos_asignar_memoria->pid;

	    valor_a_enviar = 0;
        log_info(memoria_log_obligatorios, "Proceso creado: PID=%u, Base=%u, Limite=%u",
         particion_asignada->pid, particion_asignada->base, particion_asignada->limite);
        }
    
    send(fd_kernel, &valor_a_enviar, sizeof(int), 0);
}

void asignar_particiones_dinamicas(t_asignar_memoria* datos_asignar_memoria){
    int valor_a_enviar = -1;
    Particion* particion_asignada = evaluarParticion(datos_asignar_memoria->tam_proceso);

    if (particion_asignada != NULL) {
        particion_asignada->libre = false;
        particion_asignada->pid = datos_asignar_memoria->pid;

        if (particion_asignada->tamanio > datos_asignar_memoria->tam_proceso) {
            dividir_particion(particion_asignada, datos_asignar_memoria->tam_proceso);
            valor_a_enviar = 0;
            log_info(memoria_log_obligatorios, "Proceso creado: PID=%u, Base=%u, Limite=%u",
             particion_asignada->pid, particion_asignada->base, particion_asignada->limite);
            }
    
    send(fd_kernel, &valor_a_enviar, sizeof(int), 0);
}
}
void dividir_particion(Particion* particion, uint32_t tamanio_proceso) {
    uint32_t espacio_restante = particion->tamanio - tamanio_proceso;
    particion->tamanio = tamanio_proceso;

    Particion* nueva_particion_libre = malloc(sizeof(Particion));
    nueva_particion_libre->tamanio = espacio_restante;
    nueva_particion_libre->libre = true;
    nueva_particion_libre->pid = 0;
    /*nueva_particion_libre = particion->direccion + tamanio_proceso;*/

    list_add(lista_particiones, nueva_particion_libre);
}

Particion* evaluarParticion(int tamanio){
    Particion* particion_asignada = NULL;

    if (strcmp(valores_config_memoria->algoritmo_busqueda, "FIRST") == 0) {
        particion_asignada = algoritmo_first_fit(tamanio);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "BEST") == 0) {
        particion_asignada = algoritmo_best_fit(tamanio);
    } else if (strcmp(valores_config_memoria->algoritmo_busqueda, "WORST") == 0) {
        particion_asignada = algoritmo_worst_fit(tamanio);
    }

    return particion_asignada;
}

Particion* algoritmo_first_fit(uint32_t tamanio_proceso) {
    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);
        if (particion->libre && particion->tamanio >= tamanio_proceso) {
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

void finalizar_proceso(t_paquete* paquete_kernel){
    uint32_t pid = leer_buffer_Uint32(paquete_kernel->buffer);
    bool encontrado = false;

    // Recorremos la lista de particiones
    for (int i = 0; i < list_size(lista_particiones); i++) {
        Particion* particion = list_get(lista_particiones, i);

        if (particion->pid == pid) {
            particion->libre = true;
            particion->pid = 0; //no está asociado a ningun proceso
            encontrado = true;

            if (strcmp(valores_config_memoria->esquema, "DINAMICAS")==0){
                consolidar_particiones_libres(i);
            }
            
            log_info(memoria_logger, "## Proceso Destruido - PID: %u - Tamaño: %u", pid, particion->tamanio);
            break;
        }
    }

    if (!encontrado) {
        log_warning(memoria_logger, "No se encontró partición para PID %d", pid);
    }
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
void crear_hilo(t_paquete* paquete_kernel){
    t_crear_hilo* datos_hilo = deserializar_crear_hilo(paquete_kernel);

    ContextoEjecucion* nuevo_contexto = malloc(sizeof(ContextoEjecucion));
    nuevo_contexto->pid = datos_hilo->PID;
    nuevo_contexto->tid = datos_hilo->TID;

    Particion* particion = buscar_particion_por_pid(datos_hilo->TID);

    nuevo_contexto->base = particion->base;
    nuevo_contexto->limite = particion->limite;
    memset(&(nuevo_contexto->registros), 0, sizeof(RegistrosCPU));
    nuevo_contexto->pc = 0;
    nuevo_contexto-> instrucciones = datos_hilo->path;
    nuevo_contexto-> prioridad = datos_hilo->prioridad;

    list_add(lista_contextos, nuevo_contexto);

	int valor_a_enviar = 0;
	send(fd_kernel, &valor_a_enviar, sizeof(valor_a_enviar), 0);
}

/*Finalización de hilos
Al momento de finalizar un hilo, el Kernel deberá informar a la Memoria la finalización del mismo, liberar su TCB asociado y deberá mover al estado READY a todos los hilos que se encontraban bloqueados por ese TID. De esta manera, se desbloquean aquellos hilos bloqueados por THREAD_JOIN o por mutex tomados por el hilo finalizado (en caso que hubiera).
*/

/*DUMP_MEMORY
Memory Dump
Memoria deberá solicitar al módulo FileSystem 
la creación de un nuevo archivo con el tamaño total de la memoria reservada por el proceso 
y debe escribir en dicho archivo todo el contenido actual de la memoria del proceso. 
El archivo debe llamarse “<PID>-<TID>-<TIMESTAMP>.dmp”.
En caso de que el FileSystem responda con error, se devolverá el mismo mensaje al Kernel, en caso positivo, se responde como OK.
*/

t_datos_esenciales* deserializar_datos_dump_memory(t_paquete* paq_dump_memory){
	t_datos_esenciales* datos_dm = malloc(sizeof(t_datos_esenciales));

	datos_dm->pid_inv = leer_buffer_Uint32(paq_dump_memory->buffer);
	datos_dm->tid_inv = leer_buffer_Uint32(paq_dump_memory->buffer);
	return datos_dm;
}

void envio_datos_a_FS(t_paquete* paquete_kernel){
   /* int valor_a_enviar = -1;
    t_datos_esenciales* datos_dm = deserializar_datos_dump_memory(paquete_kernel);

    

    
    send(fd_FS, &datos_dm, sizeof(datos_dm), 0);
    recv(fd_FS, &valor_a_enviar, sizeof(valor_a_enviar), 0);
    if(valor_a_enviar == 0){
        valor_a_enviar = 0;
    }
    send(fd_FS, &valor_a_enviar, sizeof(valor_a_enviar), 0);*/
}