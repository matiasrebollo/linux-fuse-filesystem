#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
ROOT_FILE="root_file.txt"
SUB_DIR="test_dir"
SUB_DIR_FILE="subdir_file.txt"

echo "Preparando entorno de prueba para 'ls'..."
touch "$MOUNT_POINT/$ROOT_FILE"
mkdir "$MOUNT_POINT/$SUB_DIR"
touch "$MOUNT_POINT/$SUB_DIR/$SUB_DIR_FILE"

echo "Ejecutando 'ls' en el root..."
ROOT_LS_OUTPUT=$(ls "$MOUNT_POINT")

echo "Verificando contenido listado por 'ls' en el root..."
if echo "$ROOT_LS_OUTPUT" | grep -q "$ROOT_FILE" && echo "$ROOT_LS_OUTPUT" | grep -q "$SUB_DIR"; then
    echo "✔️  'ls' en root funciona correctamente."
else
    echo "❌  ERROR: 'ls' en root no muestra correctamente los elementos esperados."
    echo "Contenido listado por 'ls' en root:"
    echo "$ROOT_LS_OUTPUT"
    exit 1
fi

echo "Ejecutando 'ls' dentro del directorio '$SUB_DIR'..."
SUB_DIR_LS_OUTPUT=$(ls "$MOUNT_POINT/$SUB_DIR")

echo "Verificando contenido listado por 'ls' en el directorio..."
if echo "$SUB_DIR_LS_OUTPUT" | grep -q "$SUB_DIR_FILE"; then
    echo "✔️  'ls' dentro del directorio '$SUB_DIR' funciona correctamente."
else
    echo "❌  ERROR: 'ls' dentro del directorio '$SUB_DIR' no muestra correctamente los elementos esperados."
    echo "Contenido listado por 'ls' en el directorio '$SUB_DIR':"
    echo "$SUB_DIR_LS_OUTPUT"
    exit 1
fi

# Limpieza
echo "Limpieza de los elementos de prueba..."
rm -f "$MOUNT_POINT/$ROOT_FILE"
rm -f "$MOUNT_POINT/$SUB_DIR/$SUB_DIR_FILE"
rmdir "$MOUNT_POINT/$SUB_DIR"

echo "✔️  Test de 'ls' completado con éxito."


