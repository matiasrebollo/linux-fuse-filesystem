#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fs.h"

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

char *filedisk = DEFAULT_FILE_DISK;
filesystem_t* fs;

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[DEBUG] Inicializando filesystem.\n");
	filesystem_t *fs_ = fs_init();
	if (!fs_) {
		fprintf(stderr, "[ERROR] No se pudo inicializar el filesystem.\n");
		return NULL;
	}
	printf("[DEBUG] Filesystem inicializado correctamente.\n");
	fs = fs_;
	return fs_;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[DEBUG] Destruyendo filesystem.\n");
	if (private_data) {
		fs_destroy((filesystem_t *) private_data, filedisk);
	}
	printf("[DEBUG] Filesystem destruido correctamente.\n");
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	stats_t *s = fs_getattr(fs, path);

	if(!s)
		return -ENOENT;
	
	st->st_atime = s->st_atime;
	st->st_gid = s->st_gid;
	st->st_mode = s->st_mode;
	st->st_size = s->st_size;
	st->st_uid = s->st_uid;
	st->st_nlink = s->st_nlink;
	st->st_mtime = s->st_mtime;

	return 0;

}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	directorio_t* dir = (directorio_t*)fi->fh;
	if(!dir){
		return -ENOENT;
	}
	for(int i = 0; i< dir->cant_archivos; i++){
		filler(buffer, dir->archivos[i]->nombre, NULL,0);
	}

	return 0;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fisop_file_contenidos + offset, size);

	return size;
}

static int
fisopfs_open(const char * path, struct fuse_file_info * fi){
	archivo_t* f = fs_open(fs, path);
	if(f){
		fi->fh = (uint64_t) f;
		return 0;
	}
	return -ENOENT;
}

static int fisopfs_opendir(const char* path, struct fuse_file_info * fi){
	directorio_t* d = fs_getdir(fs, path);
	if(d){
		fi->fh = (uint64_t) d;
		return 0;
	}
	return -ENOENT;
}
static int fisopfs_mkdir(const char* path, mode_t mode){
	return fs_mkdir(fs, path);
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.open = fisopfs_open,
	.mkdir = fisopfs_mkdir,
	.opendir = fisopfs_opendir	
};

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}