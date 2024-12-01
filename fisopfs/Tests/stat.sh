#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
TEST_FILE="stat_test.txt"
TEST_CONTENT="Hola, este es un test de stat."

echo "Creando archivo '$TEST_FILE'..."
echo "$TEST_CONTENT" > "$MOUNT_POINT/$TEST_FILE"

echo "Leyendo stats de '$TEST_FILE' "
touch "$MOUNT_POINT/$TEST_FILE"
OUTPUT1=$(stat "$MOUNT_POINT/$TEST_FILE")
sleep 1
touch "$MOUNT_POINT/$TEST_FILE"
OUTPUT2="$(stat "$MOUNT_POINT/$TEST_FILE")"

if [ "$OUTPUT1" != "$OUTPUT2" ]; then
    echo "✔️  El comando stat actualizó el tiempo de acceso correctamente."
else
    echo "❌  ERROR: El comando stat no actualizó correctamente."
    exit 0
fi

# Limpieza
echo "Eliminando archivo '$TEST_FILE'..."
rm "$MOUNT_POINT/$TEST_FILE"

echo "✔️  Test de cat completado con éxito."
