//Programa de prueba del modulo de expresiones
#include <stdio.h>
#include <string.h>
#include "expresiones.h"

int main(void)
{
    char linea[256];

    while (fgets(linea, sizeof linea, stdin) != NULL)
    {
        Token postfija[EXPR_MAX_TOKENS];
        char texto[1024];
        size_t largo = strlen(linea);
        int cantidad = 0;
        int profundidad = 0;
        int codigo;
        int resultado = 0;
        int i;
        int hayVariables = 0;

        while (largo > 0 && (linea[largo - 1] == '\n' || linea[largo - 1] == '\r'))
        {
            linea[--largo] = '\0';
        }

        printf("Expresion: %s\n", linea);

        codigo = expr_validar_agrupacion(linea, &profundidad);
        if (codigo != EXPR_OK)
        {
            printf("Error: %s\n\n", expr_mensaje(codigo));
            continue;
        }
        printf("Agrupacion: correcta (profundidad %d)\n", profundidad);

        codigo = expr_infija_a_postfija(linea, postfija, &cantidad);
        if (codigo != EXPR_OK)
        {
            printf("Error: %s\n\n", expr_mensaje(codigo));
            continue;
        }

        expr_postfija_a_texto(postfija, cantidad, texto, sizeof texto);
        printf("Postfija:  %s\n", texto);

        for (i = 0; i < cantidad; i++)
        {
            if (postfija[i].tipo == TOK_VAR)
            {
                hayVariables = 1;
            }
        }

        if (hayVariables)
        {
            printf("Resultado: no se evalua (contiene variables)\n\n");
            continue;
        }

        codigo = expr_evaluar_postfija(postfija, cantidad, NULL, &resultado);
        if (codigo != EXPR_OK)
        {
            printf("Error: %s\n\n", expr_mensaje(codigo));
        }
        else
        {
            printf("Resultado: %d\n\n", resultado);
        }
    }

    return 0;
}
