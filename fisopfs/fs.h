#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_FILE_NAME 256
#define MAX_FILES 128
#define MAX_PATH 1024

typedef struct stats {
	mode_t st_mode;    // mode of file
	nlink_t st_nlink;  // number of links to the file
	uid_t st_uid;      // user ID of file
	gid_t st_gid;      // group ID of file
	off_t st_size;     // file size in bytes (if file is a regular file)
	time_t st_atime;   // time of last access
	time_t st_mtime;   // time of last data modification

} stats_t;

typedef struct archivo {
	char nombre[MAX_FILE_NAME];
	int idx;
	void *data;
	stats_t *stats;
} archivo_t;

typedef struct directorio {
	char nombre[MAX_FILE_NAME];
	int idx;
	struct directorio *subdirectorios[MAX_FILES];
	archivo_t *archivos[MAX_FILES];
	size_t cant_archivos;
	size_t cant_directorios;
	stats_t *stats;
} directorio_t;

typedef struct filesystem {
	directorio_t *raiz;
	size_t max_size;
	size_t current_size;
} filesystem_t;

filesystem_t *fs_init(const char *filename);
void fs_destroy(filesystem_t *fs, const char *filename);
int fs_mkdir(filesystem_t *fs, const char *path);
int fs_rmdir(filesystem_t *fs, const char *path);
archivo_t *fs_open(filesystem_t *fs, const char *path);
stats_t *fs_getattr(filesystem_t *fs, const char *path);
directorio_t *fs_getdir(filesystem_t *fs, const char *path);
int fs_create(filesystem_t *fs, const char *path, mode_t mode);
int fs_unlink(filesystem_t *fs, const char *path);

#endif