#ifndef CMD_CLIENT_H
    #define CMD_CLIENT_H
    #include "include.h"

    // funzioni per la gestione dei comandi lato client
    void accesso(int sd);
    int cmd_start(int sd);
    bool cmd_look(int sd, char *arg);
    bool cmd_take(int sd, char *arg);
    bool cmd_use(int sd, int argc, char *arg1, char *arg2);
    bool cmd_objs(int sd);
    bool cmd_notes(int sd, bool add); // funzionalità a piacere

#endif