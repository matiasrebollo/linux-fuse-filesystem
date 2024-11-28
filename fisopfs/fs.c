#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

//PUEDE FALTAR MODULARIZAR LOS DEBUGS.

void
liberar_archivos(archivo_t **archivos, size_t cant_archivos)
{
	if (!archivos){
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
	if (!subdirectorios){
		return;
	}
	for (size_t i = 0; i < cant_directorios; i++) {
		if (subdirectorios[i]) {
			liberar_directorio(subdirectorios[i]);
			//subdirectorios[i] = NULL;
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
	if (!archivo){
		fprintf(stderr, "[ERROR] Error al asignar memoria para archivo.\n");
		return NULL;
	}

	if (fread(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "[ERROR] Error al leer el nombre del archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->tamanio, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer el tamaño archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de creacion del archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "Error al leer la fecha de modificacion del archivo.\n");
		free(archivo);
		return NULL;
	}

	if (archivo->tamanio > 0) {
		archivo->data = malloc(archivo->tamanio);
		if (!archivo->data){
			fprintf(stderr, "[ERROR] Error al asignar memoria para la data del archivo.\n");
			free(archivo);
			return NULL;
		}
		if (fread(archivo->data, sizeof(char), archivo->tamanio, file) !=
		    archivo->tamanio) {
			fprintf(stderr, "[ERROR] Error al leer la data del archivo.\n");
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
		fprintf(stderr, "[ERROR] Error al asignar memoria para directorio.\n");
		return NULL;
	}

	if (fread(dir->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "[ERROR] Error al leer el nombre del directorio.\n");
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
	if (fread(&dir->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de creacion del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer la fecha de modificacion del directorio.\n");
		free(dir);
		return NULL;
	}

	memset(dir->archivos, 0, sizeof(dir->archivos));
	memset(dir->subdirectorios, 0, sizeof(dir->subdirectorios));

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		dir->archivos[i] = deserializar_archivo(file);
		if (!dir->archivos[i]){
			liberar_archivos(dir->archivos, i);
			free(dir);
			return NULL;
		}
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		dir->subdirectorios[i] = deserializar_directorio(file);
		if (!dir->subdirectorios[i]){
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
		fprintf(stderr, "[ERROR] Error al abrir el archivo para cargar.\n");
		return NULL;
	}

	filesystem_t *fs = malloc(sizeof(filesystem_t));
	if (!fs){
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
	if (!fs->raiz){
		free(fs);
		fprintf(stderr, "[ERROR] Error al crear la raiz del filesystem.\n");
		return NULL;
	}

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

/*void
agregar_archivo(directorio_t *dir, archivo_t *archivo)
{
	if (dir->cant_archivos < MAX_FILES) {
		dir->archivos[dir->cant_archivos] = archivo;
		dir->cant_archivos++;
	}
}*/

filesystem_t *
fs_init(const char *filename)
{
	filesystem_t *fs = NULL;

	// Intentar cargar el fs desde el archivo
	FILE *file = fopen(filename, "rb");
	if (file) {
		fs = cargar_fs(filename);
		fclose(file);
		if (!fs){
			fprintf(stderr, "[ERROR] Error al cargar el filesystem desde el archivo.\n");
			return NULL;
		}
		printf("[DEBUG] Sistema de archivos cargado desde el archivo.\n");
	} else {
		fs = malloc(sizeof(filesystem_t));
		if (!fs){
			fprintf(stderr, "[ERROR] Error al asignar memoria para el filesystem.\n");
			return NULL;
		}
		fs->max_size = 1024 * 1024 * 1024;
		fs->current_size = 0;
		fs->raiz = crear_directorio("/");
		if (!fs->raiz){
			free(fs);
			fprintf(stderr, "[ERROR] Error al crear la raiz del filesystem.\n");
			return NULL;
		}
		printf("[DEBUG] Nuevo sistema de archivos creado.\n");
	}

	return fs;
}

int
serializar_archivo(FILE *file, archivo_t *archivo)
{
	if (fwrite(archivo->nombre, sizeof(char), MAX_FILE_NAME, file) !=
	    MAX_FILE_NAME) {
		fprintf(stderr, "[ERROR] Error al escribir el nombre del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->tamanio, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir el tamaño del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de creacion del archivo.\n");
		return -1;
	}
	if (fwrite(&archivo->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de modificacion archivo.\n");
		return -1;
	}

	if (archivo->data != NULL && archivo->tamanio > 0) {
		if (fwrite(archivo->data, sizeof(char), archivo->tamanio, file) !=
		    archivo->tamanio) {
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
	if (fwrite(&dir->fecha_creacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de creacion del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->fecha_modificacion, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la fecha de modificacion del directorio.\n");
		return -1;
	}

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		if (serializar_archivo(file, dir->archivos[i]) != 0){
			fprintf(stderr, "[ERROR] Error al escribir un archivo del directorio.\n");
			return -1;
		}
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		if (serializar_directorio(file, dir->subdirectorios[i]) != 0){
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
		fprintf(stderr, "[ERROR] Error al abrir el archivo para guardar.\n");
		return -1;
	}

	if (fwrite(&fs->max_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir el tamaño maximo del fs.\n");
		fclose(file);
		return -1;
	}
	if (fwrite(&fs->current_size, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir el tamaño actual del fs.\n");
		fclose(file);
		return -1;
	}

	if (serializar_directorio(file, fs->raiz) != 0){
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
	if(!fs){
		return;
	}
	if (guardar_fs(fs, filename) != 0){
		fprintf(stderr, "[ERROR] Error al serializar el file system.\n");
	}

	liberar_directorio(fs->raiz);
	free(fs);

	printf("[DEBUG] Sistema de archivos destruido y guardado.\n");
}