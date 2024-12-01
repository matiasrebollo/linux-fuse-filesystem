#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
TEST_FILE="cat_test.txt"
TEST_CONTENT="Hola, este es un test de cat."

echo "Creando archivo '$TEST_FILE'..."
echo "$TEST_CONTENT" > "$MOUNT_POINT/$TEST_FILE"

echo "Leyendo archivo '$TEST_FILE' con cat..."
OUTPUT=$(cat "$MOUNT_POINT/$TEST_FILE")

if [ "$OUTPUT" == "$TEST_CONTENT" ]; then
    echo "✔️  El comando cat funcionó correctamente."
else
    echo "❌  ERROR: El contenido leído no coincide con el escrito."
    echo "Contenido esperado: $TEST_CONTENT"
    echo "Contenido leído: $OUTPUT"
    exit 1
fi

# Limpieza
echo "Eliminando archivo '$TEST_FILE'..."
rm "$MOUNT_POINT/$TEST_FILE"

echo "✔️  Test de cat completado con éxito."
