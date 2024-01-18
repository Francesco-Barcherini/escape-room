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

    #define BUFLEN 256
    #define WORDLEN 20
    #define CMDLEN 6
    #define MAXOBJS 10
    #define MAXLOCS 5
    #define MAXNOTE 3
    #define MAXINVENTARIO 4
    #define NSTANZE 2
    #define MAXCLIENT 5
#endif