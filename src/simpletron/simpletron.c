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
#define OP_LOAD         20
#define OP_STORE        21
#define OP_ADD          30
#define OP_SUBTRACT     31
#define OP_DIVIDE       32
#define OP_MULTIPLY     33
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
static int  leerPalabra(long *valor);
static int  cargarPrograma(int mem[]);
static int  verificarAcumulador(const Registros *reg);
static int  correrCiclo(int mem[], Registros *reg);
static void volcarMemoria(const int mem[], const Registros *reg);

int main(void)
{
    Registros reg = {0, 0, 0, 0, 0};
    int huboError;

    mostrarBienvenida();
    cargarPrograma(RAM);

    printf("\n***Se termino de cargar el programa***\n");
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
 * Lee una linea de la entrada estandar y extrae un entero.
 * Devuelve LEER_OK, LEER_VACIA (linea en blanco o comentario),
 * LEER_INVALIDA (no es un entero) o LEER_EOF (fin de la entrada).
 */
static int leerPalabra(long *valor)
{
    char linea[128];
    char *p;
    char *fin;

    if (fgets(linea, sizeof linea, stdin) == NULL)
    {
        return LEER_EOF;
    }

    /* si la linea fue mas larga que el buffer, descartar el resto */
    if (strchr(linea, '\n') == NULL && !feof(stdin))
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
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

        r = leerPalabra(&dato);

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
                    r = leerPalabra(&valor);
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
