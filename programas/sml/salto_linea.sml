; salto_linea.sml - Muestra A y B separados por una linea en blanco
; (extension NEWLINE, opcode 12)
; Area de programa: 00-05 | Area de datos: 06-07
1006 ; 00 READ     A         (lee A en la posicion 06)
1007 ; 01 READ     B         (lee B en la posicion 07)
1106 ; 02 WRITE    A
1200 ; 03 NEWLINE            (imprime un salto de linea)
1107 ; 04 WRITE    B
4300 ; 05 HALT
0000 ; 06 dato A
0000 ; 07 dato B
9999 ; fin del programa
