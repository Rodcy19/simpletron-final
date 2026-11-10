#include <stdio.h>
#include "simbolos.h"

void tabla_iniciar(TablaSimbolos *t)
{
    t->cantidad = 0;
}

int tabla_buscar(const TablaSimbolos *t, TipoSimbolo tipo, int valor)
{
    int i;

    for (i = 0; i < t->cantidad; i++)
    {
        if (t->items[i].tipo == tipo && t->items[i].valor == valor)
        {
            return i;
        }
    }
    return -1;
}

int tabla_obtener(TablaSimbolos *t, TipoSimbolo tipo, int valor)
{
    int i = tabla_buscar(t, tipo, valor);

    if (i >= 0)
    {
        return i;
    }
    if (t->cantidad >= SIM_MAX)
    {
        return -1;
    }

    t->items[t->cantidad].tipo = tipo;
    t->items[t->cantidad].valor = valor;
    t->items[t->cantidad].ubicacion = -1;
    return t->cantidad++;
}

void tabla_nombre_simbolo(const Simbolo *s, char *destino, size_t tam)
{
    switch (s->tipo)
    {
        case SIM_LINEA:
            snprintf(destino, tam, "linea %d", s->valor);
            break;
        case SIM_VARIABLE:
            snprintf(destino, tam, "%c", s->valor);
            break;
        case SIM_CONSTANTE:
            snprintf(destino, tam, "const %d", s->valor);
            break;
        default:
            snprintf(destino, tam, "t%d", s->valor);
            break;
    }
}

static const char *nombre_tipo(TipoSimbolo tipo)
{
    switch (tipo)
    {
        case SIM_LINEA:
            return "linea";
        case SIM_VARIABLE:
            return "variable";
        case SIM_CONSTANTE:
            return "constante";
        default:
            return "temporal";
    }
}

void tabla_imprimir(const TablaSimbolos *t, FILE *f)
{
    int i;

    fprintf(f, "Tabla de simbolos:\n");
    fprintf(f, "%-10s %-8s %s\n", "Tipo", "Simbolo", "Ubicacion");

    for (i = 0; i < t->cantidad; i++)
    {
        const Simbolo *s = &t->items[i];
        char simbolo[16];

        switch (s->tipo)
        {
            case SIM_VARIABLE:
                snprintf(simbolo, sizeof simbolo, "%c", s->valor);
                break;
            case SIM_TEMPORAL:
                snprintf(simbolo, sizeof simbolo, "t%d", s->valor);
                break;
            default:
                snprintf(simbolo, sizeof simbolo, "%d", s->valor);
                break;
        }

        if (s->ubicacion < 0)
        {
            fprintf(f, "%-10s %-8s %s\n", nombre_tipo(s->tipo), simbolo, "--");
        }
        else
        {
            fprintf(f, "%-10s %-8s %02d\n", nombre_tipo(s->tipo), simbolo, s->ubicacion);
        }
    }
}
