#include <planificadores.h>


//semaforos


void planificador_de_largo_plazo()
{   
    while(1){
        //obtener de la cola de new el proceso
        sem_wait(&sem_plani_largo_plazo);

        //mandar a la cola de ready
        iniciar_proceso();
        

    }
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
            TCB* hiloMayorPrioridad = buscar_hilo_menorNro_prioridad(); //Acá esta el hilo de mayor prioridad
            
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

            printf("Planificacion CMN\n");

            int mayorNroPrioridad = buscar_hilo_mayorNroPrioridad(); //devuelve el numero mas grande, menor prioridad 
            //1)SE crea una cola por cada nivel de prioridad que exista 

            t_queue* cola_prioridad[mayorNroPrioridad+1];

            for(int i=0; i <= mayorNroPrioridad; i++){
                if(cola_prioridad[i] == NULL){
                    cola_prioridad[i] = queue_create();
                }
            }

            for(int i=0; i< list_size(lista_ready); i++){
                TCB* hilo = list_get(lista_ready, i);
                if(hilo->prioridad <= mayorNroPrioridad){
                    queue_push(cola_prioridad[hilo->prioridad], hilo);
                }
                //ejecutar roundrobin para cada cola, hacer funcion aparte ( cola);
            }

        }*/
    }
}
