#!/bin/bash
# Compila el simulador y ejecuta cada prueba: programa SML + entradas,
# comparando contra la salida esperada en pruebas/salidas/.
# Uso: ./pruebas/correr_pruebas.sh

cd "$(dirname "$0")/.." || exit 1
mkdir -p bin
gcc -Wall -Wextra -std=c99 -o bin/simpletron src/simpletron/simpletron.c || exit 1

total=0
ok=0
obtenida=$(mktemp)

# prueba <nombre> <programa sml>
prueba() {
    nombre=$1
    programa=$2
    total=$((total + 1))
    cat "programas/sml/$programa.sml" "pruebas/entradas/$nombre.in" \
        | ./bin/simpletron | tr -d '\r' > "$obtenida"
    if diff <(tr -d '\r' < "pruebas/salidas/$nombre.out") "$obtenida" > /dev/null; then
        echo "OK     $nombre"
        ok=$((ok + 1))
    else
        echo "FALLO  $nombre"
    fi
}

prueba suma            suma
prueba mayor_a         mayor
prueba mayor_b         mayor
prueba centinela       centinela
prueba operaciones     operaciones
prueba division_cero   operaciones
prueba entrada_invalida operaciones

rm -f "$obtenida"
echo "---"
echo "$ok de $total pruebas correctas"
[ "$ok" -eq "$total" ]
