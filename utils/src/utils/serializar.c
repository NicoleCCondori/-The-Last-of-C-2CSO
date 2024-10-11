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

void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}

void agregar_buffer_string(t_buffer* buffer, char* args){
    uint32_t tamanio = strlen(args) +1;
    agregar_buffer_Uint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->offset, args, tamanio);
    buffer->offset += tamanio;
    buffer->size += tamanio;
}

//PARA DESERIALIZAR
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
	void *a_enviar = serializar_paquete(paquete, bytes_totales); //no termino de entender este serializar

	send(socket_cliente, a_enviar, bytes_totales, 0);

	free(a_enviar);
}

t_paquete* recibir_paquete(int socket_cliente){
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = recibir_operacion(socket_cliente);
	if (paquete->codigo_operacion == -1)	{
		free(paquete);
		return NULL;
	}

	int buffer_size;
	void* buffer_stream = recibir_buffer(&buffer_size, socket_cliente);

	// Crear y asignar el buffer
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = buffer_size;
	paquete->buffer->stream = buffer_stream;


	return paquete;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}


void agregar_buffer_contextoEjecucion(t_buffer* buffer, t_ContextoEjecucion* contexto) {
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(t_ContextoEjecucion));
    if (buffer->stream == NULL) {
        perror("Error al reallocar memoria");
        return;
    }
    memcpy(buffer->stream + buffer->offset, contexto, sizeof(t_ContextoEjecucion));
    buffer->size += sizeof(t_ContextoEjecucion);
    buffer->offset += sizeof(t_ContextoEjecucion);
}






   /*
   EJEMPLO PARA LUEGO HACER LA SERIALIZACION Y DESERIALIZACION
    typedef struct {
        uint_32 pid;
        int tam_proceso;
    } t_asignar_memoria

    Ejemplo en kernel: lo que iria en k_conexiones.c

    void asignar_espacio_memoria(int fd_memoria, uint32_t pid, int tam_proceso){
    t_paquete* paquete_asignar_memoria = crear_paquete(ASIGNAR_MEMORIA);
    serializar_asignar_memoria(paquete_asignar_memoria, pid, tam_proceso);
    enviar_paquete(paquete_asignar_memoria, fd_memoria);
    destruir_buffer_paquete(paquete_asignar_memoria);
    }

    Ejemplo en kernel: lo que iria en serializar.c (aca)

    void* serializar_asignar_memoria(t_paquete* paquete_asignar_memoria, uint32_t pid, int tam_proceso){
        agregar_buffer_Uint32(paquete_asignar_memoria->buffer, pid);
        agregar_buffer_int(paquete_asignar_memoria->buffer, tam_proceso);
    }

    t_asignar_memoria* deserializar_asignar_memoria(t_paquete* paquete){
        t_asignar_memoria* asignar_memoria = malloc(sizeof(t_asignar_memoria));

        asignar_memoria->pid = leer_buffer_Uint32(paquete_asignar_memoria->buffer);
        asignar_memoria-> tam_proceso = leer_buffer_int(paquete_asignar_memoria->buffer);

    }

    LO QUE IRIA CUANDO QUIERO DESERIALZAR EN MI MODULO:
        t_paquete* paquete_asignar_memoria_recibido = recibir_paquete(fd_kernel);
        t_asignar_memoria* asignar_memoria_deserializado = deserializar_asignar_memoria(paquete_asignar_memoria_recibido);
        free(asignar_memoria_deserializado);
        destruir_buffer_paquete(paquete_asignar_memoria_recibido);
   */


/*AGREGADO DE MEMORIA*/
    t_crear_hilo* deserializar_crear_hilo(t_paquete* paquete){
    t_crear_hilo*  datos_hilo = malloc(sizeof(t_crear_hilo));

    datos_hilo->PID = leer_buffer_Uint32(paquete->buffer);
    datos_hilo->TID = leer_buffer_Uint32(paquete->buffer);
    datos_hilo-> prioridad = leer_buffer_int(paquete->buffer);
    datos_hilo-> path = leer_buffer_string(paquete->buffer);
    
    return datos_hilo;
}

t_enviar_contexto* deserializar_enviar_contexto(t_paquete* paquete){
    t_enviar_contexto* enviar_contexto = malloc(sizeof(t_enviar_contexto));

    enviar_contexto->PID = leer_buffer_Uint32(paquete->buffer);
    enviar_contexto->TID = leer_buffer_Uint32(paquete->buffer);
    
    return enviar_contexto;
}

t_obtener_instruccion* deserializar_obtener_instruccion(t_paquete* paquete){
    t_obtener_instruccion* enviar_contexto = malloc(sizeof(t_obtener_instruccion));

    enviar_contexto->TID = leer_buffer_Uint32(paquete->buffer);
    enviar_contexto->PC = leer_buffer_Uint32(paquete->buffer);
    
    return enviar_contexto;
}

void serializar_enviar_contexto(t_paquete* paquete_devolver_contexto, t_ContextoEjecucion* contextoEjecucion){
    agregar_buffer_contextoEjecucion(paquete_devolver_contexto->buffer, contextoEjecucion);
}

void serializar_enviar_instruccion(t_paquete* paquete_enviar_instruccion, char* instruccion){
    agregar_buffer_string(paquete_enviar_instruccion->buffer, instruccion);
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
