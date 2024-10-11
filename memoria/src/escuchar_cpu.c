#include <escuchar_cpu.h>
#include <m_conexiones.h>

void escuchar_cpu(){
    //atender los msjs de cpu-dispatch , otra funcion?
    printf("Ejecuto memoria_escucha_cpu.c \n");

    while (1){
        t_paquete* paquete_cpu = recibir_paquete(fd_cpu);
        op_code codigo_operacion = paquete_cpu->codigo_operacion;
		switch (codigo_operacion)
		{
		case OBTENER_CONTEXTO:
			devolver_contexto_ejecucion();

		case ACTUALIZAR_CONTEXTO:
			actualizar_contexto_ejecucion();

		case OBTENER_INSTRUCCION:
			obtener_instruccion();

		default:
			log_warning(memoria_logger, "Operacion desconocida de CPU");
			break;
		}
      eliminar_paquete(paquete_cpu);      
	}
}

void devolver_contexto_ejecucion() {
    t_paquete* paquete_contexto = recibir_paquete(fd_kernel);
    t_enviar_contexto* datos_contexto = deserializar_enviar_contexto(paquete_contexto);
    
    TCB* hilo = buscar_tcb_por_tid(datos_contexto->TID);
    if (hilo == NULL) {
        log_error(memoria_logger, "Hilo no encontrado");
        return NULL;
    }

    if (hilo->pid ==! datos_contexto->PID) {
            log_error(memoria_logger, "El proceso no corresponde");
    }

    // Crear y llenar el contexto de ejecución
    t_ContextoEjecucion* contexto = malloc(sizeof(t_ContextoEjecucion));
    if (contexto == NULL) {
        log_error(memoria_logger, "Error al asignar memoria para el contexto");
        return NULL;
    }

    // Copiar los registros y valores
    contexto->registrosCPU.AX = hilo->registro->AX;
    contexto->registrosCPU.BX = hilo->registro->BX;
    contexto->registrosCPU.CX = hilo->registro->CX;
    contexto->registrosCPU.DX = hilo->registro->DX;
    contexto->registrosCPU.EX= hilo->registro->EX;
    contexto->registrosCPU.FX = hilo->registro->FX;
    contexto->registrosCPU.GX = hilo->registro->GX;
    contexto->registrosCPU.HX = hilo->registro->HX;

    contexto->PC = hilo->pc;

    contexto->base = 0;
    contexto->limite = 0;

    
    t_paquete* paquete_enviar_contexto = crear_paquete(ENVIAR_CONTEXTO);
    serializar_enviar_contexto(paquete_enviar_contexto, contexto);
    enviar_paquete(paquete_enviar_contexto, fd_cpu);
    eliminar_paquete(paquete_enviar_contexto);

    return 0;
}

TCB* buscar_tcb_por_tid(uint32_t tid_buscado) {
    for (int i = 0; i < list_size(lista_tcb); i++) {
        TCB* tcb_actual = list_get(lista_tcb, i);

        if (tcb_actual->tid == tid_buscado) {
            return tcb_actual;
        }
    }
    return 0; 
}

//uint32_t calcular_base(PCB* proceso)

/*

int actualizar_registros_cpu(uint32_t pid, uint32_t tid, RegistrosCPU* nuevos_registros, uint32_t nuevo_pc) {
    t_paquete* paquete_contexto = recibir_paquete(fd_kernel);
    t_enviar_contexto* datos_contexto = t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete);
    
    
    PCB* proceso = buscar_proceso_por_pid(pid);
    if (proceso == NULL) {
        log_error(memoria_logger, "Proceso con PID %d no encontrado", pid);
        return -1;  // Error, proceso no encontrado
    }

    // Buscar el TCB por TID dentro del proceso
    TCB* hilo = buscar_hilo_por_tid(proceso, tid);
    if (hilo == NULL) {
        log_error(memoria_logger, "Hilo con TID %d no encontrado en el proceso con PID %d", tid, pid);
        return -1;  // Error, hilo no encontrado
    }

    // Actualizar los registros del hilo
    hilo->registro->AX = nuevos_registros->AX;
    hilo->registro->BX = nuevos_registros->BX;
    hilo->registro->CX = nuevos_registros->CX;
    hilo->registro->DX = nuevos_registros->DX;
    hilo->registro->EX = nuevos_registros->EX;
    hilo->registro->FX = nuevos_registros->FX;
    hilo->registro->GX = nuevos_registros->GX;
    hilo->registro->HX = nuevos_registros->HX;

    // Actualizar el Program Counter (PC)
    proceso->pc = nuevo_pc;

    log_info(memoria_logger, "Registros y PC actualizados para TID %d del PID %d", tid, pid);

    return 0;  
    }*/



char* obtener_instruccion() {
    t_paquete* paquete_instruccion = recibir_paquete(fd_kernel);
    t_obtener_instruccion* datos_instruccion = deserializar_obtener_instruccion(paquete_instruccion);
    
    TCB* hilo = buscar_tcb_por_tid(datos_instruccion->TID);
    if (hilo == NULL) {
        log_error(memoria_logger, "Hilo no encontrado");
        return NULL;
    }

    char* path_instrucciones = valores_config_memoria->path_instrucciones;
    char* path = hilo->path;
    
    int path1 = strlen(path_instrucciones);
    int path2 = strlen(path);

    char* path_completo = malloc(path1 + path2 + 2); // '/' y '\0'

    strcpy(path_completo, path_instrucciones);

    // Verificar que tenga "/" para que funcione
    if (path_completo[path1 - 1] != '/') {
        strcat(path_completo, "/");
    }
    strcat(path_completo, path);

    log_info(memoria_logger, "EL path completo es el siguiente: %s", path_completo);

    FILE *archivo = fopen(path_completo, "r");
    if (archivo == NULL) {
        log_error(memoria_logger, "Error al abrir el archivo");
        free(path_completo); // Libera la memoria antes de retornar
        return NULL;
    }

    char *linea = NULL;  // Puntero donde se almacenará cada línea
    size_t longitud = 0; // Inicialmente, longitud de 0 para que getline la ajuste dinámicamente
    ssize_t leidos;      // Para almacenar el número de caracteres leídos
    char **lineas = NULL; // Array de punteros para guardar las líneas
    size_t num_lineas = 0; // Número de líneas leídas

    // Leer cada línea del archivo dinámicamente
    while ((leidos = getline(&linea, &longitud, archivo)) != -1) {
        // Eliminar el salto de línea al final de la línea si existe
        linea[strcspn(linea, "\n")] = '\0';

        // Redimensionar el array de punteros para incluir la nueva línea
        lineas = realloc(lineas, sizeof(char *) * (num_lineas + 1));
        if (lineas == NULL) {
            perror("Error al redimensionar memoria");
            free(linea);
            fclose(archivo);
            free(path_completo);
            return NULL;
        }

        // Almacenar la línea actual en el array
        lineas[num_lineas] = strdup(linea); // strdup copia la línea a una nueva ubicación en memoria
        if (lineas[num_lineas] == NULL) {
            perror("Error al duplicar la línea");
            free(linea);
            fclose(archivo);
            free(path_completo);
            return NULL;
        }

        num_lineas++;
    }

    free(linea);
    fclose(archivo);
    free(path_completo);

    // Verificar que el PC sea un valor válido
    if (hilo->pc < 0 || hilo->pc >= num_lineas) {
        log_error(memoria_logger, "El valor de PC proporcionado es inválido.");
        // Liberar la memoria usada por las líneas leídas
        for (size_t i = 0; i < num_lineas; i++) {
            free(lineas[i]);
        }
        free(lineas);
        return NULL;
    }

    // Devolver la instrucción correspondiente al PC
    char *instruccion = strdup(lineas[hilo->pc]);

    // Liberar la memoria usada por las líneas leídas
    for (size_t i = 0; i < num_lineas; i++) {
        free(lineas[i]);
    }
    free(lineas);

    return instruccion;

    t_paquete* paquete_enviar_instruccion = crear_paquete(ENVIAR_INSTRUCCION);
    serializar_enviar_instruccion(paquete_enviar_instruccion, instruccion);
    enviar_paquete(paquete_enviar_instruccion, fd_cpu);
    eliminar_paquete(paquete_enviar_instruccion);

    return 0;
}

void READ_MEM(){
	return "OK";
}


void WRITE_MEM(){
return "OK";
}