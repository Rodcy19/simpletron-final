; potencia.sml - Muestra BASE elevada a EXP (extension POWER, opcode 35)
; Area de programa: 00-06 | Area de datos: 07-09
1007 ; 00 READ     BASE      (lee la base en la posicion 07)
1008 ; 01 READ     EXP       (lee el exponente en la posicion 08)
2007 ; 02 LOAD     BASE
3508 ; 03 POWER    EXP       (acc = BASE ^ EXP)
2109 ; 04 STORE    R         (R en la posicion 09)
1109 ; 05 WRITE    R
4300 ; 06 HALT
0000 ; 07 dato BASE
0000 ; 08 dato EXP
0000 ; 09 dato R
9999 ; fin del programa
