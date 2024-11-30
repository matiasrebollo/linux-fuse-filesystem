#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/stat.h>

// PUEDE FALTAR MODULARIZAR LOS DEBUGS.

void
liberar_archivos(archivo_t **archivos, size_t cant_archivos)
{
	if (!archivos) {
		return;
	}
	for (size_t i = 0; i < cant_archivos; i++) {
		if (archivos[i]) {
			free(archivos[i]->data);
			free(archivos[i]);
		}
	}
}

void liberar_directorio(directorio_t *dir);

void
liberar_subdirectorios(directorio_t *subdirectorios[], size_t cant_directorios)
{
	if (!subdirectorios) {
		return;
	}
	for (size_t i = 0; i < cant_directorios; i++) {
		if (subdirectorios[i]) {
			liberar_directorio(subdirectorios[i]);
			// subdirectorios[i] = NULL;
		}
	}
}

void
liberar_directorio(directorio_t *dir)
{
	if (!dir) {
		return;
	}

	liberar_archivos(dir->archivos, dir->cant_archivos);
	liberar_subdirectorios(dir->subdirectorios, dir->cant_directorios);

	free(dir);
}

archivo_t *
deserializar_archivo(FILE *file)
{
	archivo_t *archivo = malloc(sizeof(archivo_t));
	if (!archivo) {
		fprintf(stderr,
		        "[ERROR] Error al asignar memoria para archivo.\n");
		return NULL;
	}

	if (fread(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "[ERROR] Error al leer el nombre del archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->stats->st_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer el tamaño archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->stats->st_atime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de acceso del archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer la fecha de modificacion del archivo.\n");
		free(archivo);
		return NULL;
	}

	if (archivo->stats->st_size > 0) {
		archivo->data = malloc(archivo->stats->st_size);
		if (!archivo->data) {
			fprintf(stderr, "[ERROR] Error al asignar memoria para la data del archivo.\n");
			free(archivo);
			return NULL;
		}
		if (fread(archivo->data, sizeof(char), archivo->stats->st_size, file) !=
		    archivo->stats->st_size) {
			fprintf(stderr,
			        "[ERROR] Error al leer la data del archivo.\n");
			free(archivo->data);
			free(archivo);
			return NULL;
		}
	} else {
		archivo->data = NULL;
	}

	return archivo;
}

directorio_t *
deserializar_directorio(FILE *file)
{
	directorio_t *dir = malloc(sizeof(directorio_t));
	if (!dir) {
		fprintf(stderr,
		        "[ERROR] Error al asignar memoria para directorio.\n");
		return NULL;
	}

	if (fread(dir->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr,
		        "[ERROR] Error al leer el nombre del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->cant_archivos, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la cantidad de archivos del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->cant_directorios, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la cantidad de subdirectorios del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->stats->st_atime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de acceso del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de modificacion del directorio.\n");
		free(dir);
		return NULL;
	}

	memset(dir->archivos, 0, sizeof(dir->archivos));
	memset(dir->subdirectorios, 0, sizeof(dir->subdirectorios));

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		dir->archivos[i] = deserializar_archivo(file);
		if (!dir->archivos[i]) {
			liberar_archivos(dir->archivos, i);
			free(dir);
			return NULL;
		}
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		dir->subdirectorios[i] = deserializar_directorio(file);
		if (!dir->subdirectorios[i]) {
			liberar_archivos(dir->archivos, dir->cant_archivos);
			liberar_subdirectorios(dir->subdirectorios, i);
			free(dir);
			return NULL;
		}
	}

	return dir;
}

filesystem_t *
cargar_fs(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr,
		        "[ERROR] Error al abrir el archivo para cargar.\n");
		return NULL;
	}

	filesystem_t *fs = malloc(sizeof(filesystem_t));
	if (!fs) {
		fprintf(stderr, "[ERROR] Error al asignar memoria para el filesystem.\n");
		fclose(file);
		return NULL;
	}

	if (fread(&fs->max_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer el tamaño maximo del filesystem.\n");
		fclose(file);
		free(fs);
		return NULL;
	}
	if (fread(&fs->current_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer el tamaño actual del filesystem.\n");
		fclose(file);
		free(fs);
		return NULL;
	}

	fs->raiz = deserializar_directorio(file);
	fclose(file);
	if (!fs->raiz) {
		free(fs);
		fprintf(stderr,
		        "[ERROR] Error al crear la raiz del filesystem.\n");
		return NULL;
	}

	return fs;
}

archivo_t *
crear_archivo(const char *nombre, int idx, mode_t mode)
{
    archivo_t *file = malloc(sizeof(archivo_t));
    if (!file) {
        fprintf(stderr,
                "[ERROR] Error al asignar memoria para archivo.\n");
        return NULL;
    }

    // Configurar los campos básicos del archivo
    snprintf(file->nombre, MAX_FILE_NAME, "%s", nombre);
    file->idx = idx;
    file->data = NULL;

    // Inicializar estadísticas del archivo
    file->stats = malloc(sizeof(stats_t));
    if (!file->stats) {
        fprintf(stderr,
                "[ERROR] Error al asignar memoria para estadísticas.\n");
        free(file);
        return NULL;
    }

    file->stats->st_mode = mode;           // Tipo y permisos del archivo
    file->stats->st_nlink = 1;             // Número de enlaces
    file->stats->st_uid = getuid();        // UID del usuario actual
    file->stats->st_gid = getgid();        // GID del usuario actual
    file->stats->st_size = 0;              // Tamaño inicial
    file->stats->st_atime = time(NULL);    // Último acceso
    file->stats->st_mtime = time(NULL);    // Última modificación

    return file;
}

int
fs_create(filesystem_t *fs, const char *path, mode_t mode)
{
    if (!fs || !path || strlen(path) == 0 || strlen(path) >= MAX_PATH) {
        fprintf(stderr, "[ERROR] Path inválido o sistema no inicializado.\n");
        return -1;
    }

    // Verificar si ya existe un archivo con el mismo nombre en la raíz
    for (size_t i = 0; i < fs->raiz->cant_archivos; i++) {
        if (strcmp(fs->raiz->archivos[i]->nombre, path) == 0) {
            fprintf(stderr,
                    "[ERROR] Ya existe un archivo con este nombre en este directorio.\n");
            return -1;
        }
    }

    // Crear el archivo
    archivo_t *nuevo_archivo =
        crear_archivo(path, fs->raiz->cant_archivos, mode);
    if (!nuevo_archivo) {
        fprintf(stderr, "[ERROR] Error al crear el archivo.\n");
        return -1;
    }

    // Agregar el archivo al directorio raíz
    fs->raiz->archivos[fs->raiz->cant_archivos] = nuevo_archivo;
    fs->raiz->cant_archivos++;
    fs->raiz->stats->st_atime = time(NULL);
    fs->raiz->stats->st_mtime = time(NULL);

    // Incrementar el tamaño actual del filesystem
    fs->current_size++;

    return 0;
}




directorio_t *
crear_directorio(const char *nombre, int idx)
{
	directorio_t *dir = malloc(sizeof(directorio_t));
	if (!dir) {
		fprintf(stderr,
		        "[ERROR] Error al asignar memoria para directorio.\n");
		return NULL;
	}
	strncpy(dir->nombre,nombre, MAX_FILE_NAME);
	dir->idx = idx;
	dir->cant_archivos = 0;
	dir->cant_directorios = 0;
	dir->stats = malloc(sizeof(stats_t));
	dir->stats->st_mtime = time(NULL); //tiempo de modif
	dir->stats->st_atime = time(NULL); //tiemo de acceso
	dir->stats->st_gid = getgid();
	dir->stats->st_nlink = 1;
	dir->stats->st_mode = __S_IFDIR;
	dir->stats->st_uid = getuid();
	dir->stats->st_size = sizeof(directorio_t);

	// inicializa los arreglos de subdirectorios y archivos como NULL
	memset(dir->subdirectorios, 0, sizeof(dir->subdirectorios));
	memset(dir->archivos, 0, sizeof(dir->archivos));

	return dir;
}


filesystem_t *
fs_init(const char *filename)
{
	filesystem_t *fs = NULL;

	// Intentar cargar el fs desde el archivo
	FILE *file = fopen(filename, "rb");
	if (file) {
		fs = cargar_fs(filename);
		fclose(file);
		if (!fs) {
			fprintf(stderr, "[ERROR] Error al cargar el filesystem desde el archivo.\n");
			return NULL;
		}
		printf("[DEBUG] Sistema de archivos cargado desde el "
		       "archivo.\n");
	} else {
		fs = malloc(sizeof(filesystem_t));
		if (!fs) {
			fprintf(stderr, "[ERROR] Error al asignar memoria para el filesystem.\n");
			return NULL;
		}
		fs->max_size = 1024 * 1024 * 1024;
		fs->current_size = 0;
		fs->raiz = crear_directorio("/", 0);
		if (!fs->raiz) {
			free(fs);
			fprintf(stderr, "[ERROR] Error al crear la raiz del filesystem.\n");
			return NULL;
		}
		printf("[DEBUG] Nuevo sistema de archivos creado.\n");
	}

	return fs;
}


int
fs_mkdir(filesystem_t *fs, const char *path)
{
	for (size_t i = 0; i < fs->raiz->cant_directorios; i++) {
		if (strcmp(fs->raiz->subdirectorios[i]->nombre, path) == 0) {
			fprintf(stderr,
			        "[ERROR] Ya existe un subdirectorio con este "
			        "nombre en este directorio\n");
			return -1;
		}
	}

	directorio_t *nuevo_dir =
	        crear_directorio(path, fs->raiz->cant_directorios);
	if (!nuevo_dir) {
		fprintf(stderr, "[ERROR] Error al crear el directorio\n");
		return -1;
	}

	fs->raiz->subdirectorios[fs->raiz->cant_directorios] = nuevo_dir;
	fs->raiz->cant_directorios++;
	fs->raiz->stats->st_atime = time(NULL);
	fs->raiz->stats->st_mtime = time(NULL);

	return 0;
}




directorio_t* fs_getdir(filesystem_t *fs, const char *path){
	return obtener_directorio(fs->raiz, path);
}

int
fs_rmdir(filesystem_t *fs, const char *path)
{
	directorio_t *dir = obtener_directorio(fs->raiz, path);
	if (!dir) {
		return -1;
	}

	if (dir->cant_archivos > 0 || dir->cant_directorios > 0) {
		fprintf(stderr, "[ERROR] El directorio no está vacío\n");
		return -1;
	}

	int idx = dir->idx;
	fs->raiz->subdirectorios[idx] =
	        fs->raiz->subdirectorios[fs->raiz->cant_directorios - 1];
	fs->raiz->subdirectorios[fs->raiz->cant_directorios - 1] = NULL;
	fs->raiz->cant_directorios--;

	liberar_directorio(dir);

	return 0;
}


int
serializar_archivo(FILE *file, archivo_t *archivo)
{
	if (fwrite(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el nombre del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->stats->st_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el tamaño del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->stats->st_atime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de acceso del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de modificacion archivo.\n");
		return -1;
	}

	if (archivo->data != NULL && archivo->stats->st_size > 0) {
		if (fwrite(archivo->data, sizeof(char), archivo->stats->st_size, file) !=
		    archivo->stats->st_size) {
			fprintf(stderr, "[ERROR] Error al escribir la data del archivo.\n");
			return -1;
		}
	}
	return 0;
}

int
serializar_directorio(FILE *file, directorio_t *dir)
{
	if (fwrite(dir->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "[ERROR] Error al escribir el nombre del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->cant_archivos, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la cantidad de archivos del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->cant_directorios, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la cantidad de subdirectorios del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->stats->st_atime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de acceso del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de modificacion del directorio.\n");
		return -1;
	}

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		if (serializar_archivo(file, dir->archivos[i]) != 0) {
			fprintf(stderr, "[ERROR] Error al escribir un archivo del directorio.\n");
			return -1;
		}
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		if (serializar_directorio(file, dir->subdirectorios[i]) != 0) {
			fprintf(stderr, "[ERROR] Error al escribir un subdirectorio del directorio.\n");
			return -1;
		}
	}
	return 0;
}

int
guardar_fs(filesystem_t *fs, const char *path)
{
	FILE *file = fopen(path, "wb");
	if (!file) {
		fprintf(stderr,
		        "[ERROR] Error al abrir el archivo para guardar.\n");
		return -1;
	}

	if (fwrite(&fs->max_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el tamaño maximo del fs.\n");
		fclose(file);
		return -1;
	}
	if (fwrite(&fs->current_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el tamaño actual del fs.\n");
		fclose(file);
		return -1;
	}

	if (serializar_directorio(file, fs->raiz) != 0) {
		fprintf(stderr, "[ERROR] Error al escribir el directorio.\n");
		fclose(file);
		return -1;
	}
	fclose(file);

	return 0;
}

void
fs_destroy(filesystem_t *fs, const char *filename)
{
	if (!fs) {
		return;
	}
	if (guardar_fs(fs, filename) != 0) {
		fprintf(stderr, "[ERROR] Error al serializar el file system.\n");
	}

	liberar_directorio(fs->raiz);
	free(fs);

	printf("[DEBUG] Sistema de archivos destruido y guardado.\n");
}

archivo_t* iter_dir(directorio_t* dir, int size,const char* path){
	for(int i = 0; i < dir->cant_archivos; i++){
		if(strcmp(dir->archivos[i]->nombre, path) == 0){
			return dir->archivos[i];
		}
	}
	return NULL;
}

archivo_t* search_file(directorio_t* dir, const char* path){

	archivo_t* f = iter_dir(dir, dir->cant_archivos, path);
	if(f)
		return f;
	for(int i = 0; i < dir->cant_directorios; i++){
		f = search_file(dir->subdirectorios[i], path);
		if(f)
			return f;	
	}
	return NULL;

}
archivo_t* fs_open(filesystem_t* fs, const char* path){
	if(path == NULL || fs == NULL){
		printf("[DEBUG] Nombre de archivo nulo.\n");
		return NULL;
	}

	return search_file(fs->raiz, path);
	
}
stats_t* fs_getattr(filesystem_t *fs, const char *path){
	archivo_t* archivo = search_file(fs->raiz,path);
	if (archivo){
		return archivo->stats;
	}
	directorio_t* directorio = obtener_directorio(fs->raiz, path);
	if(directorio)
		return directorio->stats;
	return NULL;
}
