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
