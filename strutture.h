#ifndef STRUTTURE_H
#define STRUTTURE_H

#include "include.h"
enum EsitoPartita {pre, incorso, vittoria, timer, risposta}; //pre = partita non iniziata, incorso = partita in corso, vittoria = partita vinta, timer = tempo scaduto, risposta = partita terminata per risposta a tentativi sbagliata

struct Account {
    char username[WORDLEN];
    char password[WORDLEN];
    bool logged;
    int room; //-1 se non è in nessuna stanza
    enum EsitoPartita esito; // stato della stanza in cui sta giocando, controllato prima e dopo ogni comando
    struct Account *next; //puntatore al prossimo account nella lista
};

struct Locazione {
    char nome[WORDLEN];
    char desc[BUFLEN];
};

struct Enigma {
    char domanda[BUFLEN];
    char risposta[WORDLEN]; 
};

enum TipoEnigma {aperta, multipla, use}; //aperta = enigma a risposta aperta, multipla = enigma a risposta multipla, use = l'oggetto si sblocca con una use

struct Oggetto {
    char nome[WORDLEN];
    char desc[BUFLEN];
    char descBloccato[BUFLEN]; //descrizione quando l'oggetto è bloccato
    bool bloccato;
    bool token; //true se fa guadagnare un token quando sbloccato
    int seconds; //secondi che si guadagnano quando sbloccato
    bool taken; //true se è stato preso nell'inventario
    enum TipoEnigma tipo; //tipo di enigma, serve a capire come gestirlo
    struct Enigma enigma;
};

struct Partita {
    int room; //numero della stanza
    int connessi; //numero di giocatori connessi, se zero la stanza viene eliminata
    int totToken; //token totali da recuperare per vincere
    int token;  //token guadagnati
    int nOggetti;   //numero di oggetti nella stanza
    struct Oggetto oggetti[MAXOBJS];
    int maxPresi;  //numero massimo di oggetti che si possono prendere nell'inventario
    int nPresi; //numero di oggetti presi nell'inventario
    int nLocazioni; //numero di locazioni nella stanza
    struct Locazione locazioni[MAXLOCS];
    char note[MAXNOTE+1][BUFLEN]; //funzionalità a piacere: blocco note di massimo MAXNOTE note, gestite come una coda circolare in modalità FIFO
    int primanota; // le note vanno da primanota
    int ultimanota; // a ultimanota esclusa
    time_t fine; //timestamp di fine della partita
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
