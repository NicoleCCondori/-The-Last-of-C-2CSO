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
        case MENSAJE:
            break;
		case OBTENER_CONTEXTO:
			devolver_contexto_ejecucion();
            break;
		case ACTUALIZAR_CONTEXTO:
			actualizar_contexto_de_ejecucion();
            break;
		case OBTENER_INSTRUCCION:
			obtener_instruccion();
            break;
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
        exit(EXIT_FAILURE);
    }

    if (hilo->pid ==! datos_contexto->PID) {
            log_error(memoria_logger, "El proceso no corresponde");
    }

    // Crear y llenar el contexto de ejecucion
    t_contextoEjecucion* contexto = malloc(sizeof(t_contextoEjecucion));
    if (contexto == NULL) {
        log_error(memoria_logger, "Error al asignar memoria para el contexto");
        exit(EXIT_FAILURE);
    }

    // Copiar los registros y valores
    contexto->RegistrosCPU->AX = hilo->registro->AX;
    contexto->RegistrosCPU->BX = hilo->registro->BX;
    contexto->RegistrosCPU->CX = hilo->registro->CX;
    contexto->RegistrosCPU->DX = hilo->registro->DX;
    contexto->RegistrosCPU->EX= hilo->registro->EX;
    contexto->RegistrosCPU->FX = hilo->registro->FX;
    contexto->RegistrosCPU->GX = hilo->registro->GX;
    contexto->RegistrosCPU->HX = hilo->registro->HX;

    contexto->PC = hilo->pc;

    contexto->base = 0;
    contexto->limite = 0;

    contexto->TID = datos_contexto->TID;

    
    t_paquete* paquete_enviar_contexto = crear_paquete(ENVIAR_CONTEXTO);
    serializar_enviar_contexto(paquete_enviar_contexto, contexto);
    enviar_paquete(paquete_enviar_contexto, fd_cpu);
    eliminar_paquete(paquete_enviar_contexto);

    
}

TCB* buscar_tcb_por_tid(uint32_t tid_buscado) {
    for (int i = 0; i < list_size(lista_tcb); i++) {
        TCB* tcb_actual = list_get(lista_tcb, i);

        if (tcb_actual->tid == tid_buscado) {
            return tcb_actual;
        }
    }
    return NULL;
}

void actualizar_contexto_de_ejecucion() {
    t_paquete* paquete_actualizar_contexto = recibir_paquete(fd_kernel);
    t_actualizar_contexto* datos_contexto = deserializar_actualizar_contexto(paquete_actualizar_contexto);

    TCB* hilo = buscar_tcb_por_tid(datos_contexto->TID);
    
    hilo->registro->AX = datos_contexto->contexto_ejecucion->RegistrosCPU->AX;
    hilo->registro->BX = datos_contexto->contexto_ejecucion->RegistrosCPU->BX;
    hilo->registro->CX = datos_contexto->contexto_ejecucion->RegistrosCPU->CX;
    hilo->registro->DX = datos_contexto->contexto_ejecucion->RegistrosCPU->DX;
    hilo->registro->EX = datos_contexto->contexto_ejecucion->RegistrosCPU->EX;
    hilo->registro->FX = datos_contexto->contexto_ejecucion->RegistrosCPU->FX;
    hilo->registro->GX = datos_contexto->contexto_ejecucion->RegistrosCPU->GX;
    hilo->registro->HX = datos_contexto->contexto_ejecucion->RegistrosCPU->HX;

    hilo->pc = datos_contexto->contexto_ejecucion->PC;

    log_info(memoria_logger, "Contexto actualizado para TID %d", datos_contexto->TID);

}



void obtener_instruccion() {
    t_paquete* paquete_instruccion = recibir_paquete(fd_kernel);
    t_obtener_instruccion* datos_instruccion = deserializar_obtener_instruccion(paquete_instruccion);
    
    TCB* hilo = buscar_tcb_por_tid(datos_instruccion->TID);
    if (hilo == NULL) {
        log_error(memoria_logger, "Hilo no encontrado");
    	exit(EXIT_FAILURE);
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
    if (hilo->pc < 0 || hilo->pc >= num_lineas) {
        log_error(memoria_logger, "El valor de PC proporcionado es inválido.");
        // Liberar la memoria usada por las líneas leídas
        for (size_t i = 0; i < num_lineas; i++) {
            free(lineas[i]);
        }
        free(lineas);
    	exit(EXIT_FAILURE);
    }

    // Devolver la instrucción correspondiente al PC
    char *instruccion = strdup(lineas[hilo->pc]);

    // Liberar la memoria usada por las líneas leídas
    for (size_t i = 0; i < num_lineas; i++) {
        free(lineas[i]);
    }
    free(lineas);

    t_paquete* paquete_enviar_instruccion = crear_paquete(ENVIAR_INSTRUCCION);
    serializar_enviar_instruccion(paquete_enviar_instruccion, instruccion);
    enviar_paquete(paquete_enviar_instruccion, fd_cpu);
    eliminar_paquete(paquete_enviar_instruccion);
}

char* READ_MEM(){
	return "OK";
}

char* WRITE_MEM(){
return "OK";
}