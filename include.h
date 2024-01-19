#ifndef INCLUDE_H
    #define INCLUDE_H

    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <errno.h>
    #include <string.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <time.h>

    #define BUFLEN 256 // lunghezza buffer generico
    #define WORDLEN 20 // lunghezza risposte brevi
    #define CMDLEN 6 // lunghezza comandi
    #define MAXOBJS 10  // massimo oggetti in una stanza
    #define MAXLOCS 5   // massimo locazioni in una stanza
    #define MAXNOTE 3   // massimo note in una stanza
    #define MAXINVENTARIO 4 // massimo oggetti collezionabili nell'inventario
    #define NSTANZE 2  // numero di stanze presenti nell'Escape Room
    #define MAXCLIENT 5 // massimo numero di client nella listen
#endif