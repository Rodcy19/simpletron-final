#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compilador.h"
#include "../expresiones/expresiones.h"

/* Codigos de operacion SML (ver docs/opcodes) */
#define SML_READ        10
#define SML_WRITE       11
#define SML_LOAD        20
#define SML_STORE       21
#define SML_ADD         30
#define SML_SUBTRACT    31
#define SML_DIVIDE      32
#define SML_MULTIPLY    33
#define SML_POWER       35
#define SML_BRANCH      40
#define SML_BRANCHNEG   41
#define SML_BRANCHZERO  42
#define SML_HALT        43

#define MAX_TEMPORALES  16
#define LINEA_MAX       9999
#define LONG_LINEA      256

/* ------------------------------------------------------------------ */
/* Mensajes de error                                                   */
/* ------------------------------------------------------------------ */

/* Error ligado al renglon que se esta leyendo */
static void error_renglon(Compilador *c, const char *formato, ...)
{
    va_list args;

    if (c->lineaActual > 0)
    {
        printf("Error en el renglon %d (linea Simple %d): ", c->renglon, c->lineaActual);
    }
    else
    {
        printf("Error en el renglon %d: ", c->renglon);
    }
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    printf("\n");
    c->errores++;
}

/* Error que no pertenece a un renglon concreto */
static void error_global(Compilador *c, const char *formato, ...)
{
    va_list args;

    printf("Error: ");
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    printf("\n");
    c->errores++;
}

/* ------------------------------------------------------------------ */
/* Emision de instrucciones SML preliminares                           */
/* ------------------------------------------------------------------ */

static Instruccion *nueva_instruccion(Compilador *c)
{
    Instruccion *in;

    if (c->cantidadCodigo >= SML_CAPACIDAD)
    {
        if (!c->memoriaLlena)
        {
            c->memoriaLlena = 1;
            error_renglon(c, "el codigo excede las %d palabras de memoria", SML_CAPACIDAD);
        }
        return NULL;
    }

    in = &c->codigo[c->cantidadCodigo++];
    in->lineaFuente = c->lineaActual;
    in->ref = -1;
    return in;
}

/* Instruccion cuyo operando es un simbolo: queda pendiente (ref) y se
   completa en la segunda pasada. */
static void emitir(Compilador *c, int opcode, int simbolo, const char *mnemonico)
{
    Instruccion *in = nueva_instruccion(c);
    char nombre[24];

    if (in == NULL)
    {
        return;
    }
    tabla_nombre_simbolo(&c->tabla.items[simbolo], nombre, sizeof nombre);
    in->palabra = opcode * 100;
    in->ref = simbolo;
    snprintf(in->nota, sizeof in->nota, "%s %s", mnemonico, nombre);
}

/* Instruccion sin operando (HALT) */
static void emitir_simple(Compilador *c, int opcode, const char *mnemonico)
{
    Instruccion *in = nueva_instruccion(c);

    if (in == NULL)
    {
        return;
    }
    in->palabra = opcode * 100;
    snprintf(in->nota, sizeof in->nota, "%s", mnemonico);
}

/* Instruccion con direccion ya conocida */
static void emitir_directo(Compilador *c, int opcode, int direccion, const char *mnemonico)
{
    Instruccion *in = nueva_instruccion(c);

    if (in == NULL)
    {
        return;
    }
    in->palabra = opcode * 100 + direccion;
    snprintf(in->nota, sizeof in->nota, "%s %02d", mnemonico, direccion);
}

/* Salto a una linea de Simple. Si la linea ya fue compilada (salto hacia
   atras) se resuelve de inmediato; si no (salto hacia adelante) se marca
   como referencia no resuelta para la segunda pasada. */
static void emitir_salto(Compilador *c, int opcode, int lineaDestino, const char *mnemonico)
{
    Instruccion *in;
    int simbolo = tabla_obtener(&c->tabla, SIM_LINEA, lineaDestino);

    if (simbolo < 0)
    {
        error_renglon(c, "tabla de simbolos llena");
        return;
    }
    in = nueva_instruccion(c);
    if (in == NULL)
    {
        return;
    }
    in->palabra = opcode * 100;
    snprintf(in->nota, sizeof in->nota, "%s linea %d", mnemonico, lineaDestino);

    if (c->tabla.items[simbolo].ubicacion >= 0)
    {
        in->palabra += c->tabla.items[simbolo].ubicacion;
    }
    else
    {
        in->ref = simbolo;
    }
}

/* ------------------------------------------------------------------ */
/* Lectura del texto fuente                                            */
/* ------------------------------------------------------------------ */

static void saltar_espacios(const char **p)
{
    while (isspace((unsigned char)**p))
    {
        (*p)++;
    }
}

static int fin_de_linea(const char *p)
{
    saltar_espacios(&p);
    return *p == '\0';
}

/* Copia una palabra formada solo por letras */
static void leer_palabra(const char **p, char *destino, size_t tam)
{
    size_t n = 0;

    saltar_espacios(p);
    while (isalpha((unsigned char)**p))
    {
        if (n + 1 < tam)
        {
            destino[n++] = **p;
        }
        (*p)++;
    }
    destino[n] = '\0';
}

/* Lee una variable de una letra minuscula. Devuelve 1 si lo logro. */
static int leer_variable(Compilador *c, const char **p, int *simbolo)
{
    saltar_espacios(p);

    if (isupper((unsigned char)**p))
    {
        error_renglon(c, "las variables deben ser una letra minuscula");
        return 0;
    }
    if (!islower((unsigned char)**p))
    {
        error_renglon(c, "se esperaba una variable (una letra minuscula)");
        return 0;
    }
    if (isalnum((unsigned char)(*p)[1]))
    {
        error_renglon(c, "las variables son de una sola letra");
        return 0;
    }

    *simbolo = tabla_obtener(&c->tabla, SIM_VARIABLE, **p);
    if (*simbolo < 0)
    {
        error_renglon(c, "tabla de simbolos llena");
        return 0;
    }
    (*p)++;
    return 1;
}

/* Lee una variable o una constante entera. Devuelve 1 si lo logro. */
static int leer_operando(Compilador *c, const char **p, int *simbolo)
{
    saltar_espacios(p);

    if (isdigit((unsigned char)**p) ||
        (**p == '-' && isdigit((unsigned char)(*p)[1])))
    {
        char *fin;
        long v = strtol(*p, &fin, 10);

        if (v < EXPR_VALOR_MIN || v > EXPR_VALOR_MAX)
        {
            error_renglon(c, "constante fuera de rango (-9999 a 9999)");
            return 0;
        }
        *simbolo = tabla_obtener(&c->tabla, SIM_CONSTANTE, (int)v);
        if (*simbolo < 0)
        {
            error_renglon(c, "tabla de simbolos llena");
            return 0;
        }
        *p = fin;
        return 1;
    }

    return leer_variable(c, p, simbolo);
}

/* Lee un numero de linea destino (para goto) */
static int leer_linea_destino(Compilador *c, const char **p, int *destino)
{
    char *fin;
    long v;

    saltar_espacios(p);
    if (!isdigit((unsigned char)**p))
    {
        error_renglon(c, "se esperaba un numero de linea");
        return 0;
    }
    v = strtol(*p, &fin, 10);
    if (v < 1 || v > LINEA_MAX)
    {
        error_renglon(c, "numero de linea fuera de rango (1 a %d)", LINEA_MAX);
        return 0;
    }
    *destino = (int)v;
    *p = fin;
    return 1;
}

/* Lee uno de los comparadores < <= > >= == != */
static int leer_comparador(Compilador *c, const char **p, char op[3])
{
    saltar_espacios(p);

    op[0] = **p;
    op[1] = '\0';
    op[2] = '\0';

    if (op[0] != '<' && op[0] != '>' && op[0] != '=' && op[0] != '!')
    {
        error_renglon(c, "se esperaba un operador de comparacion (< <= > >= == !=)");
        return 0;
    }
    (*p)++;
    if (**p == '=')
    {
        op[1] = '=';
        (*p)++;
    }
    if ((op[0] == '=' || op[0] == '!') && op[1] != '=')
    {
        error_renglon(c, "operador de comparacion no valido (use == o !=)");
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Generacion de codigo para cada comando                              */
/* ------------------------------------------------------------------ */

static int temporal_libre(int usados[])
{
    int i;

    for (i = 0; i < MAX_TEMPORALES; i++)
    {
        if (!usados[i])
        {
            usados[i] = 1;
            return i;
        }
    }
    return -1;
}

static int opcode_operador(int op)
{
    switch (op)
    {
        case '+':
            return SML_ADD;
        case '-':
            return SML_SUBTRACT;
        case '*':
            return SML_MULTIPLY;
        case '/':
            return SML_DIVIDE;
        default:
            return SML_POWER;
    }
}

static const char *mnemonico_operador(int op)
{
    switch (op)
    {
        case '+':
            return "ADD";
        case '-':
            return "SUBTRACT";
        case '*':
            return "MULTIPLY";
        case '/':
            return "DIVIDE";
        default:
            return "POWER";
    }
}

/* let: genera el codigo a partir de la expresion en notacion postfija.
   Los operandos se llevan en una pila de simbolos; cada operador carga el
   primer operando en el acumulador, opera con el segundo y guarda el
   resultado en un temporal (o en la variable destino si es el ultimo). */
static void compilar_let(Compilador *c, int variable, const Token pf[], int n)
{
    int pilaSim[EXPR_MAX_TOKENS];
    int pilaTemp[EXPR_MAX_TOKENS];   /* indice de temporal, o -1 si no es temporal */
    int usados[MAX_TEMPORALES] = {0};
    int tope = 0;
    int i;

    for (i = 0; i < n; i++)
    {
        int simbolo;

        if (pf[i].tipo == TOK_NUM)
        {
            simbolo = tabla_obtener(&c->tabla, SIM_CONSTANTE, pf[i].valor);
        }
        else if (pf[i].tipo == TOK_VAR)
        {
            simbolo = tabla_obtener(&c->tabla, SIM_VARIABLE, pf[i].valor);
        }
        else
        {
            int simA, simB, tempA, tempB, destino;

            tope--;
            simB = pilaSim[tope];
            tempB = pilaTemp[tope];
            tope--;
            simA = pilaSim[tope];
            tempA = pilaTemp[tope];

            emitir(c, SML_LOAD, simA, "LOAD");
            emitir(c, opcode_operador(pf[i].valor), simB, mnemonico_operador(pf[i].valor));

            if (i == n - 1)
            {
                emitir(c, SML_STORE, variable, "STORE");
            }
            else
            {
                int t;

                /* el resultado reutiliza el temporal de un operando si lo hay */
                if (tempA >= 0)
                {
                    t = tempA;
                    if (tempB >= 0)
                    {
                        usados[tempB] = 0;
                    }
                }
                else if (tempB >= 0)
                {
                    t = tempB;
                }
                else
                {
                    t = temporal_libre(usados);
                    if (t < 0)
                    {
                        error_renglon(c, "la expresion necesita demasiados temporales");
                        return;
                    }
                }
                destino = tabla_obtener(&c->tabla, SIM_TEMPORAL, t);
                if (destino < 0)
                {
                    error_renglon(c, "tabla de simbolos llena");
                    return;
                }
                emitir(c, SML_STORE, destino, "STORE");
                pilaSim[tope] = destino;
                pilaTemp[tope] = t;
                tope++;
            }
            continue;
        }

        if (simbolo < 0)
        {
            error_renglon(c, "tabla de simbolos llena");
            return;
        }
        pilaSim[tope] = simbolo;
        pilaTemp[tope] = -1;
        tope++;
    }

    if (n == 1)
    {
        /* let x = y   o   let x = 5 */
        emitir(c, SML_LOAD, pilaSim[0], "LOAD");
        emitir(c, SML_STORE, variable, "STORE");
    }
}

/* if a <op> b goto destino. La comparacion se hace restando: el signo y el
   cero del acumulador deciden el salto. */
static void compilar_if(Compilador *c, int a, int b, const char *op, int destino)
{
    if (strcmp(op, "==") == 0)
    {
        emitir(c, SML_LOAD, a, "LOAD");
        emitir(c, SML_SUBTRACT, b, "SUBTRACT");
        emitir_salto(c, SML_BRANCHZERO, destino, "BRANCHZERO");
    }
    else if (strcmp(op, "!=") == 0)
    {
        emitir(c, SML_LOAD, a, "LOAD");
        emitir(c, SML_SUBTRACT, b, "SUBTRACT");
        /* si a - b es cero, se salta el BRANCH siguiente */
        emitir_directo(c, SML_BRANCHZERO, c->cantidadCodigo + 2, "BRANCHZERO");
        emitir_salto(c, SML_BRANCH, destino, "BRANCH");
    }
    else if (strcmp(op, "<") == 0)
    {
        emitir(c, SML_LOAD, a, "LOAD");
        emitir(c, SML_SUBTRACT, b, "SUBTRACT");
        emitir_salto(c, SML_BRANCHNEG, destino, "BRANCHNEG");
    }
    else if (strcmp(op, ">") == 0)
    {
        emitir(c, SML_LOAD, b, "LOAD");
        emitir(c, SML_SUBTRACT, a, "SUBTRACT");
        emitir_salto(c, SML_BRANCHNEG, destino, "BRANCHNEG");
    }
    else if (strcmp(op, "<=") == 0)
    {
        emitir(c, SML_LOAD, a, "LOAD");
        emitir(c, SML_SUBTRACT, b, "SUBTRACT");
        emitir_salto(c, SML_BRANCHNEG, destino, "BRANCHNEG");
        emitir_salto(c, SML_BRANCHZERO, destino, "BRANCHZERO");
    }
    else   /* >= */
    {
        emitir(c, SML_LOAD, b, "LOAD");
        emitir(c, SML_SUBTRACT, a, "SUBTRACT");
        emitir_salto(c, SML_BRANCHNEG, destino, "BRANCHNEG");
        emitir_salto(c, SML_BRANCHZERO, destino, "BRANCHZERO");
    }
}

/* ------------------------------------------------------------------ */
/* Primera pasada                                                      */
/* ------------------------------------------------------------------ */

static void comando_let(Compilador *c, const char *p)
{
    int variable;
    Token pf[EXPR_MAX_TOKENS];
    int cantidad = 0;
    int codigo;

    if (!leer_variable(c, &p, &variable))
    {
        return;
    }
    saltar_espacios(&p);
    if (*p != '=')
    {
        error_renglon(c, "se esperaba '=' despues de la variable");
        return;
    }
    p++;

    codigo = expr_infija_a_postfija(p, pf, &cantidad);
    if (codigo != EXPR_OK)
    {
        error_renglon(c, "expresion no valida: %s", expr_mensaje(codigo));
        return;
    }
    compilar_let(c, variable, pf, cantidad);
}

static void comando_if(Compilador *c, const char *p)
{
    int a, b, destino;
    char op[3];
    char palabra[16];

    if (!leer_operando(c, &p, &a) || !leer_comparador(c, &p, op) ||
        !leer_operando(c, &p, &b))
    {
        return;
    }
    leer_palabra(&p, palabra, sizeof palabra);
    if (strcmp(palabra, "goto") != 0)
    {
        error_renglon(c, "se esperaba 'goto' despues de la condicion");
        return;
    }
    if (!leer_linea_destino(c, &p, &destino))
    {
        return;
    }
    if (!fin_de_linea(p))
    {
        error_renglon(c, "texto inesperado al final de la linea");
        return;
    }
    compilar_if(c, a, b, op, destino);
}

static const char *comandos[] = {"rem", "input", "let", "print", "goto", "if", "end"};

static int es_comando(const char *palabra)
{
    size_t i;

    for (i = 0; i < sizeof comandos / sizeof comandos[0]; i++)
    {
        if (strcmp(palabra, comandos[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

static void procesar_linea(Compilador *c, const char *linea, int *ultimaLinea)
{
    const char *p = linea;
    char palabra[16];
    char minusculas[16];
    char *fin;
    long numero;
    int simbolo;
    int i;

    saltar_espacios(&p);
    if (*p == '\0')
    {
        return;   /* renglon en blanco */
    }

    /* numero de linea */
    if (!isdigit((unsigned char)*p))
    {
        error_renglon(c, "falta el numero de linea");
        return;
    }
    numero = strtol(p, &fin, 10);
    p = fin;
    if (numero < 1 || numero > LINEA_MAX)
    {
        error_renglon(c, "numero de linea fuera de rango (1 a %d)", LINEA_MAX);
        return;
    }
    if (numero <= *ultimaLinea)
    {
        error_renglon(c, "los numeros de linea deben ir en orden ascendente");
        return;
    }
    *ultimaLinea = (int)numero;
    c->lineaActual = (int)numero;

    if (c->finVisto)
    {
        error_renglon(c, "no puede haber lineas despues de end");
        return;
    }

    /* Registra la linea en la tabla de simbolos con la direccion de su
       primera instruccion. Si un goto anterior la menciono, esa referencia
       queda resuelta en la segunda pasada. */
    simbolo = tabla_obtener(&c->tabla, SIM_LINEA, (int)numero);
    if (simbolo < 0)
    {
        error_renglon(c, "tabla de simbolos llena");
        return;
    }
    c->tabla.items[simbolo].ubicacion = c->cantidadCodigo;

    /* comando */
    leer_palabra(&p, palabra, sizeof palabra);
    if (palabra[0] == '\0')
    {
        error_renglon(c, "falta el comando");
        return;
    }
    for (i = 0; palabra[i] != '\0'; i++)
    {
        minusculas[i] = (char)tolower((unsigned char)palabra[i]);
    }
    minusculas[i] = '\0';

    if (!es_comando(palabra))
    {
        if (es_comando(minusculas))
        {
            error_renglon(c, "los comandos deben escribirse en minusculas ('%s')", palabra);
        }
        else
        {
            error_renglon(c, "comando desconocido '%s'", palabra);
        }
        return;
    }

    if (strcmp(palabra, "rem") == 0)
    {
        return;   /* el comentario puede llevar cualquier texto */
    }
    else if (strcmp(palabra, "input") == 0)
    {
        if (leer_variable(c, &p, &simbolo))
        {
            if (!fin_de_linea(p))
            {
                error_renglon(c, "texto inesperado al final de la linea");
            }
            else
            {
                emitir(c, SML_READ, simbolo, "READ");
            }
        }
    }
    else if (strcmp(palabra, "print") == 0)
    {
        if (leer_operando(c, &p, &simbolo))
        {
            if (!fin_de_linea(p))
            {
                error_renglon(c, "texto inesperado al final de la linea");
            }
            else
            {
                emitir(c, SML_WRITE, simbolo, "WRITE");
            }
        }
    }
    else if (strcmp(palabra, "let") == 0)
    {
        comando_let(c, p);
    }
    else if (strcmp(palabra, "goto") == 0)
    {
        int destino;

        if (leer_linea_destino(c, &p, &destino))
        {
            if (!fin_de_linea(p))
            {
                error_renglon(c, "texto inesperado al final de la linea");
            }
            else
            {
                emitir_salto(c, SML_BRANCH, destino, "BRANCH");
            }
        }
    }
    else if (strcmp(palabra, "if") == 0)
    {
        comando_if(c, p);
    }
    else   /* end */
    {
        if (!fin_de_linea(p))
        {
            error_renglon(c, "texto inesperado al final de la linea");
        }
        else
        {
            emitir_simple(c, SML_HALT, "HALT");
            c->finVisto = 1;
        }
    }
}

static void primera_pasada(FILE *fuente, Compilador *c)
{
    char linea[LONG_LINEA];
    int ultimaLinea = 0;

    while (fgets(linea, sizeof linea, fuente) != NULL)
    {
        size_t largo = strlen(linea);

        c->renglon++;
        c->lineaActual = 0;

        if (largo > 0 && linea[largo - 1] != '\n' && !feof(fuente))
        {
            int ch;

            error_renglon(c, "renglon demasiado largo");
            while ((ch = fgetc(fuente)) != '\n' && ch != EOF)
            {
            }
            continue;
        }

        procesar_linea(c, linea, &ultimaLinea);
    }

    if (!c->finVisto)
    {
        error_global(c, "falta el comando end al final del programa");
    }
}

/* ------------------------------------------------------------------ */
/* Segunda pasada                                                      */
/* ------------------------------------------------------------------ */

static void segunda_pasada(Compilador *c)
{
    int i;
    int siguiente;
    int ultimoReportado = -1;

    /* 1. Referencias hacia adelante: toda linea mencionada debe existir */
    for (i = 0; i < c->cantidadCodigo; i++)
    {
        const Instruccion *in = &c->codigo[i];

        if (in->ref >= 0 && c->tabla.items[in->ref].tipo == SIM_LINEA &&
            c->tabla.items[in->ref].ubicacion < 0 && in->lineaFuente != ultimoReportado)
        {
            printf("Error en la linea Simple %d: la linea destino %d no existe\n",
                   in->lineaFuente, c->tabla.items[in->ref].valor);
            c->errores++;
            ultimoReportado = in->lineaFuente;
        }
    }
    if (c->errores > 0)
    {
        return;
    }

    /* 2. Variables, constantes y temporales van a continuacion del codigo */
    siguiente = c->cantidadCodigo;
    for (i = 0; i < c->tabla.cantidad; i++)
    {
        if (c->tabla.items[i].tipo != SIM_LINEA)
        {
            c->tabla.items[i].ubicacion = siguiente++;
        }
    }
    c->totalPalabras = siguiente;
    if (siguiente > SML_CAPACIDAD)
    {
        error_global(c, "el programa necesita %d palabras de memoria y Simpletron tiene %d",
                     siguiente, SML_CAPACIDAD);
        return;
    }

    /* 3. Completa las instrucciones con la direccion de su operando */
    for (i = 0; i < c->cantidadCodigo; i++)
    {
        Instruccion *in = &c->codigo[i];

        if (in->ref >= 0)
        {
            in->palabra += c->tabla.items[in->ref].ubicacion;
            in->ref = -1;
        }
        c->memoria[i] = in->palabra;
    }

    /* 4. Valores iniciales de los datos: las constantes con su valor, el
          resto en cero */
    for (i = 0; i < c->tabla.cantidad; i++)
    {
        const Simbolo *s = &c->tabla.items[i];

        if (s->tipo == SIM_CONSTANTE)
        {
            c->memoria[s->ubicacion] = s->valor;
        }
    }
}

int compilar(FILE *fuente, Compilador *c)
{
    memset(c, 0, sizeof *c);
    tabla_iniciar(&c->tabla);

    primera_pasada(fuente, c);
    segunda_pasada(c);

    return c->errores > 0 ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Archivo SML                                                         */
/* ------------------------------------------------------------------ */

static void escribir_palabra(FILE *f, int valor)
{
    if (valor >= 0)
    {
        fprintf(f, "%04d", valor);
    }
    else
    {
        fprintf(f, "-%04d", -valor);
    }
}

void escribir_sml(const Compilador *c, FILE *salida, const char *nombreFuente)
{
    int i, j;

    fprintf(salida, "; Programa SML generado por el compilador de Simple\n");
    fprintf(salida, "; Fuente: %s\n", nombreFuente);
    if (c->totalPalabras > c->cantidadCodigo)
    {
        fprintf(salida, "; Area de programa: 00-%02d | Area de datos: %02d-%02d\n",
                c->cantidadCodigo - 1, c->cantidadCodigo, c->totalPalabras - 1);
    }
    else
    {
        fprintf(salida, "; Area de programa: 00-%02d | Sin area de datos\n",
                c->cantidadCodigo - 1);
    }

    for (i = 0; i < c->cantidadCodigo; i++)
    {
        escribir_palabra(salida, c->memoria[i]);
        fprintf(salida, " ; %02d %-22s (linea %d)\n", i, c->codigo[i].nota,
                c->codigo[i].lineaFuente);
    }

    for (i = c->cantidadCodigo; i < c->totalPalabras; i++)
    {
        char nombre[24] = "";

        for (j = 0; j < c->tabla.cantidad; j++)
        {
            if (c->tabla.items[j].tipo != SIM_LINEA && c->tabla.items[j].ubicacion == i)
            {
                tabla_nombre_simbolo(&c->tabla.items[j], nombre, sizeof nombre);
                break;
            }
        }
        escribir_palabra(salida, c->memoria[i]);
        fprintf(salida, " ; %02d dato %s\n", i, nombre);
    }

    fprintf(salida, "9999 ; fin del programa\n");
}
