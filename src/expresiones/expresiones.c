//Validacion, conversion infija a postfija y evaluacion
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expresiones.h"

static int esApertura(char c)
{
    return c == '(' || c == '[' || c == '{';
}

static int esCierre(char c)
{
    return c == ')' || c == ']' || c == '}';
}

static char cierreDe(char apertura)
{
    if (apertura == '(')
    {
        return ')';
    }
    if (apertura == '[')
    {
        return ']';
    }
    return '}';
}

static int esOperador(char c)
{
    return c != '\0' && strchr("+-*/^", c) != NULL;
}

/* Mayor numero = se calcula primero */
static int precedencia(char op)
{
    switch (op)
    {
        case '^':
            return 3;
        case '*':
        case '/':
            return 2;
        case '+':
        case '-':
            return 1;
        default:
            return 0;
    }
}

const char *expr_mensaje(int codigo)
{
    switch (codigo)
    {
        case EXPR_OK:
            return "correcto";
        case EXPR_DESBALANCEADA:
            return "simbolos de agrupacion sin balancear";
        case EXPR_PROFUNDIDAD:
            return "anidamiento demasiado profundo";
        case EXPR_SINTAXIS:
            return "expresion mal formada";
        case EXPR_CARACTER:
            return "caracter no valido";
        case EXPR_RANGO:
            return "valor fuera del rango (-9999 a 9999)";
        case EXPR_DIV_CERO:
            return "division entre cero";
        case EXPR_EXP_NEGATIVO:
            return "exponente negativo no valido";
        case EXPR_VACIA:
            return "expresion vacia";
        case EXPR_LARGA:
            return "expresion demasiado larga";
        case EXPR_SIN_VALOR:
            return "variable sin valor";
        default:
            return "error desconocido";
    }
}

int expr_validar_agrupacion(const char *texto, int *profundidad)
{
    char pila[EXPR_PROFUNDIDAD_MAX];
    int tope = 0;
    int maxProf = 0;
    const char *p;

    if (profundidad != NULL)
    {
        *profundidad = 0;
    }

    for (p = texto; *p != '\0'; p++)
    {
        if (esApertura(*p))
        {
            if (tope >= EXPR_PROFUNDIDAD_MAX)
            {
                return EXPR_PROFUNDIDAD;
            }
            pila[tope++] = *p;
            if (tope > maxProf)
            {
                maxProf = tope;
            }
        }
        else if (esCierre(*p))
        {
            if (tope == 0 || cierreDe(pila[tope - 1]) != *p)
            {
                return EXPR_DESBALANCEADA;
            }
            tope--;
        }
    }

    if (tope != 0)
    {
        return EXPR_DESBALANCEADA;
    }

    if (profundidad != NULL)
    {
        *profundidad = maxProf;
    }
    return EXPR_OK;
}

/* Agrega un token a la salida; devuelve EXPR_LARGA si ya no cabe */
static int emitir(Token salida[], int *n, TipoToken tipo, int valor)
{
    if (*n >= EXPR_MAX_TOKENS)
    {
        return EXPR_LARGA;
    }
    salida[*n].tipo = tipo;
    salida[*n].valor = valor;
    (*n)++;
    return EXPR_OK;
}

int expr_infija_a_postfija(const char *texto, Token salida[], int *cantidad)
{
    char pila[EXPR_MAX_TOKENS];   /* operadores y simbolos de apertura */
    int tope = 0;
    int n = 0;
    int esperaOperando = 1;
    int hayTokens = 0;
    const char *p = texto;
    int codigo;

    *cantidad = 0;

    codigo = expr_validar_agrupacion(texto, NULL);
    if (codigo != EXPR_OK)
    {
        return codigo;
    }

    while (*p != '\0')
    {
        char c = *p;

        if (isspace((unsigned char)c))
        {
            p++;
            continue;
        }
        hayTokens = 1;

        if (isdigit((unsigned char)c) ||
            (c == '-' && esperaOperando && isdigit((unsigned char)p[1])))
        {
            char *fin;
            long v;

            if (!esperaOperando)
            {
                return EXPR_SINTAXIS;
            }
            v = strtol(p, &fin, 10);
            if (v < EXPR_VALOR_MIN || v > EXPR_VALOR_MAX)
            {
                return EXPR_RANGO;
            }
            codigo = emitir(salida, &n, TOK_NUM, (int)v);
            if (codigo != EXPR_OK)
            {
                return codigo;
            }
            esperaOperando = 0;
            p = fin;
        }
        else if (c >= 'a' && c <= 'z')
        {
            if (!esperaOperando)
            {
                return EXPR_SINTAXIS;
            }
            codigo = emitir(salida, &n, TOK_VAR, c);
            if (codigo != EXPR_OK)
            {
                return codigo;
            }
            esperaOperando = 0;
            p++;
        }
        else if (esApertura(c))
        {
            if (!esperaOperando)
            {
                return EXPR_SINTAXIS;
            }
            if (tope >= EXPR_MAX_TOKENS)
            {
                return EXPR_LARGA;
            }
            pila[tope++] = c;
            p++;
        }
        else if (esCierre(c))
        {
            if (esperaOperando)
            {
                return EXPR_SINTAXIS;
            }
            while (tope > 0 && !esApertura(pila[tope - 1]))
            {
                codigo = emitir(salida, &n, TOK_OP, pila[--tope]);
                if (codigo != EXPR_OK)
                {
                    return codigo;
                }
            }
            if (tope > 0)
            {
                tope--;   /* descarta el simbolo de apertura */
            }
            p++;
        }
        else if (esOperador(c))
        {
            if (esperaOperando)
            {
                return EXPR_SINTAXIS;
            }
            /* Saca los operadores que se calculan antes que c. Con igual
               precedencia se sacan solo si c asocia por la izquierda
               (todos menos la potencia). */
            while (tope > 0 && esOperador(pila[tope - 1]) &&
                   (precedencia(pila[tope - 1]) > precedencia(c) ||
                    (precedencia(pila[tope - 1]) == precedencia(c) && c != '^')))
            {
                codigo = emitir(salida, &n, TOK_OP, pila[--tope]);
                if (codigo != EXPR_OK)
                {
                    return codigo;
                }
            }
            if (tope >= EXPR_MAX_TOKENS)
            {
                return EXPR_LARGA;
            }
            pila[tope++] = c;
            esperaOperando = 1;
            p++;
        }
        else
        {
            return EXPR_CARACTER;
        }
    }

    if (!hayTokens)
    {
        return EXPR_VACIA;
    }
    if (esperaOperando)
    {
        return EXPR_SINTAXIS;   /* termina en operador */
    }

    while (tope > 0)
    {
        codigo = emitir(salida, &n, TOK_OP, pila[--tope]);
        if (codigo != EXPR_OK)
        {
            return codigo;
        }
    }

    *cantidad = n;
    return EXPR_OK;
}

int expr_evaluar_postfija(const Token postfija[], int cantidad,
                          const int variables[26], int *resultado)
{
    long pila[EXPR_MAX_TOKENS];
    int tope = 0;
    int i;

    for (i = 0; i < cantidad; i++)
    {
        const Token *t = &postfija[i];

        if (t->tipo == TOK_NUM)
        {
            pila[tope++] = t->valor;
        }
        else if (t->tipo == TOK_VAR)
        {
            if (variables == NULL)
            {
                return EXPR_SIN_VALOR;
            }
            pila[tope++] = variables[t->valor - 'a'];
        }
        else
        {
            long a, b, r = 0;

            if (tope < 2)
            {
                return EXPR_SINTAXIS;
            }
            b = pila[--tope];
            a = pila[--tope];

            switch (t->valor)
            {
                case '+':
                    r = a + b;
                    break;
                case '-':
                    r = a - b;
                    break;
                case '*':
                    r = a * b;
                    break;
                case '/':
                    if (b == 0)
                    {
                        return EXPR_DIV_CERO;
                    }
                    r = a / b;
                    break;
                case '^':
                {
                    long k;

                    if (b < 0)
                    {
                        return EXPR_EXP_NEGATIVO;
                    }
                    r = 1;
                    for (k = 0; k < b; k++)
                    {
                        r = r * a;
                        if (r > EXPR_VALOR_MAX || r < EXPR_VALOR_MIN)
                        {
                            return EXPR_RANGO;
                        }
                    }
                    break;
                }
                default:
                    return EXPR_SINTAXIS;
            }

            if (r > EXPR_VALOR_MAX || r < EXPR_VALOR_MIN)
            {
                return EXPR_RANGO;
            }
            pila[tope++] = r;
        }
    }

    if (tope != 1)
    {
        return EXPR_SINTAXIS;
    }

    *resultado = (int)pila[0];
    return EXPR_OK;
}

int expr_postfija_a_texto(const Token postfija[], int cantidad,
                          char *destino, size_t tam)
{
    size_t usado = 0;
    int i;

    if (tam == 0)
    {
        return EXPR_LARGA;
    }
    destino[0] = '\0';

    for (i = 0; i < cantidad; i++)
    {
        char pieza[16];
        int largo;

        if (postfija[i].tipo == TOK_NUM)
        {
            largo = snprintf(pieza, sizeof pieza, "%d", postfija[i].valor);
        }
        else
        {
            largo = snprintf(pieza, sizeof pieza, "%c", postfija[i].valor);
        }

        if (usado + (size_t)largo + 2 > tam)
        {
            return EXPR_LARGA;
        }
        if (i > 0)
        {
            destino[usado++] = ' ';
        }
        memcpy(destino + usado, pieza, (size_t)largo);
        usado += (size_t)largo;
        destino[usado] = '\0';
    }

    return EXPR_OK;
}
