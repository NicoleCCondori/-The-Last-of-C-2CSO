#include <planificadores.h>



void planificador_de_largo_plazo()
{   
    //while(1){
        sem_wait(&sem_plani_largo_plazo);

        log_info(kernel_logger,"Entro en la funcion plani de largo plazo");
    
        PCB* proceso_new = malloc(sizeof(PCB));//liberar

        if (cola_new != NULL && !queue_is_empty(cola_new)) {
            proceso_new = queue_peek(cola_new);
            log_info(kernel_logger, "Proceso encontrado en cola NEW: PID %d", proceso_new->pid);
            asignar_espacio_memoria(proceso_new->pid, proceso_new->tam_proceso, proceso_new->prioridad_main, proceso_new->path_main);
        }
        else {
            log_info(kernel_logger, "Cola new esta vacia o es igual a NULL ");
        }

   // }
    /*while(1){
        //obtener de la cola de new el proceso
        sem_wait(&sem_plani_largo_plazo);
        log_info(kernel_logger,"entro al plani de largo plazo");
        //mandar a la cola de ready
        iniciar_proceso();
    }*/
}

void planificador_corto_plazo(/*TCB* hilo*/){
    while(1){
        sem_wait(&TCBaPlanificar); //mientras haya tcbs en la lista_ready
        if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){

            printf("Planificacion fifo\n");
            
            TCB* hilo_exec = list_remove(lista_ready,0);

            log_info(kernel_logger, "Saco el primer hilo de la lista ready, TID: %d y PID: %d", hilo_exec->tid, hilo_exec->pid);


            hilo_exec->estadoHilo = EXEC;//Cambio el hilo a estado execute
            log_info(kernel_logger, "Cambiar el estado del hilo de READY A EXEC, TID: %d", hilo_exec->tid);

            queue_push(cola_exec, hilo_exec);//Lo agrego en la cola_exec

            //voy a imprmir el paquete que serializamos, dale?
            t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);//mandar a cpu tid y pid
            serializar_hilo_cpu(hilo_cpu, hilo_exec->pid, hilo_exec->tid);
            //log_info(kernel_logger, "Envio paquete a cpu ");
            enviar_paquete(hilo_cpu, fd_cpu_dispatch);
            
            eliminar_paquete(hilo_cpu);

            log_info(kernel_logger,"Envio a cpu el pid <%d> y tid <%d>", hilo_exec->pid, hilo_exec->tid); 

        }

        if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){

            printf("Planificación prioridades\n");
            //La máxima prioridad es el 0
            //1ro saber cuál es la mayor prioridad ---> Menor numero entero --> En sí sería la de MAXIMA PRIORIDAD
            TCB* hiloMayorPrioridad = buscar_hilo_menorNro_prioridad(); //busca el hilo de mayor prioridad y lo elimina de la lista de ready
            if(hiloMayorPrioridad==NULL){
                log_info(kernel_logger,"No se encontro un tcb a la lista_ready");
            }
            //Ahora el hiloMayorPrioridad es el hilo que tiene como prioridad el numero entero más chico
            //CAmbiar el estado del hilo
            hiloMayorPrioridad->estadoHilo = EXEC;
            //LO agrego en la cola_exec
            queue_push(cola_exec,hiloMayorPrioridad);

            //mandar a cpu tid y pid
            t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);
            serializar_hilo_cpu(hilo_cpu, hiloMayorPrioridad->pid, hiloMayorPrioridad->tid);
            enviar_paquete(hilo_cpu, fd_cpu_dispatch);
            eliminar_paquete(hilo_cpu);
            log_info(kernel_logger,"Envio a cpu el pid <%d> y tid <%d>", hiloMayorPrioridad->pid, hiloMayorPrioridad->tid); 
        }

        /*if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){

            log_info(kernel_logger,"Planificacion CMN\n");

            int mayorNroPrioridad = buscar_hilo_mayorNroPrioridad(); //devuelve el numero mas grande, menor prioridad 
            log_info(kernel_logger,"El numero de colas creadas son: %d", mayorNroPrioridad);

            //1)SE crea una cola por cada nivel de prioridad que exista 


                // Asignamos memoria para el array de colas si es NULL
                if (cola_prioridad == NULL) {
                    cola_prioridad = malloc((mayorNroPrioridad + 1) * sizeof(t_queue*));
                }

                for (int i = 0; i <= mayorNroPrioridad; i++) {
                    if (cola_prioridad[i] == NULL) {
                        log_info(kernel_logger, "Se crea la cola: %d", i);
                        cola_prioridad[i] = queue_create(); // Crear la cola si no existe
                    }
                }

            //2)encolar el hilo en la cola de prioridad correspondiente
            //sem_wait(&mutex);
            for(int i=0; i< list_size(lista_ready); i++){
                TCB* hilo = list_get(lista_ready, i);
                if(hilo->prioridad <= mayorNroPrioridad){
                    queue_push(cola_prioridad[hilo->prioridad], hilo);
                    list_remove(lista_ready, i);
                    i--; // Ajustar el índice después de eliminar un elemento
                }else{
                    log_error(kernel_logger,"la prioridad del hilo es mayor que el mayorNroPrioridad");
                }
            }
            //sem_post(&mutex);
            
            //3)Buscamos la cola de mayor prioridad a ejecutar con rr
            t_queue* colaMayorPrioridad = buscarColaMayorPrioridad(cola_prioridad[], mayorNroPrioridad); //prioridad ==O o el mas chiquito

            planificar_RR(colaMayorPrioridad);

        }*/
    }
}
/*
t_queue* buscarColaMayorPrioridad(t_queue* cola_prioridad[], int mayorNroPrioridad) {
    // Iteramos sobre las colas en el arreglo y buscamos la cola con la mayor prioridad (índice más bajo con elementos)
    for (int i = 0; i <= mayorNroPrioridad; i++) {
        if (cola_prioridad[i] != NULL && !queue_is_empty(cola_prioridad[i])) {
            // Si la cola no es NULL y tiene elementos, la retornamos como la de mayor prioridad
            log_info(kernel_logger, "Se encontró cola con prioridad: %d", i);
            return cola_prioridad[i];
        }
    }
    
    // Si no se encuntra ninguna cola con elementos
    log_error(kernel_logger, "No hay colas con hilos listos para ejecutar");
    return NULL;
}

*/

/*
void planificar_RR(t_queue* colaMayorPrioridad)
{
    TCB *hilo = malloc(sizeof(t_pcb));
    while (1)
    {
        proceso = transicion_ready_exec();
        log_info(logger, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>", proceso->cde->pid);
        proceso->estado = EXEC;
        enviar_a_cpu_cde(proceso->cde);

        inicio_quantum();
        //busco en la lista de tcb

        //busco el hilo siguiente de la lista de ready 

        //comparo las prioridades del hilo que me pasan por interrupt y de la lista de ready

        //si tienen la misma prioridad, entoces sigue el que estaba en la lista de ready
            //si no hay otro sigue mismo ejecuntado

        /si tienen distinta prioridad
                        //si el sigu
        }
    }
}
*/

/*
// QUANTUM

void inicio_quantum()
{   
    pthread_t hiloQuantum;
    pthread_create(&hiloQuantum, NULL, hilo_quantum, NULL);
    pthread_join(hiloQuantum, NULL);
}
*/
/*
void *hilo_quantum()
{
    int quantum = atoi(valores_config_kernel->quantum);

    sleep_ms(quantum*1000);

    enviar_op_code(socket_cpu_interrupt, INTERRUMPIR);
}
*/
