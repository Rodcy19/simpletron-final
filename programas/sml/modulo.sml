; modulo.sml - Muestra el residuo de A entre B (extension MODULE, opcode 34)
; Area de programa: 00-06 | Area de datos: 07-09
1007 ; 00 READ     A         (lee A en la posicion 07)
1008 ; 01 READ     B         (lee B en la posicion 08)
2007 ; 02 LOAD     A
3408 ; 03 MODULE   B         (acc = A % B)
2109 ; 04 STORE    R         (R en la posicion 09)
1109 ; 05 WRITE    R
4300 ; 06 HALT
0000 ; 07 dato A
0000 ; 08 dato B
0000 ; 09 dato R
9999 ; fin del programa
