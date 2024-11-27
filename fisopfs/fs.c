#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

void
serializar_archivo(FILE *file, archivo_t *archivo)
{
	if (fwrite(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&archivo->tamanio, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&archivo->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&archivo->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}

	if (archivo->data != NULL && archivo->tamanio > 0) {
		if (fwrite(archivo->data, sizeof(char), archivo->tamanio, file) !=
		    archivo->tamanio) {
			fprintf(stderr, "Error al escribir el archivo.\n");
			return;
		}
	}
}

void
serializar_directorio(FILE *file, directorio_t *dir)
{
	if (fwrite(dir->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&dir->cant_archivos, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&dir->cant_directorios, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&dir->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}
	if (fwrite(&dir->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return;
	}

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		serializar_archivo(file, dir->archivos[i]);
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		serializar_directorio(file, dir->subdirectorios[i]);
	}
}

int
guardar_fs(filesystem_t *fs, const char *path)
{
	FILE *file = fopen(path, "wb");
	if (!file) {
		fprintf(stderr, "Error al abrir el archivo para guardar.\n");
		return -1;
	}

	// Serializamos la raíz del sistema de archivos
	if (fwrite(&fs->max_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return -1;
	}
	if (fwrite(&fs->current_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al escribir el archivo.\n");
		return -1;
	}

	serializar_directorio(file, fs->raiz);
	fclose(file);

	return 0;
}

archivo_t *
deserializar_archivo(FILE *file)
{
	archivo_t *archivo = malloc(sizeof(archivo_t));

	if (fread(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&archivo->tamanio, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&archivo->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&archivo->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}

	if (archivo->tamanio > 0) {
		archivo->data = malloc(archivo->tamanio);
		if (fread(archivo->data, sizeof(char), archivo->tamanio, file) !=
		    archivo->tamanio) {
			fprintf(stderr, "Error al leer el archivo.\n");
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

	if (fread(dir->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&dir->cant_archivos, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&dir->cant_directorios, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&dir->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&dir->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		dir->archivos[i] = deserializar_archivo(file);
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		dir->subdirectorios[i] = deserializar_directorio(file);
	}

	return dir;
}

filesystem_t *
cargar_fs(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Error al abrir el archivo para cargar.\n");
		return NULL;
	}

	filesystem_t *fs = malloc(sizeof(filesystem_t));

	if (fread(&fs->max_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}
	if (fread(&fs->current_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer el archivo.\n");
		return NULL;
	}

	fs->raiz = deserializar_directorio(file);
	fclose(file);

	return fs;
}

/*
archivo_t *
crear_archivo(const char *nombre, size_t tamanio)
{
        archivo_t *archivo = malloc(sizeof(archivo_t));
        strncpy(archivo->nombre, nombre, MAX_FILE_NAME);
        archivo->tamanio = tamanio;
        archivo->data = malloc(tamanio);
        memset(archivo->data, 0, tamanio);  // De prueba
        archivo->fecha_creacion = time(NULL);
        archivo->fecha_modificacion = time(NULL);
        return archivo;
}*/

directorio_t *
crear_directorio(const char *nombre)
{
	directorio_t *dir = malloc(sizeof(directorio_t));
	snprintf(dir->nombre, MAX_FILE_NAME, "%s", nombre);
	dir->cant_archivos = 0;
	dir->cant_directorios = 0;
	dir->fecha_creacion = time(NULL);
	dir->fecha_modificacion = time(NULL);

	// inicializa los arreglos de subdirectorios y archivos como NULL
	memset(dir->subdirectorios, 0, sizeof(dir->subdirectorios));
	memset(dir->archivos, 0, sizeof(dir->archivos));

	return dir;
}

void
agregar_archivo(directorio_t *dir, archivo_t *archivo)
{
	if (dir->cant_archivos < MAX_FILES) {
		dir->archivos[dir->cant_archivos] = archivo;
		dir->cant_archivos++;
	}
}

filesystem_t *
fs_init(const char *filename)
{
	filesystem_t *fs = malloc(sizeof(filesystem_t));

	// Intentar cargar el fs desde el archivo
	FILE *file = fopen(filename, "rb");
	if (file) {
		fs = cargar_fs(filename);
		fclose(file);
		printf("Sistema de archivos cargado desde el archivo.\n");
	} else {
		fs->max_size = 1024 * 1024 * 1024;
		fs->current_size = 0;
		fs->raiz = crear_directorio("/");
		printf("Nuevo sistema de archivos creado.\n");
	}

	return fs;
}

void
liberar_archivos(archivo_t **archivos, size_t cant_archivos)
{
	for (size_t i = 0; i < cant_archivos; i++) {
		if (archivos[i]) {
			free(archivos[i]->data);
			free(archivos[i]);
		}
	}
}

void liberar_directorios(directorio_t *subdirectorios[], size_t cant_directorios);

void
liberar_directorio(directorio_t *dir)
{
	if (!dir) {
		return;
	}

	liberar_archivos(dir->archivos, dir->cant_archivos);
	liberar_directorios(dir->subdirectorios, dir->cant_directorios);

	free(dir->subdirectorios);
	free(dir);
}

void
liberar_directorios(directorio_t *subdirectorios[], size_t cant_directorios)
{
	for (size_t i = 0; i < cant_directorios; i++) {
		if (subdirectorios[i]) {
			liberar_directorio(subdirectorios[i]);
		}
	}
}

void
fs_destroy(filesystem_t *fs, const char *filename)
{
	guardar_fs(fs, filename);

	if (fs) {
		liberar_directorio(fs->raiz);
		free(fs);
	}

	printf("Sistema de archivos destruido y guardado.\n");
}