#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
TEST_FILE="write_test.txt"
INITIAL_CONTENT="Contenido inicial."
ADDITIONAL_CONTENT="Texto adicional."

echo "Creando archivo '$TEST_FILE'..."
echo "$INITIAL_CONTENT" > "$MOUNT_POINT/$TEST_FILE"

echo "Escribiendo contenido adicional en '$TEST_FILE'..."
echo "$ADDITIONAL_CONTENT" >> "$MOUNT_POINT/$TEST_FILE"

echo "Verificando contenido final del archivo..."
EXPECTED_CONTENT="$INITIAL_CONTENT
$ADDITIONAL_CONTENT"
ACTUAL_CONTENT=$(cat "$MOUNT_POINT/$TEST_FILE")

if [ "$ACTUAL_CONTENT" == "$EXPECTED_CONTENT" ]; then
    echo "✔️  La escritura funcionó correctamente."
else
    echo "❌  ERROR: El contenido del archivo no coincide con lo esperado."
    echo "Contenido esperado:"
    echo "$EXPECTED_CONTENT"
    echo "Contenido actual:"
    echo "$ACTUAL_CONTENT"
    exit 1
fi

# Limpieza
echo "Eliminando archivo '$TEST_FILE'..."
rm "$MOUNT_POINT/$TEST_FILE"

echo "✔️  Test de escritura completado con éxito."
