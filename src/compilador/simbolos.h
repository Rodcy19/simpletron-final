/*
 * simbolos.h - Tabla de simbolos del compilador de Simple
 *
 * Guarda cuatro tipos de simbolos:
 *   LINEA      numero de linea de Simple -> direccion de su primera instruccion
 *   VARIABLE   variable de una letra     -> direccion de su palabra de datos
 *   CONSTANTE  constante entera          -> direccion de su palabra de datos
 *   TEMPORAL   resultado intermedio de una expresion (let)
 *
 * La ubicacion vale -1 mientras el simbolo no tiene direccion asignada. Las
 * lineas se ubican en la primera pasada; variables, constantes y temporales
 * se ubican en la segunda, a continuacion del codigo.
 */
#ifndef SIMBOLOS_H
#define SIMBOLOS_H

#include <stdio.h>
#include <stddef.h>

#define SIM_MAX 256

typedef enum {
    SIM_LINEA,
    SIM_VARIABLE,
    SIM_CONSTANTE,
    SIM_TEMPORAL
} TipoSimbolo;

typedef struct {
    TipoSimbolo tipo;
    int valor;       /* LINEA: numero | VARIABLE: letra | CONSTANTE: valor | TEMPORAL: indice */
    int ubicacion;   /* direccion en memoria, o -1 si aun no se asigna */
} Simbolo;

typedef struct {
    Simbolo items[SIM_MAX];
    int cantidad;
} TablaSimbolos;

void tabla_iniciar(TablaSimbolos *t);

/* Devuelve el indice del simbolo, o -1 si no existe */
int tabla_buscar(const TablaSimbolos *t, TipoSimbolo tipo, int valor);

/* Busca el simbolo y, si no existe, lo agrega. Devuelve -1 si la tabla esta llena */
int tabla_obtener(TablaSimbolos *t, TipoSimbolo tipo, int valor);

/* Nombre corto del simbolo para los comentarios del archivo SML (x, t0, const 5, linea 30) */
void tabla_nombre_simbolo(const Simbolo *s, char *destino, size_t tam);

/* Imprime la tabla completa */
void tabla_imprimir(const TablaSimbolos *t, FILE *f);

#endif
