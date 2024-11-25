CC = gcc
CFLAGS := -ggdb3 -O2 -Wall -std=c11 -D_FILE_OFFSET_BITS=64
CFLAGS += -Wno-unused-function -Wvla

# Flags for FUSE
LDLIBS := $(shell pkg-config fuse --cflags --libs)

# Name for the filesystem!
FS_NAME := fisopfs

# Source files
SOURCES := fisopfs.c fs.c
# Object files (uno por cada fuente)
OBJECTS := $(SOURCES:.c=.o)

# Target principal
all: $(FS_NAME)

# Regla para compilar el binario principal
$(FS_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Regla genérica para compilar archivos .o desde .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Formatear el código (opcional)
format: .clang-files .clang-format
	xargs -r clang-format -i <$<

# Regla para construir el sistema dentro de un contenedor Docker
docker-build:
	docker build -t fisopfs:latest .

docker-run:
	docker container run -it --rm --name=fisopfs \
		-v $(shell pwd):/fisopfs \
		--device /dev/fuse \
		--cap-add SYS_ADMIN \
		--security-opt apparmor:unconfined \
		fisopfs:latest \
		bash

docker-attach:
	docker exec -it fisopfs bash

# Limpiar archivos compilados
clean:
	rm -rf $(OBJECTS) core vgcore.* $(FS_NAME)

.PHONY: all build clean format docker-build docker-run docker-attach
