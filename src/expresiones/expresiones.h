/*
 * expresiones.h - Evaluacion de expresiones aritmeticas
 *
 * Componentes:
 *   1. Validacion de simbolos de agrupacion () [] {} y de su profundidad.
 *   2. Conversion de notacion infija a postfija (algoritmo de patio de
 *      maniobras) respetando precedencia y asociatividad.
 *   3. Evaluacion de expresiones postfijas.
 *
 * Elementos que reconoce una expresion:
 *   - Constantes enteras de uno o mas digitos (0 a 9999). Una constante puede
 *     llevar signo negativo ("-5") cuando aparece donde se espera un operando:
 *     al inicio, despues de '(' '[' '{' o despues de un operador.
 *   - Variables de una sola letra minuscula (a-z).
 *   - Operadores: ^ (potencia, asocia por la derecha), * y / , + y -
 *     (los demas asocian por la izquierda).
 *   - Agrupacion con (), [] y {}; las tres se tratan igual.
 *
 * Este modulo es la base del compilador de Simple: las expresiones de los
 * comandos 'let' se convertiran a postfija con expr_infija_a_postfija().
 */
#ifndef EXPRESIONES_H
#define EXPRESIONES_H

#include <stddef.h>

#define EXPR_MAX_TOKENS       128   /* tokens maximos de una expresion */
#define EXPR_PROFUNDIDAD_MAX  5     /* anidamiento maximo de agrupaciones */
#define EXPR_VALOR_MIN       -9999  /* rango de una palabra de Simpletron */
#define EXPR_VALOR_MAX        9999

/* Codigos de resultado */
#define EXPR_OK                0
#define EXPR_DESBALANCEADA     1
#define EXPR_PROFUNDIDAD       2
#define EXPR_SINTAXIS          3
#define EXPR_CARACTER          4
#define EXPR_RANGO             5
#define EXPR_DIV_CERO          6
#define EXPR_EXP_NEGATIVO      7
#define EXPR_VACIA             8
#define EXPR_LARGA             9
#define EXPR_SIN_VALOR        10

typedef enum { TOK_NUM, TOK_VAR, TOK_OP } TipoToken;

typedef struct {
    TipoToken tipo;
    int valor;   /* TOK_NUM: el numero | TOK_VAR: la letra | TOK_OP: el operador */
} Token;

/* Revisa que () [] {} esten balanceados y no se aniden mas que
   EXPR_PROFUNDIDAD_MAX. Si profundidad no es NULL, recibe el anidamiento
   maximo alcanzado cuando el resultado es EXPR_OK. */
int expr_validar_agrupacion(const char *texto, int *profundidad);

/* Convierte la expresion infija a postfija. En salida deja los tokens y en
   cantidad cuantos son. Valida la agrupacion y la sintaxis. */
int expr_infija_a_postfija(const char *texto, Token salida[], int *cantidad);

/* Evalua una expresion postfija. variables (a-z en las posiciones 0-25) puede
   ser NULL si la expresion no contiene variables. */
int expr_evaluar_postfija(const Token postfija[], int cantidad,
                          const int variables[26], int *resultado);

/* Escribe la expresion postfija como texto, con los tokens separados por
   espacios (por ejemplo "2 3 4 * +"). */
int expr_postfija_a_texto(const Token postfija[], int cantidad,
                          char *destino, size_t tam);

/* Descripcion en texto de un codigo de resultado. */
const char *expr_mensaje(int codigo);

#endif
