# Simpletron Final: compilador simple 

Proyecto final: computadora virtual Simpletron (con 100 palabras de memoria), ejecuta lenjuaje maquina SML e instrucciones extendidas

**Integrante:** Rodolfo Coyotzi Montes - UPAEP

## Requisitos

- gcc y bash (probado con MSYS2/UCRT64 en Windows y Git Bash), se recomienda Git Bash o MSYS2 en windows.

## Descargar

```bash
git clone https://github.com/Rodcy19/simpletron-final.git
cd simpletron-final
```

## Compilar

```bash
mkdir -p bin
gcc -Wall -Wextra -std=c99 -o bin/simpletron src/simpletron/simpletron.c
gcc -Wall -Wextra -std=c99 -o bin/expresiones src/expresiones/expresiones.c src/expresiones/probar_expresiones.c
gcc -Wall -Wextra -std=c99 -o bin/compilador src/compilador/compilador.c src/compilador/simbolos.c \
    src/compilador/main_compilador.c src/expresiones/expresiones.c
```

## Ejecutar

### Simulador
```bash
./bin/simpletron programas/sml/suma.sml      # carga el programa desde archivo
./bin/simpletron                             # modo interactivo: se captura el programa, termina con 9999
```
Ejemplo (`suma.sml`, entradas 5 y 7): imprime `12`, luego los registros y el volcado de memoria.

### Evaluador de expresiones
```bash
echo "(2 + 3) * 4 ^ 2" | ./bin/expresiones
```
```
Expresion: (2 + 3) * 4 ^ 2
Agrupacion: correcta (profundidad 1)
Postfija:  2 3 + 4 2 ^ *
Resultado: 80
```

### Compilador de Simple
```bash
./bin/compilador [-t] entrada.simple [salida.sml]    # -t muestra la tabla de símbolos
./bin/compilador -t programas/simple/suma.simple bin/suma.sml
./bin/simpletron bin/suma.sml
```
Si hay errores el compilador indica el renglon y la linea, no genera archivo 

## Control de versiones

Ramas `feature/extensiones`, `feature/expresiones` y `feature/compilador`, integradas a `main` con `merge --no-ff` y etiquetadas `v1.1`, `v1.2` y `v2.0`. Ver `evidencias/git-branch/`.
