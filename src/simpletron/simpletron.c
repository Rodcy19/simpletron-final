/*
 * Extensiones (rama feature/extensiones):
 *   12 NEWLINE   imprime un salto de linea (el operando se ignora)
 *   13 WRITESTR  imprime la cadena guardada desde memoria[operando]
 *   14 READSTR   lee una linea del teclado y la guarda desde memoria[operando]
 *                Formato de cadena: memoria[d] = longitud n y en las n
 *                posiciones siguientes un caracter (codigo 0-255) por palabra.
 *   34 MODULE    acumulador = acumulador % memoria[operando]
 *   35 POWER     acumulador = acumulador ^ memoria[operando]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CAPACIDAD     100
#define PALABRA_MIN   -9999
#define PALABRA_MAX    9999
#define CENTINELA      9999   /* termina la carga del programa */

/* --- Codigos de operacion SML --- */
#define OP_READ         10
#define OP_WRITE        11
#define OP_NEWLINE      12   /* extension */
#define OP_WRITESTR     13   /* extension */
#define OP_READSTR      14   /* extension */
#define OP_LOAD         20
#define OP_STORE        21
#define OP_ADD          30
#define OP_SUBTRACT     31
#define OP_DIVIDE       32
#define OP_MULTIPLY     33
#define OP_MODULE       34   /* extension */
#define OP_POWER        35   /* extension */
#define OP_BRANCH       40
#define OP_BRANCHNEG    41
#define OP_BRANCHZERO   42
#define OP_HALT         43

/* Resultados de leerPalabra */
#define LEER_EOF       -1
#define LEER_INVALIDA   0
#define LEER_OK         1
#define LEER_VACIA      2

/* Registros de la maquina */
typedef struct {
    int acc;   /* accumulator */
    int pc;    /* instructionCounter */
    int ir;    /* instructionRegister */
    int op;    /* operationCode */
    int dir;   /* operand */
} Registros;

static int RAM[CAPACIDAD];

static void mostrarBienvenida(void);
static int  leerPalabra(FILE *f, long *valor);
static int  cargarPrograma(int mem[]);
static int  cargarDesdeArchivo(int mem[], const char *ruta);
static int  verificarAcumulador(const Registros *reg);
static int  correrCiclo(int mem[], Registros *reg);
static void volcarMemoria(const int mem[], const Registros *reg);

int main(int argc, char *argv[])
{
    Registros reg = {0, 0, 0, 0, 0};
    int huboError;

    if (argc > 2)
    {
        printf("Uso: %s [programa.sml]\n", argv[0]);
        return 1;
    }

    if (argc == 2)
    {
        int palabras = cargarDesdeArchivo(RAM, argv[1]);

        if (palabras < 0)
        {
            return 1;
        }
        printf("***Programa cargado desde '%s' (%d palabras)***\n",
               argv[1], palabras);
    }
    else
    {
        mostrarBienvenida();
        cargarPrograma(RAM);
        printf("\n***Se termino de cargar el programa***\n");
    }

    printf("***Comienza la ejecucion del programa***\n\n");

    huboError = correrCiclo(RAM, &reg);

    if (huboError)
    {
        printf("***La ejecucion de Simpletron termino anormalmente***\n");
    }

    volcarMemoria(RAM, &reg);

    return huboError ? 1 : 0;
}

static void mostrarBienvenida(void)
{
    printf("***Bienvenido a SIMPLETRON***\n");
    printf("***Introduzca su programa una instruccion***\n");
    printf("***(o palabra de datos) a la vez. Yo indicare***\n");
    printf("***el numero de posicion y una interrogacion (?).***\n");
    printf("***Usted tecleara una palabra para esa posicion.***\n");
    printf("***Capture 9999 para terminar de introducir***\n");
    printf("***su programa***\n\n");
}

/*
 * Lee una linea del flujo f y extrae un entero.
 * Devuelve LEER_OK, LEER_VACIA (linea en blanco o comentario),
 * LEER_INVALIDA (no es un entero) o LEER_EOF (fin de la entrada).
 */
static int leerPalabra(FILE *f, long *valor)
{
    char linea[128];
    char *p;
    char *fin;

    if (fgets(linea, sizeof linea, f) == NULL)
    {
        return LEER_EOF;
    }

    /* si la linea fue mas larga que el buffer, descartar el resto */
    if (strchr(linea, '\n') == NULL && !feof(f))
    {
        int c;
        while ((c = fgetc(f)) != '\n' && c != EOF)
        {
        }
    }

    p = linea;
    while (isspace((unsigned char)*p))
    {
        p++;
    }

    if (*p == '\0' || *p == ';')
    {
        return LEER_VACIA;
    }

    *valor = strtol(p, &fin, 10);
    if (fin == p)
    {
        return LEER_INVALIDA;
    }

    while (isspace((unsigned char)*fin))
    {
        fin++;
    }
    if (*fin != '\0' && *fin != ';')
    {
        return LEER_INVALIDA;
    }

    return LEER_OK;
}

/* Fase de carga: solicita instrucciones/datos y las coloca en memoria.
   Devuelve cuantas palabras se cargaron. */
static int cargarPrograma(int mem[])
{
    int direccion = 0;
    int mostrarPrompt = 1;
    long dato = 0;
    int r;

    while (direccion < CAPACIDAD)
    {
        if (mostrarPrompt)
        {
            printf("%02d ? ", direccion);
            mostrarPrompt = 0;
        }

        r = leerPalabra(stdin, &dato);

        if (r == LEER_EOF)
        {
            break;
        }
        if (r == LEER_VACIA)
        {
            continue;
        }

        mostrarPrompt = 1;

        if (r == LEER_INVALIDA)
        {
            printf("***Entrada no valida***\n");
            continue;
        }
        if (dato == CENTINELA)
        {
            break;
        }
        if (dato < PALABRA_MIN || dato > PALABRA_MAX)
        {
            printf("***Valor fuera de rango***\n");
            continue;
        }

        mem[direccion] = (int)dato;
        direccion++;
    }

    return direccion;
}

/* Carga el programa desde un archivo. Devuelve cuantas palabras cargo,
   o -1 si el archivo no existe o su contenido no es valido. */
static int cargarDesdeArchivo(int mem[], const char *ruta)
{
    FILE *f = fopen(ruta, "r");
    int direccion = 0;
    int linea = 0;
    long dato = 0;
    int r;

    if (f == NULL)
    {
        printf("***No se pudo abrir el archivo '%s'***\n", ruta);
        return -1;
    }

    for (;;)
    {
        r = leerPalabra(f, &dato);
        linea++;

        if (r == LEER_EOF)
        {
            break;
        }
        if (r == LEER_VACIA)
        {
            continue;
        }
        if (r == LEER_INVALIDA)
        {
            printf("***Archivo no valido: linea %d no es un entero***\n", linea);
            fclose(f);
            return -1;
        }
        if (dato == CENTINELA)
        {
            break;
        }
        if (dato < PALABRA_MIN || dato > PALABRA_MAX)
        {
            printf("***Archivo no valido: linea %d fuera de rango***\n", linea);
            fclose(f);
            return -1;
        }
        if (direccion >= CAPACIDAD)
        {
            printf("***Archivo no valido: el programa excede %d posiciones***\n",
                   CAPACIDAD);
            fclose(f);
            return -1;
        }

        mem[direccion] = (int)dato;
        direccion++;
    }

    fclose(f);

    if (direccion == 0)
    {
        printf("***Archivo no valido: no contiene palabras***\n");
        return -1;
    }

    return direccion;
}

/* Devuelve 1 (y avisa) si el acumulador se salio del rango permitido */
static int verificarAcumulador(const Registros *reg)
{
    if (reg->acc > PALABRA_MAX || reg->acc < PALABRA_MIN)
    {
        printf("***Desbordamiento del acumulador***\n");
        return 1;
    }
    return 0;
}

/* Ciclo fetch-decode-execute.
   Regresa 1 si hubo un error fatal, 0 si termino de forma normal. */
static int correrCiclo(int mem[], Registros *reg)
{
    int activo = 1;
    int hizoBrinco;
    int error = 0;
    int r;
    long valor = 0;

    while (activo)
    {
        /* 1. Traer */
        if (reg->pc < 0 || reg->pc >= CAPACIDAD)
        {
            printf("***Direccion de memoria no valida***\n");
            error = 1;
            break;
        }

        reg->ir = mem[reg->pc];

        /* 2. Decodificar */
        reg->op = reg->ir / 100;
        reg->dir = reg->ir % 100;
        hizoBrinco = 0;

        /* 3. Ejecutar */
        switch (reg->op)
        {
            case OP_READ:
                for (;;)
                {
                    printf("Teclee un entero: ");
                    r = leerPalabra(stdin, &valor);
                    if (r == LEER_EOF)
                    {
                        printf("\n***Se acabaron los datos de entrada***\n");
                        error = 1;
                        activo = 0;
                        break;
                    }
                    if (r == LEER_OK)
                    {
                        if (valor >= PALABRA_MIN && valor <= PALABRA_MAX)
                        {
                            mem[reg->dir] = (int)valor;
                            break;
                        }
                        printf("***Valor fuera de rango***\n");
                    }
                    else if (r == LEER_INVALIDA)
                    {
                        printf("***Entrada no valida***\n");
                    }
                }
                break;

            case OP_WRITE:
                printf("%02d => %d\n", reg->dir, mem[reg->dir]);
                break;

            case OP_NEWLINE:
                printf("\n");
                break;

            case OP_WRITESTR:
            {
                int n = mem[reg->dir];
                int i;

                if (n < 0 || reg->dir + n >= CAPACIDAD)
                {
                    printf("***Cadena no valida***\n");
                    error = 1;
                    activo = 0;
                    break;
                }
                for (i = 1; i <= n; i++)
                {
                    if (mem[reg->dir + i] < 0 || mem[reg->dir + i] > 255)
                    {
                        printf("***Caracter no valido en la cadena***\n");
                        error = 1;
                        activo = 0;
                        break;
                    }
                }
                if (activo)
                {
                    for (i = 1; i <= n; i++)
                    {
                        putchar(mem[reg->dir + i]);
                    }
                }
                break;
            }

            case OP_READSTR:
            {
                char buffer[128];
                size_t n;
                size_t i;

                printf("Teclee una cadena: ");
                if (fgets(buffer, sizeof buffer, stdin) == NULL)
                {
                    printf("\n***Se acabaron los datos de entrada***\n");
                    error = 1;
                    activo = 0;
                    break;
                }
                n = strlen(buffer);
                while (n > 0 && (buffer[n - 1] == '\n' || buffer[n - 1] == '\r'))
                {
                    n--;
                }
                if (reg->dir + (int)n >= CAPACIDAD)
                {
                    printf("***La cadena no cabe en memoria***\n");
                    error = 1;
                    activo = 0;
                    break;
                }
                mem[reg->dir] = (int)n;
                for (i = 0; i < n; i++)
                {
                    mem[reg->dir + 1 + (int)i] = (unsigned char)buffer[i];
                }
                break;
            }

            case OP_LOAD:
                reg->acc = mem[reg->dir];
                break;

            case OP_STORE:
                mem[reg->dir] = reg->acc;
                break;

            case OP_ADD:
                reg->acc = reg->acc + mem[reg->dir];
                if (verificarAcumulador(reg))
                {
                    error = 1;
                    activo = 0;
                }
                break;

            case OP_SUBTRACT:
                reg->acc = reg->acc - mem[reg->dir];
                if (verificarAcumulador(reg))
                {
                    error = 1;
                    activo = 0;
                }
                break;

            case OP_DIVIDE:
                if (mem[reg->dir] == 0)
                {
                    printf("***No se puede dividir entre cero***\n");
                    error = 1;
                    activo = 0;
                }
                else
                {
                    reg->acc = reg->acc / mem[reg->dir];
                }
                break;

            case OP_MULTIPLY:
                reg->acc = reg->acc * mem[reg->dir];
                if (verificarAcumulador(reg))
                {
                    error = 1;
                    activo = 0;
                }
                break;

            case OP_MODULE:
                if (mem[reg->dir] == 0)
                {
                    printf("***No se puede calcular el residuo entre cero***\n");
                    error = 1;
                    activo = 0;
                }
                else
                {
                    reg->acc = reg->acc % mem[reg->dir];
                }
                break;

            case OP_POWER:
            {
                int exponente = mem[reg->dir];
                long resultado = 1;
                int i;

                if (exponente < 0)
                {
                    printf("***Exponente negativo no valido***\n");
                    error = 1;
                    activo = 0;
                    break;
                }

                for (i = 0; i < exponente; i++)
                {
                    resultado = resultado * reg->acc;
                    if (resultado > PALABRA_MAX || resultado < PALABRA_MIN)
                    {
                        printf("***Desbordamiento del acumulador***\n");
                        error = 1;
                        activo = 0;
                        break;
                    }
                }

                if (activo)
                {
                    reg->acc = (int)resultado;
                }
                break;
            }

            case OP_BRANCH:
                reg->pc = reg->dir;
                hizoBrinco = 1;
                break;

            case OP_BRANCHNEG:
                if (reg->acc < 0)
                {
                    reg->pc = reg->dir;
                    hizoBrinco = 1;
                }
                break;

            case OP_BRANCHZERO:
                if (reg->acc == 0)
                {
                    reg->pc = reg->dir;
                    hizoBrinco = 1;
                }
                break;

            case OP_HALT:
                printf("***Termino la ejecucion de Simpletron***\n");
                activo = 0;
                break;

            default:
                printf("***Codigo de operacion no valido***\n");
                error = 1;
                activo = 0;
                break;
        }

        /* 4. Actualizar el contador de instrucciones */
        if (activo && !hizoBrinco)
        {
            reg->pc++;
        }
    }

    return error;
}

/* Vaciado de registros y de la memoria completa */
static void volcarMemoria(const int mem[], const Registros *reg)
{
    int fila, col;

    printf("\n***REGISTROS***\n");
    printf("accumulator:          %+05d\n", reg->acc);
    printf("instructionCounter:   %05d\n", reg->pc);
    printf("instructionRegister:  %+05d\n", reg->ir);
    printf("operationCode:        %02d\n", reg->op);
    printf("operand:              %02d\n", reg->dir);

    printf("\n***MEMORIA***\n\n");

    printf("  ");
    for (col = 0; col < 10; col++)
    {
        printf("%6d", col);
    }
    printf("\n");

    for (fila = 0; fila < 10; fila++)
    {
        printf("%2d", fila * 10);
        for (col = 0; col < 10; col++)
        {
            printf(" %+05d", mem[fila * 10 + col]);
        }
        printf("\n");
    }
}
