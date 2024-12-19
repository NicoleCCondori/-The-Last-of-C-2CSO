#include <kernel_escucha_memoria.h>


void kernel_escucha_memoria(){
    //atender los msjs de memoria
    bool control_key = 1;
    while (control_key){

		t_paquete* paquete_kernel = recibir_paquete(fd_memoria);
		log_info(kernel_logger, "op code paquete: %u",  paquete_kernel->codigo_operacion);
		switch (paquete_kernel->codigo_operacion)
		{
			case MENSAJE:
				break;
			case PAQUETE:
				log_info(kernel_logger,"enum paquete");
				break;
			case CONFIRMAR_ESPACIO_PROCESO:
				t_asigno_memoria* datos_proceso_memoria = deserializar_proceso_memoria(paquete_kernel);
			
				log_info(kernel_logger, "Contenido del bit de confirmacion: %u",  datos_proceso_memoria->bit_confirmacion);

				if(datos_proceso_memoria==NULL){
					log_error(kernel_logger,"ERROR al deserializar el paquete");
					break;
				}

				if(datos_proceso_memoria->bit_confirmacion == 1){
					
					log_info(kernel_logger,"El pid es -> %u", datos_proceso_memoria->pid);
					crear_hilo(datos_proceso_memoria->pid);
					sem_post(&sem_plani_largo_plazo);

				
				} else {
					log_warning(kernel_logger,"No hay espacio en memoria\n");
					sem_post(&sem_plani_largo_plazo);
					//sem_wait(&sem_binario_memoria);
					//Se mantiene en el estado NEW
				}
				log_info(kernel_logger,"estoy entrando aca depues de que mamoria me da un ok de contexto");
				eliminar_paquete(paquete_kernel);
				// Señalar al receptor que los datos están disponibles
				sem_post(&semaforo_binario);  // Libera el semáforo binario
				break;
			
			case CONFIRMAR_CREACION_HILO:
					log_info(kernel_logger, "ok se creo el hilo");

					t_creacion_hilo* datos_hilo_memoria = deserializar_creacion_hilo_memoria(paquete_kernel);
					log_info(kernel_logger, "Contenido del bit de confirmacion: %u",  datos_proceso_memoria->bit_confirmacion);


					if(datos_hilo_memoria==NULL){
						log_error(kernel_logger,"ERROR al deserializar el paquete creacion del hilo");
						break;
					}

					if(datos_hilo_memoria->bit_confirmacion == 1){
						log_info(kernel_logger,"El pid es: %u y el tid es: %u", datos_hilo_memoria->pid, datos_hilo_memoria->tid);
						confirmacion_crear_hilo(datos_hilo_memoria->pid, datos_hilo_memoria->tid);
						sem_post(&TCBaPlanificar);
        				//planificador_corto_plazo(hilo_main);
						planificador_corto_plazo();
					}else{
						log_warning(kernel_logger,"No se pudo crear el hilo\n");
					}
					//sem_post(&sem_binario_memoria);
					eliminar_paquete(paquete_kernel);
					
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de MEMORIA");
				exit(EXIT_FAILURE);
			default:
				log_warning(kernel_logger, "Operacion desconocida de MEMORIA");
				break;
		}
	}
}