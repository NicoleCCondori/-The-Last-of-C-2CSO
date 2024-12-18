#include <escuchar_cpu.h>


void escuchar_cpu(){
    //atender los msjs de cpu-dispatch , otra funcion?
    //printf("Ejecuto memoria_escucha_cpu.c \n");

    while (1){
        t_paquete* paquete_cpu = recibir_paquete(fd_cpu);
        op_code codigo_operacion = paquete_cpu->codigo_operacion;
        log_info(memoria_logger,"Codigo de operacion: %d", codigo_operacion);
        
		switch (codigo_operacion)
		{
		case OBTENER_CONTEXTO:
        
			devolver_contexto_ejecucion(paquete_cpu);
            break;

		case ACTUALIZAR_CONTEXTO:
			actualizar_contexto_de_ejecucion(paquete_cpu);
            break;

		case OBTENER_INSTRUCCION:
			obtener_instruccion(paquete_cpu);
            break;

        case WRITE_MEM:
			write_mem(paquete_cpu);
            break;
        
        case READ_MEM:
			read_mem(paquete_cpu);
            break;

		default:
			log_warning(memoria_logger, "Operacion desconocida de CPU");
			break;
		}
      eliminar_paquete(paquete_cpu);      
	}
}

ContextoEjecucion* buscar_contexto_por_tid(uint32_t tid_buscado) {

    pthread_mutex_lock(&mutex_contextos);
    for (int i = 0; i < list_size(lista_contextos); i++) {
        ContextoEjecucion* contexto_actual = list_get(lista_contextos, i);
        
        printf("Contexto tid: %u y pid: %u\n", contexto_actual->tid, contexto_actual->pid);//

        if (contexto_actual->tid == tid_buscado) {
            pthread_mutex_unlock(&mutex_contextos);
            return contexto_actual;
        }
    }
    pthread_mutex_unlock(&mutex_contextos);
    return NULL;
}

/*DEVOLVER CONTEXTO EJECUCION*/
void devolver_contexto_ejecucion(t_paquete* paquete_cpu) {
    t_enviar_contexto* datos_contexto = deserializar_enviar_contexto(paquete_cpu);
    usleep(atoi(valores_config_memoria->retardo_respuesta) * 1000);
    
    log_info(memoria_logger, "Tengo que buscar con el siguiente TID: %u y pid: %u ", datos_contexto->TID, datos_contexto->PID);
   
    ContextoEjecucion* contexto = buscar_contexto_por_tid(datos_contexto->TID);
    if (contexto == NULL) {
        log_error(memoria_logger, "Hilo no encontrado");
        return;
    }
    else log_info(memoria_logger, "Hilo encontrado");

    if (contexto->pid ==! datos_contexto->PID) {
            log_error(memoria_logger, "El proceso no corresponde");
            return;
    }

    t_contextoEjecucion* nuevo_contexto = malloc(sizeof(t_contextoEjecucion));
    if (nuevo_contexto == NULL) {
        log_error(memoria_logger, "Error al asignar memoria para el contexto");
        exit(EXIT_FAILURE);
    }

    // Copiar los registros y valores
    nuevo_contexto->RegistrosCPU = malloc(sizeof(*nuevo_contexto->RegistrosCPU));
    if (nuevo_contexto->RegistrosCPU == NULL) {
        // Error de asignación de memoria
        log_error(memoria_logger, "Error: No se pudo asignar memoria para RegistrosCPU");
        return;
    }
    *(nuevo_contexto->RegistrosCPU) = contexto->registros;
    nuevo_contexto->PC = contexto->pc;
    nuevo_contexto->base = contexto->base;
    nuevo_contexto->limite = contexto->limite;
    nuevo_contexto->TID = contexto->tid;
    
    t_paquete* paquete_enviar_contexto = crear_paquete(ENVIAR_CONTEXTO);
    serializar_enviar_contexto_cpu(paquete_enviar_contexto, nuevo_contexto);
    enviar_paquete(paquete_enviar_contexto, fd_cpu);

    log_info(memoria_log_obligatorios, "## Contexto Solicitado - (PID:TID) - (<%u>:<%u>)", contexto->pid, contexto->tid);

    eliminar_paquete(paquete_enviar_contexto);
    free(nuevo_contexto->RegistrosCPU);
    free(nuevo_contexto);
}

/*ACTUALIZAR CONTEXTO EJECUCION*/
void actualizar_contexto_de_ejecucion(t_paquete* paquete_cpu) {
    t_actualizar_contexto* datos_contexto = deserializar_actualizar_contexto(paquete_cpu);
    usleep(atoi(valores_config_memoria->retardo_respuesta) * 1000);

    pthread_mutex_lock(&mutex_contextos);
    ContextoEjecucion* contexto = buscar_contexto_por_tid(datos_contexto->TID);
    if (!contexto) {
        log_error(memoria_logger, "Contexto no encontrado para TID %u", datos_contexto->TID);
        pthread_mutex_unlock(&mutex_contextos);
        return;
    }

    contexto->registros = *(datos_contexto->contexto_ejecucion->RegistrosCPU);
    contexto->pc = datos_contexto->contexto_ejecucion->PC;

    log_info(memoria_log_obligatorios, "## Contexto Actualizado - (PID:TID) - (<%u>:<%u>)", contexto->pid, contexto->tid);
    pthread_mutex_unlock(&mutex_contextos);
}

void obtener_instruccion(t_paquete* paquete_cpu) {
    t_obtener_instruccion* datos_instruccion = deserializar_obtener_instruccion(paquete_cpu);
    usleep(atoi(valores_config_memoria->retardo_respuesta) * 1000);

    ContextoEjecucion* contexto = buscar_contexto_por_tid(datos_instruccion->TID);
    if (!contexto) {
        log_error(memoria_logger, "Hilo no encontrado para obtener instrucción");
        return;
    }

    char* instruccion = obtener_instruccion_por_pc(contexto->pc, contexto->instrucciones);
    if (!instruccion) {
        log_error(memoria_logger, "No se pudo obtener la instrucción en PC %u para TID %u", contexto->pc, contexto->tid);
        return;
    }

    log_info(memoria_log_obligatorios, "## Obtener instrucción - (PID:TID) - (<%u>:<%u>) - Instrucción: %s", contexto->pid, contexto->tid, instruccion);

    t_paquete* paquete_instruccion = crear_paquete(ENVIAR_INSTRUCCION);
    serializar_enviar_instruccion(paquete_instruccion, instruccion);
    enviar_paquete(paquete_instruccion, fd_cpu);
    eliminar_paquete(paquete_instruccion);
    free(instruccion);
}

char* obtener_instruccion_por_pc(uint32_t pc, char* path) {

    char* path_instrucciones = valores_config_memoria->path_instrucciones;
    
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
    	exit(EXIT_FAILURE);
    }

    char *linea = NULL;  // Puntero donde se almacenara cada linea
    size_t longitud = 0; // Inicialmente, longitud de 0 para que getline la ajuste dinamicamente
    ssize_t leidos;      // Para almacenar el numero de caracteres leidos
    char **lineas = NULL; // Array de punteros para guardar las lineas
    size_t num_lineas = 0; // Numero de lineas leidas

    // Leer cada linea del archivo dinamicamente
    while ((leidos = getline(&linea, &longitud, archivo)) != -1) {
        // Eliminar el salto de linea al final de la lInea si existe
        linea[strcspn(linea, "\n")] = '\0';

        // Redimensionar el array de punteros para incluir la nueva lInea
        lineas = realloc(lineas, sizeof(char *) * (num_lineas + 1));
        if (lineas == NULL) {
            perror("Error al redimensionar memoria");
            free(linea);
            fclose(archivo);
            free(path_completo);
    	exit(EXIT_FAILURE);
        }

        // Almacenar la lInea actual en el array
        lineas[num_lineas] = strdup(linea); // strdup copia la lInea a una nueva ubicaciOn en memoria
        if (lineas[num_lineas] == NULL) {
            perror("Error al duplicar la lInea");
            free(linea);
            fclose(archivo);
            free(path_completo);
    	exit(EXIT_FAILURE);
        }

        num_lineas++;
    }

    free(linea);
    fclose(archivo);
    free(path_completo);

    // Verificar que el PC sea un valor valido
    if (pc < 0 || pc >= num_lineas) {
        log_error(memoria_logger, "El valor de PC proporcionado es invalido.");
        // Liberar la memoria usada por las líneas leídas
        for (size_t i = 0; i < num_lineas; i++) {
            free(lineas[i]);
        }
        free(lineas);
    	exit(EXIT_FAILURE);
    }

    // Devolver la instrucción correspondiente al PC
    char* instruccion = strdup(lineas[pc]);

    

    // Liberar la memoria usada por las líneas leídas
    for (size_t i = 0; i < num_lineas; i++) {
        free(lineas[i]);
    }
    
    free(lineas);

    return instruccion;
}

void write_mem(t_paquete* paquete_cpu){
    t_datos_write_mem* datos_write_mem = deserializar_write_mem(paquete_cpu);
    usleep(atoi(valores_config_memoria->retardo_respuesta) * 1000);
    
    pthread_mutex_lock(&mutex_memoria);

    if (datos_write_mem->dir_fis + sizeof(uint32_t) < tamanio_memoria) {

        memcpy((char *)memoria + datos_write_mem->dir_fis, &datos_write_mem->valor, sizeof(uint32_t));
    
        uint32_t respuesta_ok = 1;
        send(fd_cpu, &respuesta_ok, sizeof(respuesta_ok), 0);
        
        log_info(memoria_log_obligatorios, "## Escritura - (PID:TID) - (<%u>:<%u>) - Direccion Fisica:%u - Tamanio: <%u>\n",datos_write_mem->pidHilo, datos_write_mem->tidHilo, datos_write_mem->dir_fis, datos_write_mem->valor);
    }
    else{
        log_error(memoria_logger,"ERROR en ESCRITURA para dirección %u", datos_write_mem->dir_fis);
    }
    pthread_mutex_unlock(&mutex_memoria);
} 

void read_mem(t_paquete* paquete_cpu){
    t_datos_read_mem* datos_read_mem = deserializar_read_mem(paquete_cpu);
    usleep(atoi(valores_config_memoria->retardo_respuesta) * 1000);

    pthread_mutex_lock(&mutex_memoria);
    if (datos_read_mem->dir_fis + sizeof(uint32_t) < tamanio_memoria) {
        uint32_t valor;
        memcpy(&valor, (char *)memoria + datos_read_mem->dir_fis, sizeof(uint32_t));
        send(fd_cpu, &valor, sizeof(valor), 0);

        log_info(memoria_log_obligatorios, "## Lectura - (PID:TID) - (<%u>:<%u>) - Direccion Fisica:%u - Tamanio: <%u>\n",datos_read_mem->pidHilo, datos_read_mem->tidHilo, datos_read_mem->dir_fis, valor);
    }
    else{
        log_error(memoria_logger,"ERROR en LECTURA");
    }
    pthread_mutex_unlock(&mutex_memoria);
} 
