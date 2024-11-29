#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <time.h>

#define MAX_FILE_NAME 256
#define MAX_FILES 128
#define MAX_PATH 1024

typedef struct archivo {
	char nombre[MAX_FILE_NAME];
	int idx;
	size_t tamanio;
	void *data;
	time_t fecha_creacion;
	time_t fecha_modificacion;
} archivo_t;

// por ahora lo cree asi. No es un directorio flat porque un directorio puede tener
// subdirectorios, despues podemos cambiarlo para que sea un UNICO nivel de recursion.
// se podria agregar un struct subdirectorio para los subdirectorios dentro del
// directorio raiz, y dejar struct directorio unicamente para el directorio raiz.
typedef struct directorio {
	char nombre[MAX_FILE_NAME];
	int idx;
	// struct directorio *padre;
	struct directorio *subdirectorios[MAX_FILES];
	archivo_t *archivos[MAX_FILES];
	size_t cant_archivos;
	size_t cant_directorios;
	time_t fecha_creacion;
	time_t fecha_modificacion;
} directorio_t;

typedef struct filesystem {
	directorio_t *raiz;
	size_t max_size;
	size_t current_size;
} filesystem_t;

filesystem_t *fs_init();
void fs_destroy(filesystem_t *fs, const char *filename);
int fs_mkdir(filesystem_t *fs, const char *path);
int fs_rmdir(filesystem_t *fs, const char *path);


#endif