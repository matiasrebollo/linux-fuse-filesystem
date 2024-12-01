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
filesystem_t *fs;

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[DEBUG] Inicializando filesystem.\n");
	filesystem_t *fs_ = fs_init(filedisk);
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

	if (!s)
		return -ENOENT;
	s->st_atime = time(NULL);
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

	directorio_t *dir = (directorio_t *) fi->fh;

	if (strcmp(path, "/") == 0) {
		// Si estamos en la raíz, usar +1
		for (int i = 0; i < dir->cant_archivos; i++) {
			filler(buffer, dir->archivos[i]->nombre + 1, NULL, 0);
		}
		for (int i = 0; i < dir->cant_directorios; i++) {
			filler(buffer, dir->subdirectorios[i]->nombre + 1, NULL, 0);
		}
	} else {
		// Si estamos en un subdirectorio, usar el nombre completo
		for (int i = 0; i < dir->cant_archivos; i++) {
			filler(buffer, dir->archivos[i]->nombre, NULL, 0);
		}
		for (int i = 0; i < dir->cant_directorios; i++) {
			filler(buffer, dir->subdirectorios[i]->nombre, NULL, 0);
		}
	}

	return 0;
}

#define MAX_CONTENIDO 100;

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	if (!fs) {
		return -ENOENT;
	}

	archivo_t *file = fs_open(fs, path);
	if (!file) {
		return -ENOENT;
	}

	printf("[debug] fisopfs_read - path: %s, offset: %li, size: %lu\n",
	       path,
	       offset,
	       size);

	if (offset >= file->stats->st_size) {
		return 0;
	}

	size_t bytes_to_read = size;
	if (offset + size > file->stats->st_size) {
		bytes_to_read = file->stats->st_size - offset;
	}

	memcpy(buffer, (char *) file->data + offset, bytes_to_read);
	file->stats->st_atime = time(NULL);

	return bytes_to_read;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	if (!fs) {
		return -ENOENT;
	}

	archivo_t *file = fs_open(fs, path);
	if (!file) {
		return -ENOENT;
	}

	if (offset > file->stats->st_size) {
		return -EINVAL;
	}

	if (offset + size > file->stats->st_size) {
		size_t new_size = offset + size;
		file->data = realloc(file->data, new_size);
		if (!file->data) {
			return -ENOMEM;
		}
		file->stats->st_size = new_size;
	}

	memcpy((char *) file->data + offset, buffer, size);

	file->stats->st_mtime = time(NULL);
	file->stats->st_atime = time(NULL);

	return size;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	if (fs == NULL) {
		return -EFAULT;
	}

	int ret = fs_create(fs, path, mode);
	if (ret < 0) {
		return ret;
	}

	fi->fh = (uint64_t) ret;

	return 0;
}


static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_open %s\n", path);
	archivo_t *f = fs_open(fs, path);
	if (f) {
		f->stats->st_atime = time(NULL);
		fi->fh = (uint64_t) f;
		return 0;
	}
	return -ENOENT;
}

static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_opendir %s\n", path);
	directorio_t *d = fs_getdir(fs, path);
	if (d) {
		fi->fh = (uint64_t) d;
		return 0;
	}
	return -ENOENT;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir %s\n", path);
	return fs_mkdir(fs, path);
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir %s\n", path);
	return fs_rmdir(fs, path);
}

static int
fisopfs_unlink(const char *path)
{
	return fs_unlink(fs, path);
}


static int
fisopfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	if (fs == NULL) {
		return -EFAULT;
	}

	if (!S_ISREG(mode)) {
		return -EINVAL;
	}

	int ret = fs_create(fs, path, mode);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	archivo_t *archivo = fs_open(fs, path);
	if (!archivo) {
		fprintf(stderr, "[ERROR] No se encontró el archivo para actualizar tiempos: %s\n", path);
		return -ENOENT;
	}
	if (ts) {
		archivo->stats->st_atime = ts[0].tv_sec;
		archivo->stats->st_mtime = ts[1].tv_sec;
	} else {
		time_t now = time(NULL);
		archivo->stats->st_atime = now;
		archivo->stats->st_mtime = now;
	}
	return 0;
}

int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush %s\n", path);
	fisopfs_destroy(NULL);
	return 0;
}

static struct fuse_operations operations = { .getattr = fisopfs_getattr,
	                                     .readdir = fisopfs_readdir,
	                                     .read = fisopfs_read,
	                                     .write = fisopfs_write,
	                                     .init = fisopfs_init,
	                                     .destroy = fisopfs_destroy,
	                                     .open = fisopfs_open,
	                                     .mkdir = fisopfs_mkdir,
	                                     .rmdir = fisopfs_rmdir,
	                                     .opendir = fisopfs_opendir,
	                                     .create = fisopfs_create,
	                                     .mknod = fisopfs_mknod,
	                                     .unlink = fisopfs_unlink,
	                                     .utimens = fisopfs_utimens,
	                                     .flush = fisopfs_flush };

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