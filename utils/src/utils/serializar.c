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

/*void agregar_buffer_Uint8(t_buffer* buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}*/

void agregar_buffer_string(t_buffer* buffer, char* args){
    uint32_t tamanio = strlen(args) +1;
    agregar_buffer_Uint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio);
    memcpy(buffer->stream + buffer->offset, args, tamanio);
    buffer->offset += tamanio;
    buffer->size += tamanio;
}

//PARA DESERIALIZAR

t_contextoEjecucion* leer_contexto_de_memoria(t_buffer* buffer){

    t_contextoEjecucion* contexto = malloc(sizeof(t_contextoEjecucion));
    void* stream=buffer->stream;
    contexto->RegistrosCPU = malloc(sizeof(RegistrosCPU));

    memcpy(&contexto->RegistrosCPU->AX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->BX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->CX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->DX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->EX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->FX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->GX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->RegistrosCPU->HX,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->PC,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->base,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->limite,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(&contexto->TID,stream,sizeof(uint32_t));
    stream+=sizeof(uint32_t);

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

t_actualizar_contexto* deserializar_actualizar_contexto(t_paquete* paquete){
    t_actualizar_contexto* enviar_contexto = malloc(sizeof(t_actualizar_contexto));

    enviar_contexto->TID = leer_buffer_Uint32(paquete->buffer);
    enviar_contexto->contexto_ejecucion = leer_contexto_de_memoria(paquete->buffer);
    
    return enviar_contexto;
}

void serializar_enviar_contexto(t_paquete* paquete_devolver_contexto, t_contextoEjecucion* contextoEjecucion){
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

void* serializar_hilo_ready(t_paquete* paquete_hilo,TCB* hilo){
    agregar_buffer_Uint32(paquete_hilo->buffer, hilo->pid);
    agregar_buffer_Uint32(paquete_hilo->buffer, hilo->tid);
    agregar_buffer_int(paquete_hilo->buffer, hilo->prioridad);
    agregar_buffer_registrosCPU(paquete_hilo->buffer, hilo->registro);
    agregar_buffer_string(paquete_hilo->buffer, hilo->path);
    agregar_buffer_Uint32(paquete_hilo->buffer, hilo->pc);
    return NULL; // retorno por defecto
}

 void* serializar_hilo_cpu(t_paquete* hilo_cpu, uint32_t pid, uint32_t tid)
 {
    agregar_buffer_Uint32(hilo_cpu->buffer, pid);
    agregar_buffer_Uint32(hilo_cpu->buffer, tid);
    return NULL; // retorno por defecto
 }

 void* serializar_asignar_memoria(t_paquete* paquete_asignar_memoria, uint32_t pid, int tam_proceso){
    agregar_buffer_Uint32(paquete_asignar_memoria->buffer, pid);
    agregar_buffer_int(paquete_asignar_memoria->buffer, tam_proceso);
    return NULL; // retorno por defecto
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
    
    pc_hiloMain->nomArchP = leer_buffer_string(paquete->buffer);
    pc_hiloMain->tamProceso = leer_buffer_Uint32(paquete->buffer);
    pc_hiloMain->prioridadHM = leer_buffer_Uint32(paquete->buffer);
    return pc_hiloMain;
}

t_thread_create* deserializar_thread_create(t_paquete* paquete){
    t_thread_create* tc_hilo = malloc(sizeof(t_thread_create));
    tc_hilo->nombreArchT = leer_buffer_string(paquete->buffer);
    tc_hilo->prioridadH = leer_buffer_Uint32(paquete->buffer);
    return tc_hilo;
}

uint32_t deserializar_thread_join_y_cancel(t_paquete* paquete){
    uint32_t tid = leer_buffer_Uint32(paquete->buffer);
    return tid;
}

int deserializar_IO(t_paquete* paquete){
    int tiempo = leer_buffer_int(paquete->buffer);
    return tiempo;
}

void* deserializar_mutex(t_paquete* paquete){
    char* recurso = leer_buffer_string(paquete->buffer);
    return recurso;
}