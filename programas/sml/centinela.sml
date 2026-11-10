; centinela.sml - Suma enteros hasta que se capture 0 (REPETICION)
; Area de programa: 00-08 | Area de datos: 09-10
1009 ; 00 READ        N          (lee N en la posicion 09)
2009 ; 01 LOAD        N
4207 ; 02 BRANCHZERO  07         (si N == 0, terminar)
2010 ; 03 LOAD        SUMA
3009 ; 04 ADD         N
2110 ; 05 STORE       SUMA
4000 ; 06 BRANCH      00         (repetir)
1110 ; 07 WRITE       SUMA
4300 ; 08 HALT
0000 ; 09 dato N
0000 ; 10 dato SUMA
9999 ; fin del programa
