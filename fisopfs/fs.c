#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

filesystem_t *fs_init(){
    filesystem_t *fs = malloc(sizeof(filesystem_t));
    if (!fs){
        fprintf(stderr, "[ERROR] No se pudo crear el filesystem\n");
        return NULL;
    }

    fs->raiz = malloc(sizeof(directorio_t));
    if (!fs->raiz){
        fprintf(stderr, "[ERROR] No se pudo crear el directorio raiz\n");
        free(fs);
        return NULL;
    }

    strncpy(fs->raiz->nombre, "/", MAX_FILE_NAME);
    fs->raiz->padre = NULL;
    fs->raiz->cant_archivos = 0;
    fs->raiz->cant_directorios = 0;
    fs->raiz->fecha_creacion = time(NULL);
    fs->raiz->fecha_modificacion = time(NULL);

    fs->max_size = 1024 * 1024 * 1024; //1GB despues podemos cambiarlo
    fs->current_size = 0;

    return fs;
}

void liberar_directorio(directorio_t *dir){
    if (!dir){
        return;
    }

    for (size_t i = 0; i < dir->cant_archivos; i++){
        free(dir->archivos[i]->data);
        free(dir->archivos[i]);
    }

    for (size_t i = 0; i < dir->cant_directorios; i++){
        liberar_directorio(dir->subdirectorios[i]);
    }
    
    free(dir);
}

void fs_destroy(filesystem_t *fs){
    if (!fs){
        return;
    }

    if (fs->raiz){
        liberar_directorio(fs->raiz);
    }

    free(fs);
}