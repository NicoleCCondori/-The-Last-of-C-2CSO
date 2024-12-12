#include <filesystem.h>


char *bitmap_path;

char *bloques_path;

char *metadata_path;

int main(int argc, char* argv[]) {

    inicializar_FS();
    fd_FS = iniciar_servidor (valores_config_FS->puerto_escucha, FS_logger, "Servidor FS iniciado");
    
    inicializar_bitmap();
    inicializar_bloques();
    //inicializar_metadata(1024, 1,"hola.txt");//hay que llamarla cada vez que se necesite crear un archivo
    conectar_memoria();

    //liberar los logs y config
    finalizar_modulo(FS_logger, FS_logs_obligatorios, valores_config_FS->config);
    free(valores_config_FS);

    //finalizar las conexiones
    close(fd_memoria);
    close(fd_FS);

    //liberar memoria
    free(bitmap_path);
    free(bloques_path);
    free(metadata_path);

    return EXIT_SUCCESS;
}

void inicializar_bitmap(){

    int bitmap_size=(valores_config_FS->block_count +7 )/8;
    

    struct stat st = {0};

    
    
    // Specify the permissions (rwxr-xr-x)
    //mode_t mode = 0755;
    bitmap_path = malloc(strlen(valores_config_FS->mount_dir) + 15);

    if(stat(valores_config_FS->mount_dir, &st) == -1) { //el directorio no existe
        if(mkdir(valores_config_FS->mount_dir, 0755) == 0) {
            //es asi? o coomo con bloques? no me cuadra lo del filepath
            
            sprintf(bitmap_path,"%s/bitmap.dat",valores_config_FS->mount_dir);
            printf("Directorio creado exitosamente, el directorio es: %s\n",bitmap_path);
            
        } 
    } else {//el directorio existe

            //bitmap_path = valores_config_FS->mount_dir;
            sprintf(bitmap_path,"%s/bitmap.dat",valores_config_FS->mount_dir);
            printf("El directorio ya existe, el directorio es: %s\n",bitmap_path);
    }


   FILE* file_bitmap=fopen(bitmap_path,"wb");//Probar lo de b o d  ""rb+""
   
   if(!file_bitmap){
    log_error(FS_logger,"Error al crear el archivo bitmap");
    exit (EXIT_FAILURE);
   }
   //escribo 0 para inicializar el archivo
    unsigned char byte_vacio=0;
    for (int i = 0; i < bitmap_size; i++)
    {
        fwrite(&byte_vacio,sizeof(unsigned char),1,file_bitmap);
    }
    
    fclose(file_bitmap);
    log_info(FS_logger,"Bitmap inicializado correctamente");

    //log_info(FS_logs_obligatorios,"Creacion Archivo: ## Archivo Creado: %s - tamanio: %d ", bitmap_path, bitmap_size );
}

void inicializar_bloques(){

    bloques_path = malloc(strlen(valores_config_FS->mount_dir) + 15);

    sprintf(bloques_path,"%s/bloques.dat",valores_config_FS->mount_dir);

    FILE* file_bloques=fopen(bloques_path,"wb");
    
    if (!file_bloques)
    {
        log_error(FS_logger,"Error al crear el archivo");
        exit (EXIT_FAILURE);
    }

    fseek(file_bloques,valores_config_FS->block_count*valores_config_FS->block_size-1,SEEK_SET);
    
    fputc('\0',file_bloques);

    fclose(file_bloques);
    log_info(FS_logger,"Archivo de bloques inicializado correctamente");
}

void inicializar_metadata(uint32_t size, uint32_t index_block,char* nombre_archivo){
    
    //printf("nombre archivo: %s\n", nombre_archivo);
    metadata_path = malloc(strlen(valores_config_FS->mount_dir) + 20);

    sprintf(metadata_path,"%s/files",valores_config_FS->mount_dir);
    //printf("path: %s\n", metadata_path);
    
    struct stat st = {0};

    if(stat(metadata_path, &st) == -1) { //el directorio no existe
        if(mkdir(metadata_path, 0755) == 0) {
            //es asi? o coomo con bloques? no me cuadra lo del filepath
            
            sprintf(metadata_path,"%s/files/%s",valores_config_FS->mount_dir,nombre_archivo);
            printf("Directorio creado exitosamente, el directorio es: %s\n",metadata_path);
            
        } 
    } else {//el directorio existe

            //bitmap_path = valores_config_FS->mount_dir;
            sprintf(metadata_path,"%s/files/%s",valores_config_FS->mount_dir,nombre_archivo);
            printf("El directorio ya existe, el directorio es: %s\n",metadata_path);
    }
    
    

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
