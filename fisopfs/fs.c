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
#include <errno.h>

// PUEDE FALTAR MODULARIZAR LOS DEBUGS.


char *
strdup(const char *src)
{
	if (!src) {
		return NULL;
	}
	size_t len = strlen(src) + 1;  // Incluye el terminador nulo
	char *dest = malloc(len);
	if (!dest) {
		return NULL;
	}
	memcpy(dest, src, len);
	return dest;
}

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

	free(dir->stats);
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
	if (fread(&archivo->idx, sizeof(int), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer el índice del archivo.\n");
		free(archivo);
		return NULL;
	}

	// Deserializa los stats
	archivo->stats = malloc(
	        sizeof(stats_t));  // Asegurarse de que 'stats' sea asignado
	if (!archivo->stats) {
		fprintf(stderr, "[ERROR] Error al asignar memoria para los stats del archivo.\n");
		free(archivo);
		return NULL;
	}
	if (fread(&archivo->stats->st_mode, sizeof(mode_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_nlink, sizeof(nlink_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_uid, sizeof(uid_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_gid, sizeof(gid_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_size, sizeof(off_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_atime, sizeof(time_t), 1, file) != 1 ||
	    fread(&archivo->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al leer los stats del archivo.\n");
		free(archivo->stats);
		free(archivo);
		return NULL;
	}

	// Si el archivo tiene datos, leer su contenido
	if (archivo->stats->st_size > 0) {
		archivo->data = malloc(archivo->stats->st_size);
		if (!archivo->data) {
			fprintf(stderr, "[ERROR] Error al asignar memoria para la data del archivo.\n");
			free(archivo->stats);
			free(archivo);
			return NULL;
		}
		if (fread(archivo->data, sizeof(char), archivo->stats->st_size, file) !=
		    archivo->stats->st_size) {
			fprintf(stderr,
			        "[ERROR] Error al leer la data del archivo.\n");
			free(archivo->data);
			free(archivo->stats);
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
	if (fread(&dir->idx, sizeof(int), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al leer el índice del directorio.\n");
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

	// Deserializa los stats del directorio
	dir->stats = malloc(sizeof(stats_t));
	if (!dir->stats) {
		fprintf(stderr, "[ERROR] Error al asignar memoria para los stats del directorio.\n");
		free(dir);
		return NULL;
	}
	if (fread(&dir->stats->st_mode, sizeof(mode_t), 1, file) != 1 ||
	    fread(&dir->stats->st_nlink, sizeof(nlink_t), 1, file) != 1 ||
	    fread(&dir->stats->st_uid, sizeof(uid_t), 1, file) != 1 ||
	    fread(&dir->stats->st_gid, sizeof(gid_t), 1, file) != 1 ||
	    fread(&dir->stats->st_size, sizeof(off_t), 1, file) != 1 ||
	    fread(&dir->stats->st_atime, sizeof(time_t), 1, file) != 1 ||
	    fread(&dir->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al leer los stats del directorio.\n");
		free(dir->stats);
		free(dir);
		return NULL;
	}

	memset(dir->archivos, 0, sizeof(dir->archivos));
	memset(dir->subdirectorios, 0, sizeof(dir->subdirectorios));

	for (size_t i = 0; i < dir->cant_archivos; i++) {
		dir->archivos[i] = deserializar_archivo(file);
		if (!dir->archivos[i]) {
			liberar_archivos(dir->archivos, i);
			free(dir->stats);
			free(dir);
			return NULL;
		}
	}
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		dir->subdirectorios[i] = deserializar_directorio(file);
		if (!dir->subdirectorios[i]) {
			liberar_archivos(dir->archivos, dir->cant_archivos);
			liberar_subdirectorios(dir->subdirectorios, i);
			free(dir->stats);
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

	fs->raiz = NULL;

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

	if (fs->max_size < fs->current_size) {
		fprintf(stderr, "[ERROR] Archivo inválido: 'current_size' es mayor que 'max_size'.\n");
		free(fs);
		fclose(file);
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

char *
dirname(char *path)
{
	if (!path || strlen(path) == 0) {
		return ".";
	}
	// Hacer una copia del path para trabajar sobre ella
	char *copy = strdup(path);
	if (!copy) {
		return NULL;
	}
	// Encontrar el último separador '/'
	char *last_slash = strrchr(copy, '/');
	if (!last_slash) {
		free(copy);
		return ".";  // No hay '/' en el path, el directorio es "."
	}
	// Si el '/' está al inicio, es la raíz "/"
	if (last_slash == copy) {
		*(last_slash + 1) = '\0';  // Mantener el '/' final
		return copy;
	}
	// Eliminar el '/' final para obtener el directorio
	*last_slash = '\0';
	return copy;
}

char *
basename(char *path)
{
	if (!path || strlen(path) == 0) {
		return ".";
	}

	// Encontrar el último separador '/'
	char *last_slash = strrchr(path, '/');
	if (!last_slash) {
		return path;  // No hay '/' en el path, todo es el nombre del archivo
	}
	return last_slash + 1;  // Retornar lo que viene después del último '/'
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
		fprintf(stderr, "[ERROR] Error al asignar memoria para estadísticas.\n");
		free(file);
		return NULL;
	}
	file->stats->st_mode = mode;         // Tipo y permisos del archivo
	file->stats->st_nlink = 1;           // Número de enlaces
	file->stats->st_uid = getuid();      // UID del usuario actual
	file->stats->st_gid = getgid();      // GID del usuario actual
	file->stats->st_size = 0;            // Tamaño inicial
	file->stats->st_atime = time(NULL);  // Último acceso
	file->stats->st_mtime = time(NULL);  // Última modificación
	return file;
}

int
fs_create(filesystem_t *fs, const char *path, mode_t mode)
{
	if (!fs || !path || strlen(path) == 0 || strlen(path) >= MAX_PATH) {
		fprintf(stderr,
		        "[ERROR] Path inválido o sistema no inicializado.\n");
		return -1;
	}

	char dir_path[MAX_PATH];
	char file_name[MAX_FILE_NAME];
	const char *slash_pos = strrchr(path, '/');

	if (slash_pos) {
		if (slash_pos == path) {
			// Caso: archivo en la raíz (path = "/archivo.txt")
			strcpy(dir_path, "/");
			if (*(slash_pos + 1) == '\0') {
				fprintf(stderr, "[ERROR] Path inválido: falta el nombre del archivo.\n");
				return -1;
			}
			strcpy(file_name, slash_pos);
		} else {
			// Caso: subdirectorio especificado (path = "/subdir/archivo.txt")
			size_t dir_len = slash_pos - path;
			if (dir_len >= MAX_PATH ||
			    strlen(slash_pos + 1) >= MAX_FILE_NAME) {
				fprintf(stderr, "[ERROR] Path o nombre de archivo demasiado largo.\n");
				return -1;
			}
			strncpy(dir_path, path, dir_len);
			dir_path[dir_len] = '\0';  // Asegurar terminación
			strcpy(file_name, slash_pos + 1);
		}
	} else {
		// Caso: archivo directamente en la raíz (path = "archivo.txt")
		strcpy(dir_path, "/");
		strcpy(file_name, path);
	}

	// Obtener el subdirectorio o la raíz donde crear el archivo
	directorio_t *target_dir = fs_getdir(fs, dir_path);
	printf("Nombre de subdirectorio conseguido: %s", dir_path);
	if (!target_dir) {
		fprintf(stderr,
		        "[ERROR] Subdirectorio no encontrado: %s\n",
		        dir_path);
		return -1;
	}

	// Verificar si ya existe un archivo con el mismo nombre
	for (size_t i = 0; i < target_dir->cant_archivos; i++) {
		if (strcmp(target_dir->archivos[i]->nombre, file_name) == 0) {
			fprintf(stderr,
			        "[ERROR] Ya existe un archivo con el mismo "
			        "nombre en el directorio '%s'.\n",
			        dir_path);
			return -1;
		}
	}

	// Crear el archivo
	archivo_t *nuevo_archivo =
	        crear_archivo(file_name, target_dir->cant_archivos, mode);
	if (!nuevo_archivo) {
		fprintf(stderr, "[ERROR] Error al crear el archivo.\n");
		return -1;
	}

	// Agregar el archivo al directorio
	target_dir->archivos[target_dir->cant_archivos++] = nuevo_archivo;
	target_dir->stats->st_atime = time(NULL);
	target_dir->stats->st_mtime = time(NULL);
	fs->current_size++;

	printf("[DEBUG] Archivo '%s' creado en '%s'.\n", file_name, dir_path);
	return 0;
}

directorio_t *
crear_directorio(const char *path, int idx)
{
	directorio_t *dir = malloc(sizeof(directorio_t));
	if (!dir) {
		fprintf(stderr,
		        "[ERROR] Error al asignar memoria para directorio.\n");
		return NULL;
	}
	snprintf(dir->nombre, MAX_FILE_NAME, "%s", path);
	dir->idx = idx;
	dir->cant_archivos = 0;
	dir->cant_directorios = 0;
	dir->stats = malloc(sizeof(stats_t));
	if (!dir->stats) {
		fprintf(stderr, "[ERROR] Error al asignar memoria para las stats del directorio.\n");
		free(dir);
		return NULL;
	}
	dir->stats->st_mtime = time(NULL);  // tiempo de modif
	dir->stats->st_atime = time(NULL);  // tiemo de acceso
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
		fclose(file);
		fs = cargar_fs(filename);
		if (!fs) {
			fprintf(stderr, "[ERROR] Error al cargar el sistema de archivos desde el archivo.\n");
			return NULL;
		}
		printf("[DEBUG] Sistema de archivos cargado desde '%s'.\n",
		       filename);
	} else {
		printf("[DEBUG] Archivo no encontrado o carga fallida. Creando "
		       "nuevo sistema de archivos.\n");
		fs = malloc(sizeof(filesystem_t));
		if (!fs) {
			fprintf(stderr, "[ERROR] Error al asignar memoria para el filesystem.\n");
			return NULL;
		}
		fs->max_size = 1024 * 1024 * 1024;
		fs->current_size = 0;
		fs->raiz = crear_directorio("/", 0);
		if (!fs->raiz) {
			fprintf(stderr, "[ERROR] Error al crear la raiz del filesystem.\n");
			free(fs);
			return NULL;
		}
		printf("[DEBUG] Nuevo sistema de archivos creado.\n");
	}

	return fs;
}

directorio_t *
obtener_directorio(directorio_t *dir, const char *path)
{
	if (!dir) {
		fprintf(stderr, "[ERROR] Error al buscar el directorio\n");
		return NULL;
	}
	if (strcmp(dir->nombre, path) == 0) {
		return dir;
	}
	// Buscar en los directorios de la raíz
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		if (strcmp(dir->subdirectorios[i]->nombre, path) == 0) {
			return dir->subdirectorios[i];
		}
	}

	fprintf(stderr, "[ERROR] Directorio %s no encontrado\n", path);
	return NULL;
}

int
fs_mkdir(filesystem_t *fs, const char *path)
{
	// Valida que el path sea directo desde la raíz y con único nivel de recursión
	if (path[0] == '/' && strchr(path + 1, '/')) {
		fprintf(stderr,
		        "[ERROR] mkdir solo permitido en el directorio raíz\n");
		return -1;
	}

	if (fs->raiz->cant_directorios >= MAX_FILES) {
		fprintf(stderr, "[ERROR] Se alcanzó el máximo de subdirectorios permitidos\n");
		return -1;
	}

	directorio_t *dir = obtener_directorio(fs->raiz, path);
	if (dir) {
		fprintf(stderr,
		        "[ERROR] Ya existe un subdirectorio con este nombre\n");
		return -1;
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

directorio_t *
fs_getdir(filesystem_t *fs, const char *path)
{
	return obtener_directorio(fs->raiz, path);
}

int
fs_rmdir(filesystem_t *fs, const char *path)
{
	if (strcmp(path, "/") == 0) {
		fprintf(stderr, "[ERROR] No está permitido eliminar el directorio raíz\n");
		return -1;
	}

	directorio_t *dir = obtener_directorio(fs->raiz, path);
	if (!dir) {
		fprintf(stderr, "[ERROR] Directorio no encontrado: %s\n", path);
		return -1;
	}

	if (dir->cant_archivos > 0 || dir->cant_directorios > 0) {
		fprintf(stderr, "[ERROR] El directorio no está vacío: %s\n", path);
		return -1;
	}

	// Actualiza el índice de los directorios
	int idx = dir->idx;
	fs->raiz->subdirectorios[idx] =
	        fs->raiz->subdirectorios[fs->raiz->cant_directorios - 1];
	if (fs->raiz->subdirectorios[idx]) {
		fs->raiz->subdirectorios[idx]->idx = idx;
	}
	fs->raiz->subdirectorios[fs->raiz->cant_directorios - 1] = NULL;
	fs->raiz->cant_directorios--;

	liberar_directorio(dir);
	fs->raiz->stats->st_atime = time(NULL);
	fs->raiz->stats->st_mtime = time(NULL);

	return 0;
}

int
fs_unlink(filesystem_t *fs, const char *path)
{
	if (!fs || !path || strlen(path) == 0 || strlen(path) >= MAX_PATH) {
		fprintf(stderr,
		        "[ERROR] Path inválido o sistema no inicializado.\n");
		return -EINVAL;  // Argumento inválido
	}

	char dir_path[MAX_PATH];
	char file_name[MAX_FILE_NAME];
	const char *slash_pos = strrchr(path, '/');

	if (slash_pos) {
		if (slash_pos == path) {
			// Caso: archivo en la raíz (path = "/archivo.txt")
			strcpy(dir_path, "/");
			if (*(slash_pos + 1) == '\0') {
				fprintf(stderr, "[ERROR] Path inválido: falta el nombre del archivo.\n");
				return -1;
			}
			strcpy(file_name, slash_pos);
		} else {
			// Caso: subdirectorio especificado (path = "/subdir/archivo.txt")
			size_t dir_len = slash_pos - path;
			if (dir_len >= MAX_PATH ||
			    strlen(slash_pos + 1) >= MAX_FILE_NAME) {
				fprintf(stderr, "[ERROR] Path o nombre de archivo demasiado largo.\n");
				return -1;
			}
			strncpy(dir_path, path, dir_len);
			dir_path[dir_len] = '\0';  // Asegurar terminación
			strcpy(file_name, slash_pos + 1);
		}
	} else {
		// Caso: archivo directamente en la raíz (path = "archivo.txt")
		strcpy(dir_path, "/");
		strcpy(file_name, path);
	}

	// Obtener el subdirectorio o la raíz donde eliminar el archivo
	directorio_t *target_dir = fs_getdir(fs, dir_path);
	if (!target_dir) {
		fprintf(stderr,
		        "[ERROR] No se encontró el directorio '%s'.\n",
		        dir_path);
		return -ENOENT;  // Directorio no encontrado
	}

	// Buscar el archivo en el directorio
	int file_idx = -1;
	for (size_t i = 0; i < target_dir->cant_archivos; i++) {
		if (strcmp(target_dir->archivos[i]->nombre, file_name) == 0) {
			file_idx = i;
			break;
		}
	}

	if (file_idx == -1) {
		fprintf(stderr,
		        "[ERROR] No se encontró el archivo '%s'.\n",
		        file_name);
		return -ENOENT;  // Archivo no encontrado
	}

	// Liberar memoria del archivo
	archivo_t *archivo = target_dir->archivos[file_idx];
	free(archivo->data);
	free(archivo->stats);
	free(archivo);

	// Reorganizar el arreglo de archivos
	for (size_t i = file_idx; i < target_dir->cant_archivos - 1; i++) {
		target_dir->archivos[i] = target_dir->archivos[i + 1];
	}
	target_dir->archivos[target_dir->cant_archivos - 1] = NULL;
	target_dir->cant_archivos--;

	// Actualizar los tiempos del directorio
	target_dir->stats->st_atime = time(NULL);
	target_dir->stats->st_mtime = time(NULL);

	// Reducir el tamaño actual del filesystem
	fs->current_size--;

	printf("[debug] Archivo '%s' eliminado correctamente.\n", file_name);
	return 0;  // Éxito
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
	if (fwrite(&archivo->idx, sizeof(int), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el index del archivo.\n");
		return -1;
	}

	// Serializa los campos de stats_t
	/*if (fwrite(archivo->stats, sizeof(stats_t), 1, file) != 1) {
	        fprintf(stderr,
	                "[ERROR] Error al escribir los stats del archivo.\n");
	        return -1;
	}*/
	if (fwrite(&archivo->stats->st_mode, sizeof(mode_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_nlink, sizeof(nlink_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_uid, sizeof(uid_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_gid, sizeof(gid_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_size, sizeof(off_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_atime, sizeof(time_t), 1, file) != 1 ||
	    fwrite(&archivo->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir los stats del archivo\n");
		return -1;
	}

	// Serializa la data del archivo si existe
	if (archivo->data != NULL && archivo->stats->st_size > 0) {
		if (fwrite(archivo->data,
		           sizeof(char),
		           archivo->stats->st_size,
		           file) != archivo->stats->st_size) {
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
		fprintf(stderr,
		        "[ERROR] Error al escribir el nombre del directorio\n");
		return -1;
	}
	if (fwrite(&dir->idx, sizeof(int), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir el index del directorio.\n");
		return -1;
	}
	if (fwrite(&dir->cant_archivos, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la cantidad de archivos del directorio\n");
		return -1;
	}
	if (fwrite(&dir->cant_directorios, sizeof(size_t), 1, file) != 1) {
		fprintf(stderr, "[ERROR] Error al escribir la cantidad de subdirectorios del directorio\n");
		return -1;
	}

	// Serializa los stats del directorio
	/*if (fwrite(dir->stats, sizeof(stats_t), 1, file) != 1) {
	        fprintf(stderr,
	                "[ERROR] Error al escribir los stats del directorio\n");
	        return -1;
	}*/
	if (fwrite(&dir->stats->st_mode, sizeof(mode_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_nlink, sizeof(nlink_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_uid, sizeof(uid_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_gid, sizeof(gid_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_size, sizeof(off_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_atime, sizeof(time_t), 1, file) != 1 ||
	    fwrite(&dir->stats->st_mtime, sizeof(time_t), 1, file) != 1) {
		fprintf(stderr,
		        "[ERROR] Error al escribir los stats del directorio\n");
		return -1;
	}

	// Serialización de archivos
	for (size_t i = 0; i < dir->cant_archivos; i++) {
		if (serializar_archivo(file, dir->archivos[i]) != 0) {
			fprintf(stderr, "[ERROR] Error al escribir un archivo del directorio\n");
			return -1;
		}
	}
	// Serialización de subdirectorios
	for (size_t i = 0; i < dir->cant_directorios; i++) {
		if (serializar_directorio(file, dir->subdirectorios[i]) != 0) {
			fprintf(stderr, "[ERROR] Error al escribir un subdirectorio del directorio\n");
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

archivo_t *
iter_dir(directorio_t *dir, const char *path)
{
	for (int i = 0; i < dir->cant_archivos; i++) {
		if (strcmp(dir->archivos[i]->nombre, path) == 0) {
			return dir->archivos[i];
		}
	}
	return NULL;
}

archivo_t *
search_file(directorio_t *dir, const char *path)
{
	char dir_path[MAX_PATH];
	char file_name[MAX_FILE_NAME];
	const char *slash_pos = strrchr(path, '/');

	if (slash_pos) {
		if (slash_pos == path) {
			// Caso: archivo en la raíz (path = "/archivo.txt")
			strcpy(dir_path, "/");
			if (*(slash_pos + 1) == '\0') {
				fprintf(stderr, "[ERROR] Path inválido: falta el nombre del archivo.\n");
				return NULL;
			}
			strcpy(file_name, slash_pos);
		} else {
			// Caso: subdirectorio especificado (path = "/subdir/archivo.txt")
			size_t dir_len = slash_pos - path;
			if (dir_len >= MAX_PATH ||
			    strlen(slash_pos + 1) >= MAX_FILE_NAME) {
				fprintf(stderr, "[ERROR] Path o nombre de archivo demasiado largo.\n");
				return NULL;
			}
			strncpy(dir_path, path, dir_len);
			dir_path[dir_len] = '\0';  // Asegurar terminación
			strcpy(file_name, slash_pos + 1);
		}
	} else {
		// Caso: archivo directamente en la raíz (path = "archivo.txt")
		strcpy(dir_path, "/");
		strcpy(file_name, path);
	}

	archivo_t *f = iter_dir(dir, file_name);
	if (f)
		return f;
	for (int i = 0; i < dir->cant_directorios; i++) {
		f = search_file(dir->subdirectorios[i], path);
		if (f)
			return f;
	}
	return NULL;
}

archivo_t *
fs_open(filesystem_t *fs, const char *path)
{
	if (path == NULL || fs == NULL) {
		printf("[DEBUG] Nombre de archivo nulo.\n");
		return NULL;
	}

	return search_file(fs->raiz, path);
}

stats_t *
fs_getattr(filesystem_t *fs, const char *path)
{
	if (!fs || !path) {
		fprintf(stderr,
		        "[ERROR] Filesystem o path inválidos en fs_getattr.\n");
		return NULL;
	}
	archivo_t *archivo = search_file(fs->raiz, path);
	if (archivo) {
		return archivo->stats;
	}
	directorio_t *directorio = obtener_directorio(fs->raiz, path);
	if (directorio) {
		return directorio->stats;
	}
	return NULL;
}