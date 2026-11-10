; mayor.sml - Muestra el mayor de dos enteros (SELECCION)
; Area de programa: 00-08 | Area de datos: 09-10
1009 ; 00 READ       A         (lee A en la posicion 09)
1010 ; 01 READ       B         (lee B en la posicion 10)
2009 ; 02 LOAD       A
3110 ; 03 SUBTRACT   B         (acc = A - B)
4107 ; 04 BRANCHNEG  07        (si A < B, ir a la 07)
1109 ; 05 WRITE      A         (A >= B: el mayor es A)
4300 ; 06 HALT
1110 ; 07 WRITE      B         (B es el mayor)
4300 ; 08 HALT
0000 ; 09 dato A
0000 ; 10 dato B
9999 ; fin del programa
