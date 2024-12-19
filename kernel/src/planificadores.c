#include <planificadores.h>


//semaforos


void planificador_de_largo_plazo()
{
    //sem_init(&sem_binario_memoria, 0, 0);// semaforo binario para otorgarle espacio de memoria a un proceso nuevo
    //sem_init(&sem_binario_memoria, 1, 0);//es uno porque lo compartiria con memoria
    //sem_init(&sem_mutex_cola_ready,0,1);//mutex de cola de ready
    //inicializar semáforos
    sem_init(&TCBaPlanificar, 0, 0);
    
    while(1){
        //obtener de la cola de new el proceso
        sem_wait(&sem_plani_largo_plazo);
        iniciar_proceso();
        //mandar a la cola de ready
        //int prioridad = 0;
        //crear_proceso(tamanio_proceso_main, archivo_pseudocodigo_main, prioridad); llamar en kernel
       
    }
}

void planificador_corto_plazo(/*TCB* hilo*/){
    while(1){
        sem_wait(&TCBaPlanificar); //mientras haya tcbs en la lista_ready
        if(strcmp(valores_config_kernel->algoritmo_planificacion,"FIFO")==0){
            printf("Planificacion fifo\n");
            
           // sem_wait(&sem_mutex_cola_ready); LO COMENTO PORQUE NO SE INICIALIZA EN NIGNUN LADO - REVISAR
            TCB* hilo_exec = list_remove(lista_ready,0);
            log_info(kernel_logger, "saco el primer elemento de la lista");
            // sem_post(&sem_mutex_cola_ready);LO COMENTO PORQUE NO SE INICIALIZA EN NIGNUN LADO - REVISAR

            hilo_exec->estadoHilo = EXEC;//Cambio el hilo a estado execute
            log_info(kernel_logger, "Cambiar el estado del hilo");
            queue_push(cola_exec, hilo_exec);//Lo agrego en la cola_exec

            t_paquete* hilo_cpu = crear_paquete(RECIBIR_TID);//mandar a cpu tid y pid
            serializar_hilo_cpu(hilo_cpu, hilo_exec->pid, hilo_exec->tid);
            log_info(kernel_logger, "Envio paquete a cpu");
            enviar_paquete(hilo_cpu, fd_cpu_dispatch);
            eliminar_paquete(hilo_cpu);

            log_info(kernel_logger,"Envio a cpu el pid <%d> y tid <%d>", hilo_exec->pid, hilo_exec->tid); 
        }

        if(strcmp(valores_config_kernel->algoritmo_planificacion,"PRIORIDADES")==0){
            printf("Planificación prioridades\n");
            //La máxima prioridad es el 0
            //1ro saber cuál es la menor prioridad ---> Menor le decimos al numero entero --> En sí sería la de MAXIMA PRIORIDAD
            TCB* hiloMayorPrioridad =buscar_hilo_menorNro_prioridad(); //Acá esta el nro entero más grande
            
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
        }

        if(strcmp(valores_config_kernel->algoritmo_planificacion,"CMN")==0){
            printf("Planificacion CMN\n");
            int mayorNroPrioridad = buscar_hilo_mayorNroPrioridad(); //SIrve para saber la cantidad de colas a crear
            t_queue* cola_prioridad[mayorNroPrioridad+1];
            for(int i=0; i <= mayorNroPrioridad; i++){
                cola_prioridad[i] = queue_create();
            }
            for(int i=0; i< list_size(lista_ready); i++){
                TCB* hilo = list_get(lista_ready, i);
                if(hilo->prioridad <= mayorNroPrioridad){
                    queue_push(cola_prioridad[hilo->prioridad], hilo);
                }
                //ejecutar roundrobin para cada cola, hacer funcion aparte ( cola);
            }

        }
    }
}
