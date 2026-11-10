; eco.sml - Lee una cadena del teclado y la repite (READSTR 14 y WRITESTR 13)
; Area de programa: 00-03 | Area de datos: 04 en adelante (buffer de la cadena)
1404 ; 00 READSTR   04      (guarda longitud en 04 y caracteres desde 05)
1304 ; 01 WRITESTR  04      (imprime la cadena leida)
1200 ; 02 NEWLINE
4300 ; 03 HALT
0000 ; 04 buffer: longitud (las posiciones siguientes quedan libres)
9999 ; fin del programa
