; hola.sml - Imprime la cadena "Hola" (extension WRITESTR, opcode 13)
; Area de programa: 00-02 | Area de datos: 03-07 (cadena)
; Formato de la cadena: primero la longitud, luego un codigo ASCII por palabra
1303 ; 00 WRITESTR  03      (imprime la cadena que empieza en la posicion 03)
1200 ; 01 NEWLINE
4300 ; 02 HALT
0004 ; 03 longitud = 4
0072 ; 04 'H'
0111 ; 05 'o'
0108 ; 06 'l'
0097 ; 07 'a'
9999 ; fin del programa
