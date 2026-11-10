#!/bin/bash
# Compila el simulador y ejecuta cada prueba comparando contra la salida
# esperada en pruebas/salidas/.
# Uso: ./pruebas/correr_pruebas.sh

cd "$(dirname "$0")/.." || exit 1
mkdir -p bin
gcc -Wall -Wextra -std=c99 -o bin/simpletron src/simpletron/simpletron.c || exit 1

total=0
ok=0
obtenida=$(mktemp)

comparar() {
    total=$((total + 1))
    if diff <(tr -d '\r' < "pruebas/salidas/$1.out") <(tr -d '\r' < "$obtenida") > /dev/null; then
        echo "OK     $1"
        ok=$((ok + 1))
    else
        echo "FALLO  $1"
    fi
}

# prueba <nombre> <programa sml>
# El programa y los datos se mandan juntos por la entrada estandar (teclado).
prueba() {
    cat "programas/sml/$2.sml" "pruebas/entradas/$1.in" | ./bin/simpletron > "$obtenida"
    comparar "$1"
}

# prueba_archivo <nombre> <ruta del programa> <archivo de datos o /dev/null>
# El programa se carga desde archivo (argumento) y los datos vienen de la entrada.
prueba_archivo() {
    ./bin/simpletron "$2" < "$3" > "$obtenida"
    comparar "$1"
}

# Simulador base
prueba suma             suma
prueba mayor_a          mayor
prueba mayor_b          mayor
prueba centinela        centinela
prueba operaciones      operaciones
prueba division_cero    operaciones
prueba entrada_invalida operaciones

# Carga desde archivo
prueba_archivo archivo_suma        programas/sml/suma.sml                 pruebas/entradas/suma.in
prueba_archivo archivo_inexistente programas/sml/no_existe.sml            /dev/null
prueba_archivo archivo_invalido    pruebas/entradas/programa_invalido.sml /dev/null

# Extensiones: MODULE (34), POWER (35), NEWLINE (12)
prueba_archivo modulo            programas/sml/modulo.sml      pruebas/entradas/modulo.in
prueba_archivo modulo_cero       programas/sml/modulo.sml      pruebas/entradas/modulo_cero.in
prueba_archivo potencia          programas/sml/potencia.sml    pruebas/entradas/potencia.in
prueba_archivo potencia_overflow programas/sml/potencia.sml    pruebas/entradas/potencia_overflow.in
prueba_archivo potencia_negativa programas/sml/potencia.sml    pruebas/entradas/potencia_negativa.in
prueba_archivo potencia_cero     programas/sml/potencia.sml    pruebas/entradas/potencia_cero.in
prueba_archivo salto_linea       programas/sml/salto_linea.sml pruebas/entradas/salto_linea.in

rm -f "$obtenida"
echo "---"
echo "$ok de $total pruebas correctas"
[ "$ok" -eq "$total" ]
