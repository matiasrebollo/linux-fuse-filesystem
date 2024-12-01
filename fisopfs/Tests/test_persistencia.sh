#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
DISK_FILE="persistence_file.fisopfs"
ROOT_FILE="persist_file.txt"
SUB_DIR="persist_dir"
SUB_DIR_FILE="persist_subdir_file.txt"

if mount | grep "$MOUNT_POINT" > /dev/null; then
    echo "Sistema de archivos aún montado. Intentando desmontar..."
    # Intentar usar fusermount si está disponible
    if command -v fusermount &> /dev/null; then
        fusermount -u "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    else
        # Usar umount como alternativa
        umount "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    fi
else
    echo "El sistema de archivos no está montado."
fi

# Eliminar el directorio de montaje y archivos residuales
rm -rf "$MOUNT_POINT" || true
mkdir -p "$MOUNT_POINT"
rm -f "$DISK_FILE"
rm -f "$ROOT_FILE" 2>/dev/null
rm -rf "$SUB_DIR" 2>/dev/null

echo "Iniciando test de persistencia..."
echo "Montando sistema de archivos por primera vez..."
./fisopfs "$MOUNT_POINT" --filedisk "$DISK_FILE" || { echo "❌  ERROR: No se pudo montar el sistema de archivos."; exit 1; }

echo "Creando archivo y directorio para prueba de persistencia..."
touch "$MOUNT_POINT/$ROOT_FILE"
mkdir "$MOUNT_POINT/$SUB_DIR"
touch "$MOUNT_POINT/$SUB_DIR/$SUB_DIR_FILE"

echo "Desmontando el sistema de archivos..."
# Verifica si el sistema está montado y desmontarlo
if mount | grep "$MOUNT_POINT" > /dev/null; then
    echo "Sistema de archivos montado. Procediendo al desmontaje..."
    if command -v fusermount &> /dev/null; then
        fusermount -u "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    else
        umount "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    fi
else
    echo "El sistema de archivos no está montado. No es necesario desmontar."
fi

echo "Volviendo a montar el sistema de archivos..."
./fisopfs "$MOUNT_POINT" --filedisk "$DISK_FILE" || { echo "❌  ERROR: No se pudo volver a montar el sistema de archivos."; exit 1; }

echo "Verificando persistencia de los elementos creados..."
if [ -f "$MOUNT_POINT/$ROOT_FILE" ] && [ -d "$MOUNT_POINT/$SUB_DIR" ] && [ -f "$MOUNT_POINT/$SUB_DIR/$SUB_DIR_FILE" ]; then
    echo "✔️  Los elementos persisten correctamente tras el desmontaje y remonte."
else
    echo "❌  ERROR: Los elementos no persisten tras el desmontaje y remonte."
    echo "Contenido en root tras remonte:"
    ls -l "$MOUNT_POINT"
    echo "Contenido en directorio '$SUB_DIR' tras remonte:"
    ls -l "$MOUNT_POINT/$SUB_DIR"
    exit 1
fi

# Limpieza después del test
echo "Limpieza de los elementos de prueba..."
rm -f "$MOUNT_POINT/$ROOT_FILE" 2>/dev/null
rm -f "$MOUNT_POINT/$SUB_DIR/$SUB_DIR_FILE" 2>/dev/null
rmdir "$MOUNT_POINT/$SUB_DIR" 2>/dev/null
# Intentar desmontar nuevamente si es necesario
if mount | grep "$MOUNT_POINT" > /dev/null; then
    echo "Sistema de archivos montado durante la limpieza. Desmontando..."
    if command -v fusermount &> /dev/null; then
        fusermount -u "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    else
        umount "$MOUNT_POINT" || { echo "❌  ERROR: No se pudo desmontar el sistema de archivos."; exit 1; }
    fi
fi

rm -f "$DISK_FILE"

echo "✔️  Test de persistencia completado con éxito."
