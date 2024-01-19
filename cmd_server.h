#ifndef CMD_SERVER_H
    #define CMD_SERVER_H
    #include "include.h"
    #include "strutture.h"

    void cmd_sign(int sd, struct Account **accounts);
    struct Account* cmd_login(int sd, struct Account *accounts);
    bool cmd_start(int sd, struct Account *account, struct Partita *stanze);
    void cmd_look(int sd, struct Partita partita);
    bool cmd_take(int sd, struct Partita *partita, struct Account *accounts);
    bool cmd_use(int sd, struct Partita *partita, struct Account *accounts);
    void cmd_notes(int sd, struct Partita *partita); // funzionalità a piacere
    void cmd_objs(int sd, struct Partita partita);

#endif