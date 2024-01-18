#ifndef STRUTTURE_H
#define STRUTTURE_H

#include "include.h"
enum EsitoPartita {pre, incorso, vittoria, timer, risposta};

struct Account {
    char username[WORDLEN];
    char password[WORDLEN];
    bool logged;
    int room; //-1 se non è in nessuna stanza
    enum EsitoPartita esito;
    struct Account *next;
};

struct Locazione {
    char nome[WORDLEN];
    char desc[BUFLEN];
};

struct Enigma {
    char domanda[BUFLEN];
    char risposta[WORDLEN]; 
};

enum TipoEnigma {aperta, multipla, use};

struct Oggetto {
    char nome[WORDLEN];
    char desc[BUFLEN];
    char descBloccato[BUFLEN]; 
    bool bloccato;
    bool token; //true se fa guadagnare un token quando sbloccato
    int seconds; //secondi che si guadagnano quando sbloccato
    bool taken; //true se è stato preso
    enum TipoEnigma tipo;
    struct Enigma enigma;
};

struct Partita {
    int room;
    int connessi;
    int totToken;
    int token;
    int nOggetti;
    struct Oggetto oggetti[MAXOBJS];
    int maxPresi;
    int nPresi;
    int nLocazioni;
    struct Locazione locazioni[MAXLOCS];
    char note[MAXNOTE+1][BUFLEN]; //coda circolare
    int primanota;
    int ultimanota;
    time_t fine;
};

// funzioni per la gestione degli account
struct Account* cerca_account(struct Account *accounts, char *username);
struct Account* verifica_account(struct Account *accounts, char *username, char *password);
bool aggiungi_account(struct Account **accounts, char *username, char *password);
void logout_account(struct Account *account, struct Partita *rooms);

// funzioni per la gestione delle partite
void init_partita(struct Partita *partita, int room);
enum EsitoPartita invia_stato(int sd, struct Account *account, struct Account *accounts, struct Partita *stanze);
void fine_comando(int sd, int room, struct Account *account, struct Account *accounts, struct Partita *stanze);
uint8_t gestisci_enigma(int sd, struct Partita *partita, int obj, struct Account *accounts);
int name2obj(struct Partita *partita, char *nome);

#endif
