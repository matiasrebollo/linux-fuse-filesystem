#!/bin/bash

# Variables
MOUNT_POINT="./prueba"
FILESYSTEM_BINARY="./fisopfs"
TEST_DIR="./Tests"

# Colores para mensajes (opcional)
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

# Asegurarse de que el filesystem esté compilado
if [ ! -f "$FILESYSTEM_BINARY" ]; then
    echo -e "${RED}[ERROR] El binario del sistema de archivos no se encontró en $FILESYSTEM_BINARY. Compila primero.${RESET}"
    exit 1
fi

# Montar el sistema de archivos
echo "Montando el sistema de archivos..."
$FILESYSTEM_BINARY $MOUNT_POINT &
FS_PID=$!
sleep 1

# Verificar que el sistema de archivos esté montado
if [ ! -d "$MOUNT_POINT" ]; then
    echo -e "${RED}[ERROR] No se pudo montar el sistema de archivos en $MOUNT_POINT.${RESET}"
    kill $FS_PID
    exit 1
fi

# Ejecutar todos los tests
echo "Ejecutando tests..."
for test_script in $TEST_DIR/*.sh; do
    echo "-----------------------------------------"
    echo "Ejecutando $(basename "$test_script")..."
    bash "$test_script"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[SUCCESS] $(basename "$test_script") completó correctamente.${RESET}"
    else
        echo -e "${RED}[FAIL] $(basename "$test_script") falló.${RESET}"
        kill $FS_PID
        exit 1
    fi
done


echo -e "${GREEN}Todos los tests finalizaron correctamente.${RESET}"
