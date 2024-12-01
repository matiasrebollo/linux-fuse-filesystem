# **TP3: Filesystem FUSE**
## **Introducción**

El objetivo de este proyecto es desarrollar un sistema de archivos básico, con funciones esenciales para gestionar directorios y archivos.

El sistema de archivos usa el mecanismo de *FUSE* (Filesystem in USErspace) provisto por el kernel, que nos permitirá definir en modo usuario la implementación de un filesystem. Gracias a ello, el mismo tendrá la interfaz *VFS* y podrá ser accedido con las syscalls y programas habituales (read, open, ls, etc).

Está diseñado para simular el comportamiento de un sistema de archivos real, proporcionando una estructura que soporta la creación de archivos, la búsqueda de archivos y directorios mediante rutas, así como la lectura y escritura en dichos archivos entre otras cosas. Es importante destacar que las funciones implementadas en este proyecto representan un subconjunto de las operaciones soportadas por *FUSE*, enfocándonos en las más relevantes para la gestión básica de archivos y directorios.

Cabe aclarar que el sistema de archivos implementado tiene una estructura jerárquica limitada: solo existe un unico nivel de recursion en los directorios. El directorio raíz puede contener archivos y subdirectorios, mientras que los subdirectorios solo pueden contener archivos. No se permite una jerarquía de mayor profundidad.

La implementación del filesystem será enteramente en memoria: tanto archivos como directorios serán representados mediante estructuras que vivirán en memoria RAM. Aún así, los datos del filesystem estarán representados de manera persistente en disco por un archivo.
