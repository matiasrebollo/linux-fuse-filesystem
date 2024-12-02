# **TP3: Filesystem FUSE**
## **Introducción**

El objetivo de este proyecto es desarrollar un sistema de archivos básico, con funciones esenciales para gestionar directorios y archivos.

El sistema de archivos usa el mecanismo de *FUSE* (Filesystem in USErspace) provisto por el kernel, que nos permitirá definir en modo usuario la implementación de un filesystem. Gracias a ello, el mismo tendrá la interfaz *VFS* y podrá ser accedido con las syscalls y programas habituales (read, open, ls, etc).

Está diseñado para simular el comportamiento de un sistema de archivos real, proporcionando una estructura que soporta la creación de archivos, la búsqueda de archivos y directorios mediante rutas, así como la lectura y escritura en dichos archivos entre otras cosas. Es importante destacar que las funciones implementadas en este proyecto representan un subconjunto de las operaciones soportadas por *FUSE*, enfocándonos en las más relevantes para la gestión básica de archivos y directorios.

Cabe aclarar que el sistema de archivos implementado tiene una estructura jerárquica limitada: solo existe un unico nivel de recursion en los directorios. El directorio raíz puede contener archivos y subdirectorios, mientras que los subdirectorios solo pueden contener archivos. No se permite una jerarquía de mayor profundidad.

La implementación del filesystem será enteramente en memoria: tanto archivos como directorios serán representados mediante estructuras que vivirán en memoria RAM. Aún así, los datos del filesystem estarán representados de manera persistente en disco por un archivo.

## **Estructuras en memoria**

El sistema de archivos se organiza en tres tipos principales de estructuras: archivos, directorios y el sistema de archivos en sí mismo. A esto se le suma una estructura stat que contiene metadata de cada archivo o directorio.

### **Estructura de archivos (archivo_t)**

Cada archivo se representa mediante la estructura archivo_t, que incluye los siguientes campos:

- **nombre**: un nombre de archivo.
- **idx**: un índice que representa la posicion del archivo en el array de archivos del directorio padre.
- **data**: un puntero a los datos del archivo, almacenados íntegramente en memoria.
- **stats**: un puntero a la estructura stats_t, que almacena metadatos del archivo.

### **Estructura de directorios (directorio_t)**

Los directorios son representados con la estructura directorio_t, diseñada para almacenar tanto archivos como subdirectorios. Sus campos son:

- **nombre**: el nombre del directorio.
- **idx**: el índice que representa la posición del directorio en el array de subdirectorios de su directorio padre.
- **subdirectorios**: un array de punteros que apunta a otros directorios contenidos dentro del directorio actual.
- **archivos**: un array de punteros a archivos que residen en el directorio actual.
- **cant_archivos**: el número de archivos presentes actualmente en el directorio.
- **cant_directorios**: el número de subdirectorios que existen en este directorio.
- **stats**: un puntero a la estructura stats_t, que contiene los metadatos del directorio.

### **Estructura del sistema de archivos (filesystem_t)**

El sistema de archivos en memoria se organiza mediante la estructura filesystem_t, que define las principales características del sistema. Sus campos son:

- **raiz**: un puntero al directorio raíz del sistema de archivos, el cual actúa como punto de entrada y contiene todos los archivos y subdirectorios.
- **max_size**: el tamaño máximo del sistema de archivos en memoria, que permite controlar la capacidad total del sistema.
- **current_size**: el tamaño actual ocupado por el sistema de archivos, lo cual incluye tanto archivos como directorios.

### **Estructura de metadatos (stats_t)**

La estructura stats_t es utilizada para almacenar metadatos relacionados tanto con archivos como con directorios. Sus campos son:

- **st_mode**: el modo del archivo o directorio.
- **st_nlink**: el número de enlaces (links) asociados al archivo o directorio.
- **st_uid**: el ID del usuario propietario del archivo o directorio.
- **st_gid**: el ID del grupo asociado al archivo o directorio.
- **st_size**: el tamaño del archivo en bytes.
- **st_atime**: el tiempo de último acceso al archivo o directorio, almacenado como un valor time_t.
- **st_mtime**: el tiempo de la última modificación de los datos, también representado como un valor time_t.

## **Ubicacion de un archivo dado un path**

En nuestra implementación, algunas de las funciones que necesitan localizar un archivo específico dado un path son:

- read
- write
- open

Estas funciones dependen de fs_open para localizar un archivo dentro del sistema de archivos. A continuación, se explica el proceso de búsqueda que realiza fs_open.

### **Paso 1: Llamada a fs_open**

Cuando se invoca fs_open, la función toma el sistema de archivos y el path del archivo a buscar. Si alguno de estos parámetros es nulo, la función imprime un mensaje de error.  En caso contrario, invoca la función search_file con el directorio raíz del sistema de archivos.

### **Paso 2: Búsqueda de archivos en el directorio**

La función search_file comienza analizando el path recibido. Para ello, utiliza strrchr para dividir el path en dos partes:

- **dir_path**: La parte del path que indica el directorio donde debería estar el archivo.
  - Por ejemplo, si el path es /subdir/archivo.txt, dir_path será /subdir.
  - Si el archivo está directamente en la raíz (/archivo.txt), dir_path será /.
- **file_name**: El nombre del archivo que se está buscando, extraído de la última parte del path. En el anterior ejemplo, file_name sería archivo.txt

Durante el procesamiento del path, se realizan las siguientes validaciones:

- **Path inválido**: Si el path termina con un / pero no incluye un nombre de archivo, se considera inválido.
- **Longitudes excesivas**: Si el directorio o el nombre del archivo exceden los límites establecidos (MAX_PATH o MAX_FILE_NAME), se reporta un error.

Con el path ya procesado, search_file comienza buscando el archivo en el directorio actual utilizando la función iter_dir. iter_dir recorre todos los archivos en el directorio y compara sus nombres con el nombre proporcionado. Si encuentra una coincidencia, retorna el archivo. Si no la encuentra, devuelve un puntero nulo.

### **Paso 3: Búsqueda recursiva en subdirectorios**

Si iter_dir no encuentra el archivo en el directorio actual, search_file recursivamente busca el archivo en todos los subdirectorios del directorio actual. Esto se hace mediante una llamada recursiva a search_file para cada subdirectorio. Si encuentra el archivo en alguno de los subdirectorios, lo retorna.

### **Paso 4: Retorno del resultado**

Si el archivo es encontrado en alguno de los directorios o subdirectorios, la función search_file retorna el puntero al archivo. Si el archivo no se encuentra en ningún lugar dentro del sistema de archivos, la función retorna nulo.

## **Funciones de FUSE implementadas**

- **init**: Se invoca al iniciar el sistema de archivos. Inicializa el sistema de archivos y prepara el archivo persistente para guardar los datos.
- **destroy**: Se ejecuta al desmontar el sistema de archivos. Libera los recursos asignados y guarda cualquier cambio en el archivo persistente.
- **getattr**: Obtiene atributos de un archivo o directorio (metadata).
- **readdir**: Lista el contenido de un directorio. Agrega los pseudo-directorios . y . . al buffer de salida, además de los archivos y subdirectorios contenidos en el directorio especificado.
- **read**: Lee datos de un archivo desde una posición específica (offset). Copia los datos al buffer de salida y actualiza el tiempo de acceso del archivo.
- **write**: Escribe datos en un archivo desde una posición específica. Si es necesario, expande el tamaño del archivo y actualiza el tiempo de modificación y acceso.
- **create**: Crea un archivo nuevo con los permisos especificados (mode).
- **open**: Abre un archivo para su uso posterior especificando un path.
- **opendir**: Abre un directorio para listar su contenido posteriormente. Retorna un identificador asociado al directorio.
- **mkdir**: Crea un nuevo directorio en el sistema de archivos.
- **rmdir**: Elimina un directorio vacío.
- **unlink**: Elimina un archivo del sistema de archivos.
- **mknod**: Crea un nodo en el sistema de archivos (un archivo regular en este caso).
- **utimens**: Actualiza las marcas de tiempo (acceso y modificación) de un archivo. Permite especificar tiempos personalizados o asignar la hora actual.

## **Persistencia**

El sistema de archivos implementado garantiza la persistencia de los datos entre ejecuciones mediante el uso de un archivo físico en disco, con la extensión .fisopfs. Este archivo actúa como la representación persistente del sistema de archivos, almacenando toda la información necesaria para reconstruir las estructuras en memoria durante la inicialización.

La persistencia se maneja mediante dos puntos principales en el ciclo de vida del sistema:

- **Montaje (fs_init)**:
Al iniciar el sistema de archivos, se verifica la existencia del archivo de persistencia especificado (o un archivo por defecto). Si el archivo ya existe, su contenido se lee y deserializa para reconstruir las estructuras de datos en memoria, asegurando que el estado previo del sistema sea restaurado correctamente. Si el archivo no existe, se inicializan estructuras vacías en memoria y se crea el archivo de persistencia.

- **Desmontaje (fs_destroy)**:
Al finalizar la ejecución, el estado actual del sistema de archivos en memoria se serializa y se escribe de vuelta al archivo en disco. Esto garantiza que los cambios realizados durante la ejecución se conserven para futuras instancias.

- **Pruebas**
Para ejecutar los test luego de montar en la carpeta prueba se deben ejecutar los siguientes comandos:
chmod +x correr_test.sh
./correr_test.sh

- **Salidas de pruebas**
Creacion de archivo:

```bash
$ /fisopfs# cd prueba
$ /fisopfs/prueba# touch testfile.txt
$ /fisopfs/prueba# ls
  testfile.txt
```
Creacion de directorio:

```bash
$ /fisopfs# cd prueba
$ /fisopfs/prueba# mkdir dir
$ /fisopfs/prueba# ls
  dir
```

Stats:

```bash

oot@47c77605a8b5:/fisopfs/prueba# echo "hola" > stat_test.txt
root@47c77605a8b5:/fisopfs/prueba# stat stat_test.txt
  File: stat_test.txt
  Size: 5               Blocks: 8          IO Block: 4096   regular file
  Device: 803h/2051d      Inode: 928858      Links: 1
  Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
  Access: 2024-12-02 00:43:04.391312463 +0000
  Modify: 2024-12-02 00:43:04.391312463 +0000
  Change: 2024-12-02 00:43:04.391312463 +0000
  Birth: -
root@47c77605a8b5:/fisopfs/prueba# sleep 
root@47c77605a8b5:/fisopfs/prueba# sleep 1
root@47c77605a8b5:/fisopfs/prueba# touch stat_test.txt
root@47c77605a8b5:/fisopfs/prueba# stat stat_test.txt
  File: stat_test.txt
  Size: 5               Blocks: 8          IO Block: 4096   regular file
  Device: 803h/2051d      Inode: 928858      Links: 1
  Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
  Access: 2024-12-02 00:43:39.128322164 +0000
  Modify: 2024-12-02 00:43:39.128322164 +0000
  Change: 2024-12-02 00:43:39.128322164 +0000
  Birth: -
```
En este caso vemos que el tiempo de acceso cambia

Lectura de archivo:

```bash
  root@47c77605a8b5:/fisopfs/prueba# echo "hola" > cat_test.txt
  root@47c77605a8b5:/fisopfs/prueba# cat cat_test.txt
  hola
```
Escritura de archivo:

```bash
  root@47c77605a8b5:/fisopfs/prueba# echo "hola" > write_test.txt
  root@47c77605a8b5:/fisopfs/prueba# echo "extra" >> write_test.txt
  root@47c77605a8b5:/fisopfs/prueba# cat write_test.txt
  hola
  extra
```
Lectura de directorios:

```bash
  root@47c77605a8b5:/fisopfs/prueba# touch root_file.txt
  root@47c77605a8b5:/fisopfs/prueba# mkdir test_dir
  root@47c77605a8b5:/fisopfs/prueba# touch test_dir/subdir_file
  root@47c77605a8b5:/fisopfs/prueba# ls
  root_file.txt  test_dir
  root@47c77605a8b5:/fisopfs/prueba# cd test_dir
  root@47c77605a8b5:/fisopfs/prueba/test_dir# ls
  subdir_file

```