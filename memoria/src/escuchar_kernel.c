#include <escuchar_kernel.h>
#include <m_conexiones.h>

void escuchar_kernel(){
    //atender los msjs de KERNEL , otra funcion?
  
    bool control_key = 1;
    while (control_key){

		int cod_op = recibir_operacion(fd_kernel);
		//debemos extraer el resto, primero el tamaño y luego el contenido
			t_paquete* paquete = malloc(sizeof(t_paquete));
			paquete->buffer = malloc(sizeof(t_buffer));
			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);			
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);
			
		switch (cod_op)
		{
			case ASIGNAR_MEMORIA:
				printf("HAY ESPACIO??\n");
				break;
			case MENSAJE:

				break;
			case PAQUETE:

				break;
			case -1:
				log_error(memoria_logger, "Desconexion de KERNEL");
				exit(EXIT_FAILURE);
			default:
				log_warning(memoria_logger, "Operacion desconocida de KERNEL");
				break;
		}
	}

}