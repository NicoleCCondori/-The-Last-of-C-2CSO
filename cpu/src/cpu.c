#include <cpu.h>



int main(int argc, char* argv[]) {

    sem_init(&sem_instruccion, 0, 0);
    sem_init(&sem_syscall,0,0);
    sem_init(&sem_syscallKernel,0,0);
    sem_init(&sem_contexto,0,0);

    pthread_mutex_init(&mutex_contextos2, NULL);

    inicializar_cpu();
    
    conectar_memoria();
    conectar_kernel_dispatch();
    conectar_kernel_interrupt();

    //liberar semaforo
    sem_destroy(&sem_instruccion);
    sem_destroy(&sem_syscall);
    pthread_mutex_destroy(&mutex_contextos2);
    sem_destroy(&sem_syscallKernel);
    sem_destroy(&sem_contexto);

    //liberar los logs config y semaforos
    finalizar_modulo(cpu_logger,cpu_logs_obligatorios,valores_config_cpu->config);
    free(valores_config_cpu);
    
    //finalizar las conexiones
    close(fd_cpu_dispatch);
    close(fd_cpu_interrupt);
    close(fd_kernel_dispatch);
    close(fd_kernel_interrupt);
    close(fd_memoria);

    return 0;
}