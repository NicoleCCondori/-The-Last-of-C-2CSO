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
    buffer->stream = realloc(buffer->stream , buffer->size + sizeof(int));
    buffer->size += sizeof(int);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(int));
    buffer->offset += sizeof(int);
}

void agregar_buffer_Uint32(t_buffer *buffer, uint32_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
    buffer->size += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

void agregar_buffer_Uint8(t_buffer *buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}

void agregar_buffer_string(t_buffer* buffer, char* args)
{
    uint32_t tamanio = strlen(args) +1;
    agregar_buffer_Uint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->offset, args, tamanio);
    buffer->offset += tamanio;
    buffer->size += tamanio;
}
//AGREGO
void agregar_buffer_registrosCPU(t_buffer* buffer, RegistrosCPU* registro)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(RegistrosCPU));
    buffer->size += sizeof(RegistrosCPU);
    memcpy(buffer->stream + buffer->offset, registro, sizeof(RegistrosCPU));
    buffer->offset += sizeof(RegistrosCPU);
}
///////////////////////////

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
	void *a_enviar = serializar_paquete(paquete, bytes_totales); //copia los datos del buffer a la variable a_enviar

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


void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void* serializar_asignar_memoria(t_paquete* paquete_asignar_memoria, uint32_t pid, int tam_proceso){
    agregar_buffer_Uint32(paquete_asignar_memoria->buffer, pid);
    agregar_buffer_int(paquete_asignar_memoria->buffer, tam_proceso);
}

t_asignar_memoria* deserializar_asignar_memoria(t_paquete* paquete){
    t_asignar_memoria* asignar_memoria = malloc(sizeof(t_asignar_memoria));

    asignar_memoria->pid = leer_buffer_Uint32(paquete->buffer);
    asignar_memoria-> tam_proceso = leer_buffer_int(paquete->buffer);
}


void* serializar_hilo_ready(t_paquete* paquete_hilo,TCB* hilo){
    agregar_buffer_Uint32(paquete_hilo, hilo->pid);
    agregar_buffer_Uint32(paquete_hilo, hilo->tid);
    agregar_buffer_int(paquete_hilo, hilo->prioridad);
    agregar_buffer_registrosCPU(paquete_hilo, hilo->registro);
    agregar_buffer_string(paquete_hilo, hilo->path);
    agregar_buffer_Uint32(paquete_hilo, hilo->pc);
}

 void* serializar_hilo_cpu(t_paquete* hilo_cpu, uint32_t pid, uint32_t tid)
 {
    agregar_buffer_Uint32(hilo_cpu->buffer, pid);
    agregar_buffer_Uint32(hilo_cpu->buffer, tid);

 }
