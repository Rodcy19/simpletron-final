#include <stdio.h>
#include <string.h>
#include "compilador.h"

int main(int argc, char *argv[])
{
    int mostrarTabla = 0;
    int i = 1;
    const char *entrada;
    char salida[512];
    FILE *fuente, *destino;
    static Compilador c;

    if (i < argc && strcmp(argv[i], "-t") == 0)
    {
        mostrarTabla = 1;
        i++;
    }
    if (i >= argc)
    {
        printf("Uso: %s [-t] entrada.simple [salida.sml]\n", argv[0]);
        return 1;
    }
    entrada = argv[i++];

    if (i < argc)
    {
        snprintf(salida, sizeof salida, "%s", argv[i]);
    }
    else
    {
        char *punto;

        snprintf(salida, sizeof salida, "%s", entrada);
        punto = strrchr(salida, '.');
        if (punto != NULL && strchr(punto, '/') == NULL)
        {
            *punto = '\0';
        }
        strncat(salida, ".sml", sizeof salida - strlen(salida) - 1);
    }

    fuente = fopen(entrada, "r");
    if (fuente == NULL)
    {
        printf("Error: no se pudo abrir el archivo '%s'\n", entrada);
        return 1;
    }

    if (compilar(fuente, &c) != 0)
    {
        fclose(fuente);
        printf("Compilacion fallida: %d error(es)\n", c.errores);
        return 1;
    }
    fclose(fuente);

    destino = fopen(salida, "w");
    if (destino == NULL)
    {
        printf("Error: no se pudo crear el archivo '%s'\n", salida);
        return 1;
    }
    escribir_sml(&c, destino, entrada);
    fclose(destino);

    printf("Compilacion exitosa: %d instrucciones, %d datos (%d de %d palabras)\n",
           c.cantidadCodigo, c.totalPalabras - c.cantidadCodigo,
           c.totalPalabras, SML_CAPACIDAD);
    printf("Archivo generado: %s\n", salida);
    if (mostrarTabla)
    {
        tabla_imprimir(&c.tabla, stdout);
    }
    return 0;
}
