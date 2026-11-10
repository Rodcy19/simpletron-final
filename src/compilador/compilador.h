/*
 * compilador.h - Compilador del lenguaje Simple a lenguaje de maquina SML
 *
 * Lenguaje Simple (una instruccion por linea, con numero de linea ascendente):
 *
 *   10 rem comentario                 cualquier texto
 *   20 input x                        lee un entero en la variable x
 *   30 let y = (x + 2) * 3            asigna el valor de una expresion
 *   40 print y                        imprime una variable o constante
 *   50 goto 80                        salta a la linea 80
 *   60 if x >= y goto 80              salto condicional (< <= > >= == !=)
 *   70 end                            termina; debe ser la ultima linea
 *
 * Variables de una letra minuscula, constantes enteras (-9999 a 9999),
 * operadores + - * / ^ . Todo va en minusculas salvo los comentarios.
 *
 * Funcionamiento en dos pasadas:
 *   Primera pasada: lee el fuente, registra lineas, variables y constantes
 *     en la tabla de simbolos y genera instrucciones SML preliminares. Los
 *     operandos de datos y los saltos hacia adelante quedan marcados como
 *     referencias pendientes.
 *   Segunda pasada: ubica los datos a continuacion del codigo, resuelve las
 *     referencias pendientes y completa las instrucciones.
 */
#ifndef COMPILADOR_H
#define COMPILADOR_H

#include <stdio.h>
#include "simbolos.h"

#define SML_CAPACIDAD 100

typedef struct {
    int palabra;       /* opcode * 100 + operando (el operando puede faltar) */
    int ref;           /* simbolo pendiente de resolver, o -1 */
    int lineaFuente;   /* linea de Simple que origino la instruccion */
    char nota[40];     /* texto para el comentario del archivo SML */
} Instruccion;

typedef struct {
    Instruccion codigo[SML_CAPACIDAD];
    int cantidadCodigo;
    int totalPalabras;               /* codigo + datos */
    int memoria[SML_CAPACIDAD];      /* imagen final de memoria */
    TablaSimbolos tabla;
    int errores;
    int renglon;                     /* renglon del archivo que se procesa */
    int lineaActual;                 /* numero de linea de Simple que se procesa */
    int finVisto;
    int memoriaLlena;
} Compilador;

/* Compila el programa Simple leido de fuente (primera y segunda pasada).
   Los errores se informan en la salida estandar.
   Devuelve 0 si compilo sin errores, 1 si hubo errores. */
int compilar(FILE *fuente, Compilador *c);

/* Escribe el programa SML (con comentarios) listo para cargarse en el
   simulador. Solo debe usarse si compilar() devolvio 0. */
void escribir_sml(const Compilador *c, FILE *salida, const char *nombreFuente);

#endif
