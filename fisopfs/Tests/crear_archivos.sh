#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
TEST_FILE="testfile.txt"

echo "Creando archivo '$TEST_FILE'..."
touch "$MOUNT_POINT/$TEST_FILE"

if [ -f "$MOUNT_POINT/$TEST_FILE" ]; then
    echo "✔️  El archivo '$TEST_FILE' fue creado correctamente."
else
    echo "❌  ERROR: El archivo '$TEST_FILE' no se creó correctamente."
    exit 1
fi

echo "Eliminando archivo '$TEST_FILE'..."
rm "$MOUNT_POINT/$TEST_FILE"

if [ ! -f "$MOUNT_POINT/$TEST_FILE" ]; then
    echo "✔️  El archivo '$TEST_FILE' fue eliminado correctamente."
else
    echo "❌  ERROR: El archivo '$TEST_FILE' no se eliminó correctamente."
    exit 1
fi
