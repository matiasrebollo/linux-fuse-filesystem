# Filesystem en Memoria con FUSE – Sistemas Operativos

Implementación de un sistema de archivos en espacio de usuario (FUSE) enteramente en memoria. Permite crear, leer, escribir y borrar archivos y directorios, todo accesible a través de syscalls estándar como `ls`, `cat`, `touch`, etc.
Trabajo práctico de la materia **Sistemas Operativos** (FIUBA) – Grupo 25 – Cátedra Méndez-Fresia.

Integrantes:

- Agustín García Dresch
- Matías Gabriel Rebollo
- Marcos García Neira
- Sabrina García Lucentini

## Compilar

```bash
$ make
```

## Ejecutar

### Setup

Primero hay que crear un directorio de prueba:

```bash
$ mkdir prueba
```

### Iniciar el servidor FUSE

En el mismo directorio que se utilizó para compilar la solución, ejectuar:

```bash
$ ./fisopfs prueba/
```

Hay una flag `--filedisk NAME` para indicar que archivo se
 quiere utilizar como archivo de persistencia en disco. 
 El valor por defecto es "persistence_file.fisopfs"

```bash
$ ./fisopfs prueba/ --filedisk nuevo_disco.fisopfs
```

### Verificar directorio

```bash
$ mount | grep fisopfs
```

### Utilizar el directorio de "pruebas"

En otra terminal, ejecutar:

```bash
$ cd prueba
$ ls -al
```

### Limpieza

```bash
$ sudo umount prueba
```

## Docker

Existen tres _targets_ en el archivo `Makefile` para utilizar _docker_.

- `docker-build`: genera la imagen basada en "Ubuntu 20.04" con las dependencias de FUSE
- `docker-run`: crea un _container_ basado en la imagen anterior ejecutando `bash`
   - acá se puede ejecutar `make` y luego `./fisopfs -f ./prueba`
- `docker-attach`: permite vincularse al mismo _container_ anterior para poder realizar pruebas
   - acá se puede ingresar al directorio `prueba`

## Linter

```bash
$ make format
```

Para efectivamente subir los cambios producidos por el `format`, hay que `git add .` y `git commit`.

# Funcionalidades implementadas

## Archivos
- Crear archivos (`touch`, `echo > archivo`)
- Leer archivos (`cat`, `less`, `more`)
- Escribir archivos (modo truncado `>`, modo append `>>`)
- Eliminar archivos (`rm`)

## Directorios
- Crear directorios (`mkdir`)
- Leer contenido (`ls`, incluyendo `.` y `..`)
- Borrar directorios vacíos (`rmdir`)

## Estadísticas
- Soporte para `stat` (último acceso, modificación)
- Todos los archivos son propiedad del usuario y grupo actuales (`getuid`, `getgid`)
- Soporte para contenido binario

## Persistencia
- Todo el filesystem vive en memoria
- Se guarda en un único archivo `.fisopfs`
- Al desmontar o hacer flush, se escribe en disco automáticamente

---

## Tecnologías y herramientas
- Lenguaje: C11
- Sistema de archivos: FUSE
- Estándar: POSIX.1-2008
- Compilación: `make`

# Pruebas

Cada funcionalidad incluye pruebas automáticas que validan el comportamiento del sistema mediante scripts.
Las salidas esperadas se comparan contra las reales.

