#include <filesystem.h>


int main(int argc, char* argv[]) {

    inicializar_FS();

    conectar_memoria();

    //finalizar_conexiones(1, cliente_memoria);
    //finalizar_modulo(logger_FS, valores_config_FS->config);
    log_destroy(FS_logger);
    free(valores_config_FS);
    
    return EXIT_SUCCESS;
}

void inicializar_bitmap(){
    
    uint32_t bitmap_size=(BLOCK_COUNT )/8;//que seria block_count?
    bitmap_size+=7;//redondeo para arriba
    //es asi? o coomo con bloques? no me cuadra lo del filepath
    char file_path[100];
    sprintf(file_path,sizeof(file_path),%s/bitmap.dat,MOUNT_DIR);//que seria mount dir?


   FILE* file_bitmap=fopen(file_path,"wd");
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
    log_info(FS_logger,"Bitmap inicializado")
   
}
void inicializar_bloques(){

    FILE* file_bloques=fopen("bloques.dat","wd");
    
    if (!file_bloques)
    {
        log_error(FS_logger,"Erorr al crear el archivo");
        exit (EXIT_FAILURE);
    }
    fseek(file_bloques,BLOCK_COUNT*BLOCK_SIZE-1,SEKK_SET);
    fputc('\0',file_bloques);

    fclose(file_bloques);
    log_info(FS_logger,"Archivo de bloques inicializado");

}