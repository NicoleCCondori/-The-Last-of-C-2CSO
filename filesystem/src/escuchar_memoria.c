#include <escuchar_memoria.h>
#include <f_conexiones.h>

void escuchar_memoria(){
	while(1){

		int op_code_memoria = recibir_operacion(fd_memoria);

		switch(op_code_memoria){
            case DUMP_MEMORY:
            //  
                log_info(FS_logger,"OK");
                crear_archivo();
                break;
            break;
            case MENSAJE:
                //recibir_mensaje(cliente_memoria, logger_FS);
                break;
            case PAQUETE:
                //t_list* lista = recibir_paquete(fd_memoria);
                //log_info(logger_FS, "Me llegaron los siguientes valores:\n");
                //list_iterate(lista, (void*) iterator);
                break;
            case -1:
                log_error(FS_logger, "Desconexion de FS");
                exit(EXIT_FAILURE);
            default:
                log_warning(FS_logger,"Operacion desconocida de FS");
                break;
		}
	}
}

void crear_archivo(){

    t_paquete* archivo_memoria = recibir_paquete(fd_memoria);

    t_crear_archivo_memoria* crear_archivo_memoria = deserializar_crear_archivo_memoria(archivo_memoria);
    
    char* nombre_archivo = crear_archivo_memoria->nombre_archivo;
    char* contenido = crear_archivo_memoria->contenido;
    uint32_t tamanio = crear_archivo_memoria->tamanio;

    uint32_t bloques_necesarios = (tamanio + valores_config_FS->block_size - 1) / valores_config_FS->block_size;

    if(!espacio_disponible(bloques_necesarios)){
        log_info(FS_logger, "No hay espacio suficiente para crear el archivo");
        //se deberá informar a memoria del error y finalizar la creación del archivo en este punto.
        //enviar_mensaje(fd_memoria,"Error: NO hay espacio suficiente para crear el archivo");
    }
    else{

        log_info(FS_logs_obligatorios, "## Archivo Creado: %s - Tamaño: %d",nombre_archivo, tamanio);

        uint32_t bloque_inicial = reservar_bloques(bloques_necesarios, nombre_archivo);
        /*if(bloque_inicial==-1){
            log_error(FS_logger,"ERROR al reservar bloques");
            enviar_mensaje(fd_memoria,"ERROR al reservar bloques");
        }
        */
        escribir_contenido_bloques(bloque_inicial, contenido, bloques_necesarios, tamanio, nombre_archivo);

    }
}

uint32_t cantidad_bloques_libres(FILE *file_bitmap){

    uint32_t bloques_libres = 0;
    unsigned char byte;//Variable para leer el contenido del archivo del bitmap byte a byte.
    
    while (fread(&byte, 1, 1, file_bitmap) == 1) {
        for (int i = 0; i < 8; i++) {
            if (!(byte & (1 << i))) {
                bloques_libres++;
            }
        }
    }
    return bloques_libres;
}
   // Función para verificar si hay espacio suficiente en el bitmap
bool espacio_disponible(uint32_t bloques_necesarios) {
    
    // Leer el bitmap
    FILE* file_bitmap = fopen(bitmap_path, "rb");
    if (!file_bitmap) {
        log_error(FS_logger, "Error al leer el bitmap");
        return false;
    }

    uint32_t bloques_libres = cantidad_bloques_libres(file_bitmap);

    if (bloques_libres >= bloques_necesarios) {
        fclose(file_bitmap);
        return true;
    }
     
    fclose(file_bitmap);
    return false;
}

uint32_t reservar_bloques(uint32_t bloques_necesarios, char *nombre_archivo) {
    FILE* file_bitmap = fopen(bitmap_path, "rb+");
    if (!file_bitmap) {
        log_error(FS_logger, "Error al abrir el bitmap");
        return -1;
    }

    uint32_t bloques_asignados = 0;
    uint32_t primer_bloque = -1;
    unsigned char byte;
    long offset = 0;

    while (fread(&byte, 1, 1, file_bitmap) == 1) {
        for (int i = 0; i < 8; i++) {
            if (!(byte & (1 << i))) {
                // Marcar como ocupado
                byte |= (1 << i); //se utiliza para marcar un bit específico en un byte como ocupado
                fseek(file_bitmap, offset, SEEK_SET);
                fwrite(&byte, 1, 1, file_bitmap);

                // Registro del primer bloque asignado
                if (primer_bloque == -1)
                    primer_bloque = (offset * 8) + i;

                bloques_asignados++;
                if (bloques_asignados == bloques_necesarios) {
                    fclose(file_bitmap);

                    uint32_t bloques_libres = cantidad_bloques_libres(file_bitmap);

                    log_info(FS_logs_obligatorios, "## Bloque asignado: %d- Archivo: %s - Bloques Libres: %d" ,bloques_asignados, nombre_archivo, bloques_libres);
                    
                    return primer_bloque;
                }
            }
        }
        offset++;
    }

    fclose(file_bitmap);
    return -1;
}

void escribir_contenido_bloques(uint32_t primer_bloque, char* contenido, uint32_t bloques_necesarios, uint32_t tamanio, char *nombre_archivo) {
    FILE* file_bloques = fopen(bloques_path, "rb+");
    if (!file_bloques) {
        log_error(FS_logger, "Error al abrir bloques.dat");
        exit(EXIT_FAILURE);
    }

    uint32_t offset = primer_bloque * valores_config_FS->block_size;
    uint32_t bytes_escritos = 0;

    for (uint32_t i = 0; i < bloques_necesarios; i++) {
        fseek(file_bloques, offset, SEEK_SET);
        uint32_t bytes_a_escribir;
        if ((tamanio - bytes_escritos) > valores_config_FS->block_size) {
            bytes_a_escribir = valores_config_FS->block_size;
        } else {
            bytes_a_escribir = (tamanio - bytes_escritos);
        }
        fwrite(contenido + bytes_escritos, 1, bytes_a_escribir, file_bloques);
        bytes_escritos += bytes_a_escribir;
        offset += valores_config_FS->block_size;

        log_info(FS_logs_obligatorios, "Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS/INDICE - Bloque File System %d", 
                 nombre_archivo, offset / valores_config_FS->block_size);
        usleep(valores_config_FS->retardo_acceso_bloque * 1000); // Retardo
    }

    fclose(file_bloques);
}

