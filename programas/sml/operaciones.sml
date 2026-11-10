; operaciones.sml - Muestra A*B y A/B (usa MULTIPLY y DIVIDE)
; Area de programa: 00-10 | Area de datos: 11-14
1011 ; 00 READ      A         (lee A en la posicion 11)
1012 ; 01 READ      B         (lee B en la posicion 12)
2011 ; 02 LOAD      A
3312 ; 03 MULTIPLY  B
2113 ; 04 STORE     R1        (R1 = A * B en la posicion 13)
1113 ; 05 WRITE     R1
2011 ; 06 LOAD      A
3212 ; 07 DIVIDE    B
2114 ; 08 STORE     R2        (R2 = A / B en la posicion 14)
1114 ; 09 WRITE     R2
4300 ; 10 HALT
0000 ; 11 dato A
0000 ; 12 dato B
0000 ; 13 dato R1
0000 ; 14 dato R2
9999 ; fin del programa
