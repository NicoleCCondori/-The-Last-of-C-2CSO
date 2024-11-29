#include <filesystem.h>

int main(int argc, char* argv[]) {

    inicializar_FS();
    fd_FS = iniciar_servidor (valores_config_FS->puerto_escucha, FS_logger, "Servidor FS iniciado");
    inicializar_bitmap();
    inicializar_bloques();
    
    conectar_memoria();

    //liberar los logs y config
    finalizar_modulo(FS_logger, FS_logs_obligatorios, valores_config_FS->config);
    free(valores_config_FS);

    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);
    return EXIT_SUCCESS;
}

void inicializar_bitmap(){

    int bitmap_size=(valores_config_FS->block_count +7 )/8;
 
 
    //es asi? o coomo con bloques? no me cuadra lo del filepath
    char bitmap_path[100];
    sprintf(bitmap_path,"%s/bitmap.dat",valores_config_FS->mount_dir);


   FILE* file_bitmap=fopen(bitmap_path,"wb");//Probar lo de b o d  ""rb+""
   if(!file_bitmap){
    log_error(FS_logger,"Erorr al crear el archivo bitmap");
    exit (EXIT_FAILURE);
   }
   //escribo 0 para inicializar el archivo, lo vi por google no estoy seguro
    unsigned char byte_vacio=0;
    for (int i = 0; i < bitmap_size; i++)
    {
        fwrite(&byte_vacio,sizeof(unsigned char),1,file_bitmap);
    }
    
    fclose(file_bitmap);
    log_info(FS_logger,"Bitmap inicializado correctamente");
    log_info(FS_logs_obligatorios,"Creacion Archivo: ## Archivo Creado: <%s> - tamanio: <%d> ", bitmap_path, bitmap_size );
}
void inicializar_bloques(){

    char bloques_path[100];
    sprintf(bloques_path,"%s/bloques.dat",valores_config_FS->mount_dir);
    FILE* file_bloques=fopen(bloques_path,"wb");
    
    if (!file_bloques)
    {
        log_error(FS_logger,"Erorr al crear el archivo");
        exit (EXIT_FAILURE);
    }

    fseek(file_bloques,valores_config_FS->block_count*valores_config_FS->block_size-1,SEEK_SET);
    
    fputc('\0',file_bloques);

    fclose(file_bloques);
    log_info(FS_logger,"Archivo de bloques inicializado correctamente");
    


}
void inicializar_metadata(uint32_t size, uint32_t index_block,char* nombre_archivo){


    char metadata_path[1024];
    sprintf(metadata_path,"%s/files/%s",valores_config_FS->mount_dir,nombre_archivo);

    FILE* metadata_file=fopen(metadata_path,"wb");
    if (!metadata_file)
    {
        log_error(FS_logger,"Erorr al crear el archivo");
        exit (EXIT_FAILURE);
    }

    fprintf(metadata_file,"SIZE=%d",size);
    fprintf(metadata_file,"INDEX_BLOCK=%d",index_block);
    
    fclose(metadata_file);
    log_info(FS_logger,"Archivo metadata inicializado correctamente");
}

int reservar_bloques (uint32_t bloques_necesarios, uint32_t* bloques_reservados){
    char bitmap_path[100];
    sprintf(bitmap_path,"%s/bitmap.dat",valores_config_FS->mount_dir);

    FILE* file_bitmap=fopen(bitmap_path,"rb+");
    if (!file_bitmap)
    {
        log_error(FS_logger,"Error al abrir el archivo bitmap impidiendo verificar el espacio");
        return -1;
    }
    uint8_t byte;
    uint32_t bloques_libres=0;
    uint32_t block_count=valores_config_FS->block_count;
    uint32_t bitmap_size=(block_count+7)/8;

    for (uint32_t i= 0; i <bitmap_size; i++)
    {
        fread(&byte,sizeof(uint8_t),1,file_bitmap);
        for (uint8_t j = 0; i < 8; j++)
        {
            uint32_t bloque_actual=i*8+j;

            if (bloque_actual>=block_count)
            {
                break;//evitamos leer fuera del rango
            }

            if(!(byte & (1<<j)))//lee el anterior a la posicion de j, es para verificar si el bloque esta libre.
            {
                bloques_reservados[bloques_libres++]=bloque_actual;

                byte|=(1<<j);// reemplazo con OR bit a bit para marcarlo como ocupado
                fseek(file_bitmap,-1,SEEK_CUR)//retroceder una posicion para actualizar el byte
                fwrite(&byte,sizeof(uint8_t),1,file_bitmap);
                fflush(file_bitmap); //no estoy seguro de eso, lo vi en google y es para asegurarme que los cambios se guardan rapido en disco

                if (bloques_libres==bloques_necesarios)
                {
                    fclose(file_bitmap);
                    return 1;// Se reservaron los bloques
                }
                
            }
        }
    }
    
    fclose(file_bitmap);
    return 0;
}

void dump_memory(char* nombre,uint32_t tam,char* contenido){
    uint32_t bloques_necesarios=tamvalores_config_FS->block_size;// me da la cantidad de bloques
    uint32_t* bloques_reservados= malloc(bloques_necesarios*sizeof(uint32_t));

    if(!bloques_reservados){
        log_error(FS_logger,"Error al reservar memoria");
        exit(EXIT_FAILURE);
    }
    if (reservar_bloques(bloques_necesarios,bloques_reservados)=-1)
    {
        log_error(FS_logger,"No hay espacio suficiente para el archivo %s",nombre);
        free(bloques_reservados);
        exit(EXIT_FAILURE);
    }

    char bloques_path[100];
    sprintf(bloques_path,"%s/bloques.dat",valores_config_FS->mount_dir);
    FILE* file_bloques=fopen(bloques_path,"rb+");
    if(!file_bloques){
        log_error(FS_logger,"Error al abrir el archivo de bloques para escritura");
        free(bloques_reservados);
        exit(EXIT_FAILURE);

    }
    
    fseek(file_bloques,bloques_reservados[0]*valores_config_FS->block_size,SEEK_SET);//me escribo los punteros en el bloque indice
    for (uint32_t i = 0; i <= bloques_necesarios; i++)
    {
        fwrite(&bloques_reservados[i],sizeof(uint32_t),1,file_bloques);
    }
    uint32_t bytes_escritos=0;
    for (uint32_t i = 0; i <= bloques_necesarios; i++)
    {
        fseek(file_bloques,bloques_reservados[i]*valores_config_FS->block_size,SEEK_SET);
        uint32_t cant_bytes_a_escribr= (tam-bytes_escritos > valores_config_FS->block_size ? valores_config_FS->block_size : tam-bytes_escritos );
        fwrite(contenido + bytes_escritos,sizeof(char),cant_bytes_a_escribr,file_bloques);
        bytes_escritos+=cant_bytes_a_escribr;

        log_info(FS_logger,"## Acceso Bloque - Archivo %s - Tipo Bloque:<DATOS> - Bloque File System <%d>",nombre,,,bloques_reservados[i]);

        bytes_escritos+= cant_bytes_a_escribr;

        usleep(valores_config_FS->retardo_acceso_bloque*1000);

    }
    fclose(file_bloques);

    log_info(FS_logger,"## Fin de solicitud - Archivo: %s ",nombre);
}