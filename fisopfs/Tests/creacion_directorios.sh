#!/bin/bash

# Variables
MOUNT_POINT="./prueba"  # Cambia esto al punto de montaje de tu sistema de archivos
TEST_DIR_1="dir1"
TEST_DIR_2="dir2"

# Función para verificar si un directorio existe
check_directory_exists() {
    local dir_path=$1
    if [ -d "$dir_path" ]; then
        echo "✔️  El directorio '$dir_path' existe."
    else
        echo "❌  ERROR: El directorio '$dir_path' no se creó correctamente."
        exit 1
    fi
}

# Comienza el test
echo "Iniciando test de creación de directorios..."

# Crea directorios en el sistema de archivos montado
echo "Creando directorio '$TEST_DIR_1'..."
mkdir "$MOUNT_POINT/$TEST_DIR_1"

echo "Creando directorio '$TEST_DIR_2'..."
mkdir "$MOUNT_POINT/$TEST_DIR_2"


# Verifica si los directorios fueron creados correctamente
check_directory_exists "$MOUNT_POINT/$TEST_DIR_1"
check_directory_exists "$MOUNT_POINT/$TEST_DIR_2"

echo "✔️  Todos los directorios fueron creados correctamente."

mkdir "$MOUNT_POINT/$TEST_DIR_1" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "✔️  No se puede crear un directorio que ya existe (comportamiento esperado)."
else
    echo "❌  ERROR: Se permitió crear un directorio duplicado."
    exit 1
fi

touch "$MOUNT_POINT/$TEST_DIR_1/testfile.txt"
if [ -f "$MOUNT_POINT/$TEST_DIR_1/testfile.txt" ]; then
    echo "✔️  El archivo testfile.txt fue creado correctamente en $TEST_DIR_1."
else
    echo "❌  ERROR: No se pudo crear testfile.txt en $TEST_DIR_1."
    exit 1
fi


# Limpieza (opcional)
echo "Eliminando los directorios de prueba..."
rmdir "$MOUNT_POINT/$TEST_DIR_1"
rmdir "$MOUNT_POINT/$TEST_DIR_2"

echo "Test finalizado con éxito."
