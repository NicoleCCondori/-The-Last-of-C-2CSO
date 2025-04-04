#include "serializar.h"

t_paquete* crear_paquete(op_code codigo_op){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_op;
    crear_buffer(paquete);
    return paquete;
}

void crear_buffer(t_paquete* paquete){
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

void destruir_buffer_paquete(t_paquete* paquete){ 
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void eliminar_paquete(t_paquete* paquete){
    if(paquete!=NULL){
        if(paquete->buffer!=NULL){
            if(paquete->buffer->stream!=NULL){
                free(paquete->buffer->stream);
            }
            	free(paquete->buffer);
        }
        	free(paquete);
    }
}


//PARA SERIALIZAR
void agregar_buffer_int(t_buffer* buffer, int entero){
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(int));
    buffer->size += sizeof(int);
    memcpy(buffer->stream+ buffer->offset, &entero, sizeof(int));
    buffer->offset += sizeof(int);
}

void agregar_buffer_Uint32(t_buffer* buffer, uint32_t entero){
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
    buffer->size += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

void agregar_buffer_string(t_buffer* buffer, char* args){
    uint32_t tamanio = strlen(args) +1;
    agregar_buffer_Uint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->offset, args, tamanio);
    buffer->offset += tamanio;
    buffer->size += tamanio;
}

void serializar_enviar_contexto_cpu(t_paquete* paquete, t_contextoEjecucion* contexto){
    if (paquete == NULL || contexto == NULL) {
        printf("Error: paquete o contexto son NULL en serializar_enviar_contexto\n");
        return;
    }
/*
    size_t size_total = sizeof(uint32_t) *2  // TID
                        + sizeof(uint32_t)       // PC
                        + sizeof(uint32_t) * 2 // base y limite
                        + sizeof(RegistrosCPU); // RegistrosCPU
                        */
                        size_t size_total = sizeof(uint32_t)   // TID
                            + sizeof(uint32_t) // PC
                            + sizeof(uint32_t) // base
                            + sizeof(uint32_t) // limite
                            + sizeof(uint32_t) // pid
                            + sizeof(RegistrosCPU); // RegistrosCPU



    paquete->buffer = malloc(sizeof(t_buffer));
    if (paquete->buffer == NULL) {
        printf("Error: No se pudo asignar memoria para el buffer del paquete\n");
        return;
    }
    
    paquete->buffer->size = size_total;
    paquete->buffer->offset = 0;
    paquete->buffer->stream = malloc(size_total);
    if (paquete->buffer->stream == NULL) {
        printf("Error: No se pudo asignar memoria para el stream del buffer\n");
        free(paquete->buffer);
        return;
    }

    // Puntero para escribir en el buffer
    void* stream = paquete->buffer->stream;

    // Serializar TID
    memcpy(stream, &(contexto->TID), sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Serializar PC
    memcpy(stream, &(contexto->PC), sizeof(uint32_t));
    stream += sizeof(int32_t);

    // Serializar base
    memcpy(stream, &(contexto->base), sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Serializar limite
    memcpy(stream, &(contexto->limite), sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(stream, &(contexto->pid), sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Serializar RegistrosCPU (aquí suponemos que la estructura RegistrosCPU se serializa directamente)
    /*memcpy(stream, contexto->RegistrosCPU, sizeof(RegistrosCPU));
    stream += sizeof(RegistrosCPU);*/
        if (contexto->RegistrosCPU != NULL) {
        memcpy(stream, contexto->RegistrosCPU, sizeof(RegistrosCPU));
    } else {
        memset(stream, 0, sizeof(RegistrosCPU)); // Evitar datos basura
    }
    stream += sizeof(RegistrosCPU);
}
char* deserializar_enviar_instruccion(t_paquete* paquete){
    if (paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        printf("Error: el paquete o su buffer son nulos.");
        return NULL;
    }

    void* stream = paquete->buffer->stream;

    // Leer la longitud de la instrucción
    uint32_t longitud_instruccion;
    memcpy(&longitud_instruccion, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);


    // Leer la instrucción
    char* instruccion = malloc(longitud_instruccion);
    if (instruccion == NULL) {
        printf("Error: no se pudo asignar memoria para la instrucción.");
        return NULL;
    }

    memcpy(instruccion, stream, longitud_instruccion);

    printf("Instrucción deserializada correctamente: %s \n", instruccion);

    return instruccion;
}

void serializar_enviar_instruccion(t_paquete* paquete_enviar_instruccion, char* instruccion) {
    if (paquete_enviar_instruccion == NULL || instruccion == NULL) {
        printf("Error: el paquete o la instrucción son nulos");
        return;
    }

    // Longitud de la instrucción
    uint32_t longitud_instruccion = strlen(instruccion) + 1; // Incluye el terminador nulo

    // Tamaño del nuevo buffer
    paquete_enviar_instruccion->buffer->size = sizeof(uint32_t) + longitud_instruccion;

    // Reservar memoria para el buffer
    paquete_enviar_instruccion->buffer->stream = malloc(paquete_enviar_instruccion->buffer->size);
    if (paquete_enviar_instruccion->buffer->stream == NULL) {
        printf("Error: no se pudo asignar memoria para el buffer.");
        return;
    }

    // Serializar los datos
    void* stream = paquete_enviar_instruccion->buffer->stream;

    // Copiar la longitud de la instrucción
    memcpy(stream, &longitud_instruccion, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Copiar la instrucción en sí
    memcpy(stream, instruccion, longitud_instruccion);

    printf("EN SERIALIZAR: Instrucción serializada correctamente: %s \n", instruccion);
}


//PARA DESERIALIZAR

t_contextoEjecucion* deserializar_enviar_contexto_cpu(t_paquete* paquete) {
    if (paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        printf("Error: El paquete o el buffer son NULL en deserializar_contexto_ejecucion\n");
        return NULL;
    }

    // Crear una nueva estructura para almacenar los datos deserializados
    t_contextoEjecucion* contexto = malloc(sizeof(t_contextoEjecucion));
    if (contexto == NULL) {
        printf("Error: No se pudo asignar memoria para la estructura t_contextoEjecucion\n");
        return NULL;
    }

    // Puntero para leer desde el stream del buffer
    void* stream = paquete->buffer->stream;

    // Deserializar TID
    memcpy(&(contexto->TID), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Deserializar PC
    memcpy(&(contexto->PC), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Deserializar base
    memcpy(&(contexto->base), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Deserializar limite
    memcpy(&(contexto->limite), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    memcpy(&(contexto->pid), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // Deserializar RegistrosCPU (suponiendo que es una estructura)
        contexto->RegistrosCPU = malloc(sizeof(RegistrosCPU));
    if (contexto->RegistrosCPU == NULL) {
        printf("Error: No se pudo asignar memoria para RegistrosCPU\n");
        free(contexto);
        return NULL;
    }
    memcpy(contexto->RegistrosCPU, stream, sizeof(RegistrosCPU));
    stream += sizeof(RegistrosCPU);
    /*contexto->RegistrosCPU = malloc(sizeof(RegistrosCPU));
    if (contexto->RegistrosCPU == NULL) {
        printf("Error: No se pudo asignar memoria para RegistrosCPU\n");
        free(contexto); // Liberamos el contexto previamente asignado
        return NULL;
    }
    memcpy(contexto->RegistrosCPU, stream, sizeof(RegistrosCPU));
    stream += sizeof(RegistrosCPU);*/

    return contexto;
}

uint32_t leer_buffer_int(t_buffer* buffer)
{
    int entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(int));
    buffer->offset += sizeof(int);
    return entero;
}

uint32_t leer_buffer_Uint32(t_buffer* buffer)
{
    uint32_t entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    return entero;
}

uint8_t leer_buffer_Uint8(t_buffer* buffer)
{
    uint8_t entero;
    memcpy(&entero, buffer->stream + buffer->offset, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
    return entero;
}

char *leer_buffer_string(t_buffer* buffer)
{
    char *cadena;
    uint32_t tamanio;

    tamanio = leer_buffer_Uint32(buffer);
    cadena = malloc((tamanio) + 1);
    memcpy(cadena, buffer->stream + buffer->offset, tamanio);
    buffer->offset += tamanio;

    *(cadena + tamanio) = '\0';

    return cadena;
}


void *serializar_paquete(t_paquete *paquete, int bytes){
	void *magic = malloc(bytes); // Reserva memoria del tamaño del paquete completo
	int offset = 0;

	memcpy(magic + offset, &(paquete->codigo_operacion), sizeof(int));
	offset += sizeof(int);

	memcpy(magic + offset, &(paquete->buffer->size), sizeof(int));
	offset += sizeof(int);

	memcpy(magic + offset, paquete->buffer->stream, paquete->buffer->size);
	offset += paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete *paquete, int socket_cliente){
	int bytes_totales = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes_totales);

    int resultado = send(socket_cliente, a_enviar, bytes_totales, 0);
    if (resultado == -1) {
        printf("Error al enviar el paquete:");
        free(a_enviar);
        return;  // Error
    }

	free(a_enviar);
}

t_paquete* recibir_paquete(int socket_cliente){
	t_paquete *paquete = malloc(sizeof(t_paquete));
     if (paquete == NULL) {
        printf("Error al asignar memoria para el paquete\n");
        return NULL;
    }

	paquete->codigo_operacion = recibir_operacion(socket_cliente);
	if (paquete->codigo_operacion == -1){
		free(paquete);
		return NULL;
	}

	int buffer_size;
	void* buffer_stream = recibir_buffer(&buffer_size, socket_cliente);

	// Crear y asignar el buffer
	paquete->buffer = malloc(sizeof(t_buffer));
    if(paquete->buffer==NULL){
        free(buffer_stream);
        free(paquete);
        return NULL;
    }
	paquete->buffer->size = buffer_size;
	paquete->buffer->stream = buffer_stream;


	return paquete;
}

void agregar_buffer_contextoEjecucion(t_buffer* buffer, t_contextoEjecucion* contexto) {
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(t_contextoEjecucion));
    if (buffer->stream == NULL) {
        perror("Error al reallocar memoria");
        return;
    }
    memcpy(buffer->stream + buffer->offset, contexto, sizeof(t_contextoEjecucion));
    buffer->size += sizeof(t_contextoEjecucion);
    buffer->offset += sizeof(t_contextoEjecucion);
}

/*AGREGADO DE MEMORIA*/
t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete_hilo){
    if (paquete_hilo == NULL || paquete_hilo->buffer == NULL) {
        printf("Error: Paquete o buffer es NULL en deserializar_hilo_ready\n");
        return NULL;
    }

    // Puntero a los datos dentro del buffer
    void* stream = paquete_hilo->buffer->stream;
    
    // Deserializar PID (uint32_t)
    uint32_t pid;
    memcpy(&pid, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);  // Avanzar el puntero

    // Deserializar TID (uint32_t)
    uint32_t tid;
    memcpy(&tid, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);  // Avanzar el puntero

    // Deserializar Prioridad (int)
    int prioridad;
    memcpy(&prioridad, stream, sizeof(int));
    stream += sizeof(int);  // Avanzar el puntero

    // Deserializar el tamaño de la cadena 'path' (size_t)
    size_t path_length;
    memcpy(&path_length, stream, sizeof(size_t));
    stream += sizeof(size_t);  // Avanzar el puntero

    // Deserializar la cadena 'path'
    char* path = malloc(path_length);
    if (path == NULL) {
        printf("Error: No se pudo asignar memoria para la cadena 'path'\n");
        return NULL;
    }
    memcpy(path, stream, path_length);
    
    // Crear la estructura t_crear_hilo y asignar los valores deserializados
    t_crear_hilo* hilo = malloc(sizeof(t_crear_hilo));
    if (hilo == NULL) {
        printf("Error: No se pudo asignar memoria para t_crear_hilo\n");
        free(path);  // Liberar memoria de path si no se pudo asignar t_crear_hilo
        return NULL;
    }

    hilo->PID = pid;
    hilo->TID = tid;
    hilo->prioridad = prioridad;
    hilo->path = path;  // Asignamos la cadena path deserializada

    return hilo;
}

t_enviar_contexto* deserializar_enviar_contexto(t_paquete* paquete){
    t_enviar_contexto* enviar_contexto = malloc(sizeof(t_enviar_contexto));

    enviar_contexto->PID = leer_buffer_Uint32(paquete->buffer);
    printf("PID %u:", enviar_contexto->PID );
    enviar_contexto->TID = leer_buffer_Uint32(paquete->buffer);
    printf("TID %u:", enviar_contexto->TID );
    return enviar_contexto;
}

t_obtener_instruccion* deserializar_obtener_instruccion(t_paquete* paquete) {
    // Reservar memoria para la estructura de salida
    t_obtener_instruccion* enviar_contexto = malloc(sizeof(t_obtener_instruccion));
    if (enviar_contexto == NULL) {
        printf("Error: No se pudo asignar memoria para deserializar el paquete\n");
        return NULL;
    }

    // Definir el offset para leer los valores del buffer
    size_t offset = 0;

    // Leer el valor de PC desde el buffer usando memcpy
    memcpy(&enviar_contexto->PC, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);  // Aumentamos el offset por el tamaño de uint32_t

    // Leer el valor de TID desde el buffer usando memcpy
    memcpy(&enviar_contexto->TID, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);  // Aumentamos el offset por el tamaño de uint32_t

    // Leer el valor de PID desde el buffer usando memcpy
    memcpy(&enviar_contexto->PID, paquete->buffer->stream + offset, sizeof(uint32_t));

    return enviar_contexto;
}

void serializar_hilo_ready(t_paquete* paquete_hilo, uint32_t pid, uint32_t tid, int prioridad, const char* path) {
    // Calcular el tamaño total necesario para los datos serializados
    size_t path_length = strlen(path) + 1;  

    if (paquete_hilo == NULL) {
        printf( "Error: paquete es NULL en serializar_obtener_contexto\n");
        return;
    }

    paquete_hilo->buffer = malloc(sizeof(t_buffer));
    paquete_hilo->buffer->size = 2 * sizeof(uint32_t) + 1*sizeof(int) + path_length + sizeof(size_t);
    paquete_hilo->buffer->stream = malloc(paquete_hilo->buffer->size);

    // Escribir los valores en el buffer
    void* stream = paquete_hilo->buffer->stream;
    memcpy(stream, &pid, sizeof(uint32_t));   // Copiar PID
    stream += sizeof(uint32_t);
    
    memcpy(stream, &tid, sizeof(uint32_t));   // Copiar TID
    stream += sizeof(uint32_t);

    memcpy(stream, &prioridad, sizeof(size_t));   // Copiar TID
    stream += sizeof(int);

    // Serializar la longitud de la cadena 'path'
    memcpy(stream, &path_length, sizeof(size_t));
    stream += sizeof(size_t);

    // Serializar la cadena 'path'
    memcpy(stream, path, path_length);
    paquete_hilo->buffer->offset += path_length;

    paquete_hilo->buffer->offset = stream - paquete_hilo->buffer->stream; 
}

 void serializar_hilo_cpu(t_paquete* paquete, uint32_t pid, uint32_t tid)
 {
    if (paquete == NULL) {
        printf( "Error: paquete es NULL en serializar_obtener_contexto\n");
        return;
    }

    // Tamaño del buffer para PID y TID (2 * uint32_t)
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 2 * sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Escribir los valores en el buffer
    void* stream = paquete->buffer->stream;
    memcpy(stream, &pid, sizeof(uint32_t));   // Copiar PID
    stream += sizeof(uint32_t);
    memcpy(stream, &tid, sizeof(uint32_t));   // Copiar TID
}

void serializar_obtener_instruccion(t_paquete* paquete, uint32_t pc, uint32_t tid, uint32_t pid)
{
    if (paquete == NULL) {
        printf("Error: paquete es NULL en serializar_obtener_instruccion\n");
        return;
    }

    // Asignar memoria para el buffer
    paquete->buffer = malloc(sizeof(t_buffer));
    if (paquete->buffer == NULL) {
        printf("Error: No se pudo asignar memoria para el buffer\n");
        return;
    }

    // Asignar memoria para el stream dentro del buffer
    paquete->buffer->size = 3 * sizeof(uint32_t);  // Espacio para PC, TID, PID
    paquete->buffer->stream = malloc(paquete->buffer->size);
    if (paquete->buffer->stream == NULL) {
        printf("Error: No se pudo asignar memoria para el stream\n");
        free(paquete->buffer);  // Liberar el buffer si falla la asignación del stream
        return;
    }

    // Usamos desplazamiento para saber en qué parte del buffer estamos
    size_t offset = 0;

    // Copiar los valores en el buffer usando el desplazamiento
    memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));  // Copiar PC
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &tid, sizeof(uint32_t)); // Copiar TID
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &pid, sizeof(uint32_t)); // Copiar PID
}


 void* serializar_asignar_memoria(t_paquete* paquete, uint32_t pid, int tam_proceso){
      
    if (paquete == NULL) {
        printf( "Error: paquete es NULL en serializar_obtener_contexto\n");
        return NULL;
    }

    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 1 * sizeof(uint32_t) + 1*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Escribir los valores en el buffer
    void* stream = paquete->buffer->stream;
    memcpy(stream, &pid, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream, &tam_proceso, sizeof(int));

    return NULL;
}

t_asignar_memoria* deserializar_asignar_memoria(t_paquete* paquete){
    t_asignar_memoria* asignar_memoria = malloc(sizeof(t_asignar_memoria));

    asignar_memoria->pid = leer_buffer_Uint32(paquete->buffer);
    asignar_memoria-> tam_proceso = leer_buffer_int(paquete->buffer);
    return asignar_memoria;
}

RegistrosCPU* leer_buffer_registro(t_buffer* buffer)
{
    RegistrosCPU* registro = malloc(sizeof(RegistrosCPU));
    void* stream = buffer->stream;
    
    memcpy(&registro->AX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->BX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->CX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->DX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->EX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->FX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->GX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&registro->HX, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    return registro;
}

void agregar_buffer_registrosCPU(t_buffer* buffer, RegistrosCPU* registro)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(RegistrosCPU));
    buffer->size += sizeof(RegistrosCPU);
    memcpy(buffer->stream + buffer->offset, registro, sizeof(RegistrosCPU));
    buffer->offset += sizeof(RegistrosCPU);
}

//===================================================================
t_datos_esenciales* deserializar_datos_esenciales(t_paquete* paquete){
    t_datos_esenciales* invocadores = malloc(sizeof(t_datos_esenciales));
    invocadores->pid_inv = leer_buffer_Uint32(paquete->buffer);
    invocadores->tid_inv = leer_buffer_Uint32(paquete->buffer);
    return invocadores;
}

t_process_create* deserializar_process_create(t_paquete* paquete){
    t_process_create* pc_hiloMain = malloc(sizeof(t_process_create));
    
    pc_hiloMain->PID = leer_buffer_Uint32(paquete->buffer);
    pc_hiloMain->TID= leer_buffer_Uint32(paquete->buffer);
    pc_hiloMain->nomArchP = leer_buffer_string(paquete->buffer);
    pc_hiloMain->tamProceso = leer_buffer_Uint32(paquete->buffer);
    pc_hiloMain->prioridadHM = leer_buffer_Uint32(paquete->buffer);
    return pc_hiloMain;
}

t_thread_create* deserializar_thread_create(t_paquete* paquete) {
    
    size_t offset = 0;
    size_t size_archivo;

    // Crear la estructura para almacenar los datos
    t_thread_create* thread_create = malloc(sizeof(t_thread_create));
    if (!thread_create) {
        perror("Error al asignar memoria para t_thread_create");
        exit(EXIT_FAILURE);
    }

    // Buffer del paquete
    //char* buffer = paquete->buffer;

    // Deserializar PID
    memcpy(&thread_create->PID, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar TID
    memcpy(&thread_create->TID, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializar tamaño del archivo
    memcpy(&size_archivo, paquete->buffer->stream + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // Deserializar archivo
    thread_create->nombreArchT = malloc(size_archivo);
    if (!thread_create->nombreArchT) {
        perror("Error al asignar memoria para archivo");
        free(thread_create);
        exit(EXIT_FAILURE);
    }
    memcpy(thread_create->nombreArchT, paquete->buffer->stream  + offset, size_archivo);
    offset += size_archivo;

    // Deserializar prioridad
    memcpy(&thread_create->prioridadH, paquete->buffer->stream  + offset, sizeof(uint32_t));

    return thread_create;
}



t_thread_join_y_cancel* deserializar_thread_join_y_cancel(t_paquete* paquete){
    t_thread_join_y_cancel* tc_hilo = malloc(sizeof(t_thread_join_y_cancel));
    tc_hilo->PID = leer_buffer_Uint32(paquete->buffer);
    tc_hilo->TID = leer_buffer_Uint32(paquete->buffer);
    tc_hilo->tid = leer_buffer_Uint32(paquete->buffer);
    return tc_hilo;
}

t_IO* deserializar_IO(t_paquete* paquete){

    t_IO* io_paquete = malloc(sizeof(t_IO));

    io_paquete->PID = leer_buffer_Uint32(paquete->buffer);
    io_paquete->TID= leer_buffer_Uint32(paquete->buffer);
    io_paquete->tiempo = leer_buffer_int(paquete->buffer);
    
    return io_paquete;
}

t_mutex_todos* deserializar_mutex(t_paquete* paquete){
    t_mutex_todos* mutex_paquete = malloc(sizeof(t_mutex_todos));
    mutex_paquete->PID = leer_buffer_Uint32(paquete->buffer);
    mutex_paquete->TID = leer_buffer_Uint32(paquete->buffer);
    mutex_paquete->recurso = leer_buffer_string(paquete->buffer);
    return mutex_paquete;
}

t_crear_archivo_memoria* deserializar_crear_archivo_memoria(t_paquete* paquete){
    t_crear_archivo_memoria* archivo_memoria = malloc(sizeof(t_crear_archivo_memoria));
    archivo_memoria->contenido = leer_buffer_string(paquete->buffer);
    archivo_memoria->nombre_archivo = leer_buffer_string(paquete->buffer);
    archivo_memoria->tamanio = leer_buffer_Uint32(paquete->buffer);
    return archivo_memoria;
}

//PAra informar(kernel) a memoria, la finalización de un hilo
void serializar_finalizar_hilo(t_paquete* paquete_memoria, uint32_t pid, uint32_t tid){
    agregar_buffer_Uint32(paquete_memoria->buffer,pid);
    agregar_buffer_Uint32(paquete_memoria->buffer,pid);
}

t_datos_esenciales* deserializar_finalizar_hilo(t_paquete* paquete_memoria){
    t_datos_esenciales* datos_mem =malloc(sizeof(t_datos_esenciales));
    datos_mem->pid_inv = leer_buffer_Uint32(paquete_memoria->buffer);
    datos_mem->tid_inv = leer_buffer_Uint32(paquete_memoria->buffer);
    return datos_mem;
}

//================================================================
void enviar_mensaje(char *mensaje, int socket){
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

t_datos_write_mem* deserializar_write_mem(t_paquete* paquete){
    t_datos_write_mem* write_mem = malloc(sizeof(t_datos_write_mem));

    write_mem->dir_fis = leer_buffer_Uint32(paquete->buffer);
    write_mem->valor = leer_buffer_Uint32(paquete->buffer);
    write_mem->pidHilo=leer_buffer_Uint32(paquete->buffer);
    write_mem->tidHilo=leer_buffer_Uint32(paquete->buffer);

    return write_mem;
}

void* serializar_write_mem(t_paquete* paquete_write_mem, uint32_t dir_fis, uint32_t valor,uint32_t PidHilo, uint32_t TidHilo){
    agregar_buffer_Uint32(paquete_write_mem->buffer, dir_fis);
    agregar_buffer_Uint32(paquete_write_mem->buffer, valor);
    agregar_buffer_Uint32(paquete_write_mem->buffer,PidHilo);
    agregar_buffer_Uint32(paquete_write_mem->buffer,TidHilo);
    return NULL;
}

t_datos_read_mem* deserializar_read_mem(t_paquete* paquete){
    t_datos_read_mem* read_mem = malloc(sizeof(t_datos_read_mem));
    read_mem->dir_fis = leer_buffer_Uint32(paquete->buffer);
    read_mem->pidHilo=leer_buffer_Uint32(paquete->buffer);
    read_mem->tidHilo=leer_buffer_Uint32(paquete->buffer);
    return read_mem;
}

void* serializar_read_mem(t_paquete* paquete_enviar_datos_lectura, uint32_t direccion_fisica, uint32_t PidHilo, uint32_t TidHilo){
    agregar_buffer_Uint32(paquete_enviar_datos_lectura->buffer, direccion_fisica);
    agregar_buffer_Uint32(paquete_enviar_datos_lectura->buffer,PidHilo);
    agregar_buffer_Uint32(paquete_enviar_datos_lectura->buffer,TidHilo);
    return NULL;
}

void* serializar_enviar_DUMP_MEMORY(t_paquete* paquete_dump_memory, void* datos,uint32_t tamanio,char* nombre){
    agregar_buffer_Uint32(paquete_dump_memory->buffer, tamanio);

    // Agrego el contenido de la memoria
    paquete_dump_memory->buffer->stream = realloc(paquete_dump_memory->buffer->stream, paquete_dump_memory->buffer->size + tamanio);
    memcpy(paquete_dump_memory->buffer->stream + paquete_dump_memory->buffer->offset, datos, tamanio);
    paquete_dump_memory->buffer->offset += tamanio;
    paquete_dump_memory->buffer->size += tamanio;

    agregar_buffer_string(paquete_dump_memory->buffer, nombre);

    return NULL;
}

void* serializar_obtener_contexto(t_paquete* paquete_obtener_contexto, uint32_t pid, uint32_t tidHilo){
    agregar_buffer_Uint32(paquete_obtener_contexto->buffer,pid);
    agregar_buffer_Uint32(paquete_obtener_contexto->buffer,tidHilo);
    printf("entra aca\n");
    return NULL;
}

void serializar_int(t_paquete* paquete, int numero){
    int tamanio_buffer = sizeof(int);
    paquete->buffer->stream = realloc(paquete->buffer->stream, tamanio_buffer);
    memcpy(paquete->buffer->stream, &numero, sizeof(int));
    paquete->buffer->size = tamanio_buffer;
}

int deserializar_int(t_paquete* paquete){
    int numero;
    memcpy(&numero, paquete->buffer->stream, sizeof(int));
    return numero;
}

void serializar_proceso_memoria(t_paquete* paquete, uint32_t pid, uint32_t bit_confirmacion) {
    // Calcular el tamaño del buffer necesario para serializar los datos
    int tamanio_buffer = sizeof(uint32_t) + sizeof(uint32_t);  // bit_confirmacion (uint32_t) y pid (uint32_t)

    // Verificar si el buffer está inicializado. Si no, asignar memoria.
    if (paquete->buffer->stream == NULL) {
        paquete->buffer->stream = malloc(tamanio_buffer);
        if (paquete->buffer->stream == NULL) {
            // Error en la asignación de memoria
            printf("Error al asignar memoria para el buffer\n");
            return;
        }
    } else {
        // Si el buffer ya existe, redimensionarlo
        void* nuevo_stream = realloc(paquete->buffer->stream, tamanio_buffer);
        if (nuevo_stream == NULL) {
            // Error en realloc, no modificamos el buffer original
            printf("Error al redimensionar el buffer con realloc\n");
            return;
        }
        paquete->buffer->stream = nuevo_stream;
    }

    // Inicializar el desplazamiento dentro del buffer
    int desplazamiento = 0;

    // Copiar pid al buffer
    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Copiar bit_confirmacion al buffer
    memcpy(paquete->buffer->stream + desplazamiento, &bit_confirmacion, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Actualizar el tamaño del buffer
    paquete->buffer->size = tamanio_buffer;

    // Mensaje de depuración con printf
    printf("Paquete serializado: bit_confirmacion=%u, pid=%u, tamanio=%d\n",
             bit_confirmacion, pid, tamanio_buffer);
}


t_asigno_memoria* deserializar_proceso_memoria(t_paquete* paquete) {
    // Verificar que el tamaño del paquete recibido sea suficiente para contener la información
   
    // Asignar memoria para la estructura t_asigno_memoria
    t_asigno_memoria* asigno_memoria = malloc(sizeof(t_asigno_memoria));
    if (asigno_memoria == NULL) {
        printf("Error al asignar memoria para t_asigno_memoria\n");
        return NULL;
    }

    // Inicializar el desplazamiento para leer los datos del buffer
    int desplazamiento = 0;

    // Copiar el valor de bit_confirmacion desde el buffer al campo correspondiente en la estructura
    memcpy(&(asigno_memoria->pid), paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Copiar el valor de pid desde el buffer al campo correspondiente en la estructura
    memcpy(&(asigno_memoria->bit_confirmacion), paquete->buffer->stream + desplazamiento, sizeof(uint32_t));

    // Devolver la estructura deserializada
    return asigno_memoria;
}
/*
void serializar_hilo_memoria(t_paquete* paquete, uint32_t bit_confirmacion,uint32_t pid,uint32_t tid){
    int tamanio_buffer = sizeof(uint32_t)*3;  // bit_confirmacion (uint32_t) y pid (uint32_t)

    // Verificar si el buffer está inicializado. Si no, asignar memoria.
    if (paquete->buffer->stream == NULL) {
        paquete->buffer->stream = malloc(tamanio_buffer);
        if (paquete->buffer->stream == NULL) {
            // Error en la asignación de memoria
            printf("Error al asignar memoria para el buffer\n");
            return;
        }
    } else {
        // Si el buffer ya existe, redimensionarlo
        void* nuevo_stream = realloc(paquete->buffer->stream, tamanio_buffer);
        if (nuevo_stream == NULL) {
            // Error en realloc, no modificamos el buffer original
            printf("Error al redimensionar el buffer con realloc\n");
            return;
        }
        paquete->buffer->stream = nuevo_stream;
    }

    // Inicializar el desplazamiento dentro del buffer
    int desplazamiento = 0;

    // Copiar pid al buffer
    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

     memcpy(paquete->buffer->stream + desplazamiento, &tid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Copiar bit_confirmacion al buffer
    memcpy(paquete->buffer->stream + desplazamiento, &bit_confirmacion, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Actualizar el tamaño del buffer
    paquete->buffer->size = tamanio_buffer;

    // Mensaje de depuración con printf
    printf("Paquete serializado: bit_confirmacion=%u, pid=%u,tid=%u, tamanio=%d\n",
             bit_confirmacion, pid, tid, tamanio_buffer);
}*/
void serializar_hilo_memoria(t_paquete* paquete, uint32_t bit_confirmacion, uint32_t pid, uint32_t tid) {
    if (paquete == NULL) {
        printf("Error: El paquete es NULL\n");
        return;
    }

    // Verificar si el buffer está inicializado
    if (paquete->buffer == NULL) {
        paquete->buffer = malloc(sizeof(t_buffer));
        if (paquete->buffer == NULL) {
            printf("Error al asignar memoria para el buffer\n");
            return;
        }
        paquete->buffer->stream = NULL;
        paquete->buffer->size = 0;
    }

    // Calcular el tamaño necesario para el buffer
    int tamanio_buffer = sizeof(uint32_t) * 3; // bit_confirmacion, pid y tid

    // Asignar memoria para el stream
    paquete->buffer->stream = realloc(paquete->buffer->stream, tamanio_buffer);
    if (paquete->buffer->stream == NULL) {
        printf("Error al asignar memoria para el stream del buffer\n");
        free(paquete->buffer);
        paquete->buffer = NULL;
        return;
    }

    // Serializar los datos en el buffer
    int desplazamiento = 0;
    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &tid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &bit_confirmacion, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Actualizar el tamaño del buffer
    paquete->buffer->size = tamanio_buffer;

    // Mensaje de depuración
    printf("Paquete serializado: bit_confirmacion=%u, pid=%u, tid=%u, tamanio=%d\n",
           bit_confirmacion, pid, tid, tamanio_buffer);
}
/*
t_creacion_hilo*  deserializar_creacion_hilo_memoria(t_paquete* paquete){
     // Verificar que el tamaño del paquete recibido sea suficiente para contener la información
   
    // Asignar memoria para la estructura t_asigno_memoria
    t_creacion_hilo* creacion_hilo = malloc(sizeof(t_creacion_hilo));
    if (creacion_hilo == NULL) {
        printf("Error al asignar memoria para t_creacion_hilo\n");
        return NULL;
    }

    // Inicializar el desplazamiento para leer los datos del buffer
    int desplazamiento = 0;

    // Copiar el valor de bit_confirmacion desde el buffer al campo correspondiente en la estructura
    memcpy(&(creacion_hilo->pid), paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(creacion_hilo->tid), paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Copiar el valor de pid desde el buffer al campo correspondiente en la estructura
    memcpy(&(creacion_hilo->bit_confirmacion), paquete->buffer->stream + desplazamiento, sizeof(uint32_t));

    // Devolver la estructura deserializada
    return creacion_hilo;
}*/
t_creacion_hilo* deserializar_creacion_hilo_memoria(t_paquete* paquete) {
    if (paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        printf("Error: Paquete o su buffer son NULL\n");
        return NULL;
    }

    // Validar que el tamaño del buffer sea suficiente
    int tamanio_esperado = sizeof(uint32_t) * 3;
    if (paquete->buffer->size < tamanio_esperado) {
        printf("Error: Tamaño del buffer insuficiente. Esperado: %d, Recibido: %d\n", tamanio_esperado, paquete->buffer->size);
        return NULL;
    }

    // Reservar memoria para la estructura
    t_creacion_hilo* hilo = malloc(sizeof(t_creacion_hilo));
    if (hilo == NULL) {
        printf("Error al asignar memoria para la estructura t_creacion_hilo\n");
        return NULL;
    }

    // Extraer los datos del buffer
    int desplazamiento = 0;
    memcpy(&hilo->pid, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&hilo->tid, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&hilo->bit_confirmacion, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));

    return hilo;
}