#include "cmd_server.h"

/* Funzione che gestisce il comando sign */
void cmd_sign(int sd, struct Account **accounts) {
    /*
    esito
        0 -> esito positivo
        1 -> errore nella ricezione dei dati
        2 -> account già esistente
    */
    char buffer[BUFLEN]; // buffer per la ricezione dei dati
    char username[WORDLEN], password[WORDLEN];
    int ret;
    uint8_t esito = 0;

    // Ricezione username, \0 compreso
    ret = recv(sd, buffer, WORDLEN, 0);
    if (ret == -1) {
        perror("sign -> Errore nella ricezione dello username");
        esito = 1;
    }
    else
        strncpy(username, buffer, WORDLEN);
    

    // Ricezione password, \0 compreso
    ret = recv(sd, buffer, WORDLEN, 0);
    if (ret == -1) {
        perror("sign -> Errore nella ricezione della password");
        esito = 1;
    }
    else
        strncpy(password, buffer, WORDLEN);

    // Se c'è stato un errore nella ricezione dei dati
    if (esito) {
        // Invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("sign -> Errore nell'invio dell'esito");
            exit(1);
        }
        return;
    }

    // Creazione nuovo account
    ret = aggiungi_account(accounts, username, password);
    if (!ret) {
        // Invio esito negativo
        esito = 2;
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("sign -> Errore nell'invio dell'esito");
            exit(1);
        }
        return;
    }

    // Invio esito positivo
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("sign -> Errore nell'invio dell'esito");
        exit(1);
    }

    return;
}

/* Funzione che gestisce il comando login */
struct Account* cmd_login(int sd, struct Account *accounts) {
    /*
    esito
        0 -> esito positivo
        1 -> errore nella ricezione dei dati
        2 -> account non esistente o password errata
        3 -> account già loggato
    */
    struct Account *account;
    char buffer[BUFLEN]; // buffer per la ricezione dei dati
    char username[WORDLEN], password[WORDLEN];
    int ret;
    uint8_t esito = 0;

    // Ricezione username, \0 compreso
    ret = recv(sd, buffer, WORDLEN, 0);
    if (ret == -1) {
        perror("login -> Errore nella ricezione dello username");
        esito = 1;
    }
    else
        strncpy(username, buffer, WORDLEN);

    // Ricezione password, \0 compreso
    ret = recv(sd, buffer, WORDLEN, 0);
    if (ret == -1) {
        perror("login -> Errore nella ricezione della password");
        esito = 1;
    }
    else
        strncpy(password, buffer, WORDLEN);

    // Se c'è stato un errore nella ricezione dei dati
    if (esito) {
        // Invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("login -> Errore nell'invio dell'esito");
            exit(1);
        }
        return NULL;
    }

    // Verifica account
    account = verifica_account(accounts, username, password);
    if (!account) {
        // Invio esito negativo
        esito = 2;
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("login -> Errore nell'invio dell'esito");
            exit(1);
        }
        return NULL;
    }

    // Se l'account è già loggato
    if (account->logged) {
        // Invio esito negativo
        esito = 3;
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("login -> Errore nell'invio dell'esito");
            exit(1);
        }
        return NULL;
    }

    // aggiorno account
    account->logged = true;

    // Invio esito positivo
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("login -> Errore nell'invio dell'esito");
        exit(1);
    }

    return account;
}

/* Funzione che gestisce il comando start, restituisce true se esito positivo*/
bool cmd_start(int sd, struct Account *account, struct Partita *stanze) {
    /*
    esito
        0 -> esito positivo
        1 -> errore nella ricezione dei dati
    */
    struct Partita *partita;
    uint8_t esito = 0, room = 0;
    int ret;

    ret = recv(sd, (void *)&room, sizeof(uint8_t), 0);
    if (ret == -1) {
        perror("start -> Errore nella ricezione della stanza");
        esito = 1;
    }

    // Se c'è stato un errore nella ricezione dei dati
    if (esito) {
        // Invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("start -> Errore nell'invio dell'esito");
            exit(1);
        }
        return false;
    }

    room--; // nel server room parte da zero

    // aggiorno account
    account->room = room;
    account->esito = incorso;
    // gestione partita
    partita = &stanze[room];
    if (partita->connessi == 0) // riavvio partita
        init_partita(partita, room);
    else
        partita->connessi++;

    // Invio esito positivo
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("start -> Errore nell'invio dell'esito");
        exit(1);
    }

    printf("Utente %s connesso alla stanza %d\n", account->username, room + 1);
    return true;    
}

/* Funzione che gestisce il comando look*/
void cmd_look(int sd, struct Partita partita) {
    char buffer[BUFLEN];
    char nome[WORDLEN];
    int i, ret;

    // look è gestita lato client

    // look [location | object]
    // recupero l'argomento
    ret = recv(sd, nome, sizeof(nome), 0);
    if (ret == -1) {
        perror("look -> Errore nella ricezione dell'argomento");
        return;
    }

    // cerco tra gli oggetti
    for (i = 0; i < partita.nOggetti; i++) {
        if (!strcmp(nome, partita.oggetti[i].nome)) {
            // invio la descrizione
            strcpy(buffer, partita.oggetti[i].bloccato ? partita.oggetti[i].descBloccato : partita.oggetti[i].desc);
            ret = send(sd, buffer, sizeof(buffer), 0);
            if (ret == -1) 
                perror("look -> Errore nell'invio della descrizione");
            else 
                printf("Inviata descrizione oggetto %s\n", nome);
            return;
        }
    }

    // cerco tra le locazioni
    for (i = 0; i < partita.nLocazioni; i++) {
        if (!strcmp(nome, partita.locazioni[i].nome)) {
            // invio la descrizione
            ret = send(sd, partita.locazioni[i].desc, sizeof(partita.locazioni[i].desc), 0);
            if (ret == -1) 
                perror("look -> Errore nell'invio della descrizione");
            else
                printf("Inviata descrizione locazione %s\n", nome);
            return;
        }
    }

    // se non ho trovato nulla
    strcpy(buffer, "notfound");
    ret = send(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) 
        perror("look -> Errore nell'invio della descrizione");
    else
        printf("Descrizione di %s non trovata\n", nome);
}

/* Funzione che gestisce il comando take
ritorna true se client disconnesso*/
bool cmd_take(int sd, struct Partita *partita, struct Account *accounts) {
    /*
    esito:
        0 -> errore nella ricezione dell'argomento
        1 -> non esiste
        2 -> già preso => lasciato
        3 -> bloccato (use)
        4 -> bloccato (enigma)
        5 -> inventario pieno
        6 -> prendibile=>preso
    */
    char nome[WORDLEN];
    struct Oggetto *oggetto = NULL;
    int i, ret;
    uint8_t esito = 0;

    // recupero l'argomento
    ret = recv(sd, nome, sizeof(nome), 0);
    if (ret == -1) {
        perror("take -> Errore nella ricezione dell'argomento");
        
        // Invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("take -> Errore nell'invio dell'esito");
            exit(1);
        }
        return false;
    }

    // cerco tra gli oggetti
    for (i = 0; i < partita->nOggetti; i++) {
        if (!strcmp(nome, partita->oggetti[i].nome)) {
            oggetto = &partita->oggetti[i];
            break;
        }
    }

    // se non ho trovato nulla
    if (!oggetto) {
        esito = 1;
        // invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("take -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Oggetto %s non trovato\n", nome);
        return false;
    }

    // se è già stato preso
    if (oggetto->taken) {
        esito = 2;
        oggetto->taken = false;
        partita->nPresi--;
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("take -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Oggetto %s lasciato\n", nome);
        return false;
    }

    if (oggetto->bloccato) {
        // se si sblocca con una use
        if (oggetto->tipo == use) {
            esito = 3;
            // invio esito negativo
            ret = send(sd, &esito, sizeof(esito), 0);
            if (ret == -1) {
                perror("take -> Errore nell'invio dell'esito");
                exit(1);
            }
            printf("Oggetto %s bloccato da una use\n", nome);
            return false;
        }

        // se si sblocca con un enigma
        esito = 4;
        // invio esito enigma
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("take -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Oggetto %s bloccato da un enigma\n", nome);
        return (gestisci_enigma(sd, partita, i, accounts) == 2 ? true : false); // se il client si è disconnesso ritorno true
    }

    // se l'inventario è pieno
    if (partita->nPresi == partita->maxPresi) {
        esito = 5;
        // invio esito negativo
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("take -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Inventario pieno\n");
        return false;
    }

    // se non è bloccato lo prendo
    esito = 6;
    oggetto->taken = true;
    partita->nPresi++;
    // invio esito positivo
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("take -> Errore nell'invio dell'esito");
        exit(1);
    }
    printf("Oggetto %s preso\n", nome);
    return false;
}

/* Funzione che gestisce il comando use
Ritorna true se il client si è disconnesso*/
bool cmd_use(int sd, struct Partita *partita, struct Account *accounts) {
    /*
    esito:
        0 -> errore nella ricezione degli argomenti
        1 -> oggetto1 non esiste
        2 -> oggetto1 non preso
        3 -> oggetto2 non esiste
        4 -> oggetto2 bloccato (use)
        5 -> oggetto2 bloccato (enigma)
        6 -> oggetti già usati/inutile
        7 -> risultato
    */
    char buffer[BUFLEN];
    char aux[WORDLEN];
    char obj1[WORDLEN];
    char obj2[WORDLEN];
    struct Oggetto *oggetto1 = NULL;
    struct Oggetto *oggetto2 = NULL;
    struct Oggetto *target = NULL;
    int quanti, ret, i;
    int room;
    uint8_t esito;

    room = partita->room;

    // recupero l'argomento: una stringa obj1 obj2
    ret = recv(sd, buffer, 2*WORDLEN, 0);
    if (ret == -1) {
        esito = 0;
        perror("use -> Errore nella ricezione dell'argomento");
        // Invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("use -> Errore nell'invio dell'esito");
            exit(1);
        }
        return false;
    }

    // recupero uno o due oggetti
    quanti = sscanf(buffer, "%s %s", obj1, obj2);

    i = name2obj(partita, obj1);
    oggetto1 = (i == -1) ? NULL : &partita->oggetti[i];
    
    // se non c'è
    if (!oggetto1) {
        esito = 1;
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("use -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Oggetto %s non trovato\n", obj1);
        return false;
    }

    // se non è preso
    if (!oggetto1->taken) {
        esito = 2;
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("use -> Errore nell'invio dell'esito");
            exit(1);
        }
        printf("Oggetto %s non preso\n", obj1);
        return false;
    }

    // se ho un solo oggetto
    if (quanti == 1) {
        // use <object>
        switch (room) // distinguo le stanze
        {
            case 0: // lo scellino sblocca la mappa
                if (!strcmp(obj1, "Scellino"))
                    target = &partita->oggetti[name2obj(partita, "Mappa")];
                break;
            case 1: // la lampada sblocca la lettera
                if (!strcmp(obj1, "Lampada"))
                    target = &partita->oggetti[name2obj(partita, "Lettera")];
                break;
            default:
                // altre stanze
                break;
        }
    }
    else {
        i = name2obj(partita, obj2);
        oggetto2 = (i == -1) ? NULL : &partita->oggetti[i];
        // se non c'è
        if (!oggetto2) {
            esito = 3;
            // invio esito
            ret = send(sd, &esito, sizeof(esito), 0);
            if (ret == -1) {
                perror("use -> Errore nell'invio dell'esito");
                exit(1);
            }
            printf("Oggetto %s non trovato\n", obj2);
            return false;
        }

        // se è bloccato
        if (oggetto2->bloccato) {
            // se si sblocca con una use
            if (oggetto2->tipo == use) {
                esito = 4;
                // invio esito
                ret = send(sd, &esito, sizeof(esito), 0);
                if (ret == -1) {
                    perror("use -> Errore nell'invio dell'esito");
                    exit(1);
                }
                printf("Oggetto %s bloccato da una use\n", obj2);
                return false;
            }

            // se si sblocca con un enigma
            esito = 5;
            // invio esito enigma
            ret = send(sd, &esito, sizeof(esito), 0);
            if (ret == -1) {
                perror("use -> Errore nell'invio dell'esito");
                exit(1);
            }
            printf("Oggetto %s bloccato da un enigma\n", obj2);
            esito = gestisci_enigma(sd, partita, name2obj(partita, obj2), accounts);
            if (esito == 2) // client disconnesso
                return true;
            
            if (esito == 0) // enigma fallito
                return false;

            // enigma superato
        }

        // use <object1> <object2>
        switch (room) // distinguo le stanze
        {
            case 0: // chiave e spada sbloccano serrature
                if (!strcmp(obj1, "Chiave") && !strcmp(obj2, "Spada"))
                    target = &partita->oggetti[name2obj(partita, "Serrature")];
                break;
            case 1: // niente
                break;
            default:
                // altre stanze
                break;
        }
    }

    // in target ho l'oggetto da sbloccare
    // se non ho trovato nulla o se già sbloccato
    if (!target || !target->bloccato) {
        esito = 6;
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("use -> Errore nell'invio dell'esito");
            exit(1);
        }
        if (quanti == 1)
            printf("Oggetto %s già usato o inutile\n", obj1);
        else
            printf("Oggetti %s e %s già usati o inutili\n", obj1, obj2);
        return false;
    }

    // sblocco target e rilascio obj1
    esito = 7;
    target->bloccato = false;
    partita->token += target->token;
    partita->fine += target->seconds;
    oggetto1->taken = false;
    partita->nPresi--;
    // invio esito
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("use -> Errore nell'invio dell'esito");
        exit(1);
    }
    // invio stringa di sblocco
    sprintf(buffer, "Oggetto %s sbloccato!", target->nome);
    if (target->token)
        strcat(buffer, "\t+1 token");
    if (target->seconds) {
        sprintf(aux, "\t+%d secondi", target->seconds);
        strcat(buffer, aux);
    }
    ret = send(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("gestisci_enigma -> Errore nell'invio della stringa di sblocco");
        exit(1);
    }
    if (quanti == 1)
        printf("Use %s: oggetto %s sbloccato\n", obj1, target->nome);
    else
        printf("Use %s %s: oggetto %s sbloccato\n", obj1, obj2, target->nome);
    return false;
}

/* Funzione che gestisce il comando objs*/
void cmd_objs(int sd, struct Partita partita) {
    char buffer[BUFLEN];
    int i, ret;

    // concateno gli oggetti con taken = true
    strcpy(buffer, "");

    for (i = 0; i < partita.nOggetti; i++) {
        if (partita.oggetti[i].taken) {
            strcat(buffer, partita.oggetti[i].nome);
            strcat(buffer, "\n");
        }
    }

    // invio la lista
    ret = send(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) 
        perror("objs -> Errore nell'invio della lista");
    else
        printf("Inviata lista oggetti\n");

    return;
}

/* 
Funzionalità a piacere: i giocatori in una stanza condividono un blocco note in cui possono annotarsi delle osservazioni visibili dagli altri giocatori
Nel blocco note ci sono massimo MAXNOTES note, gestite come una coda FIFO
notes -> stampa le note presenti nel blocco note
notes add -> aggiunge una nota al blocco note
*/
/* Funzione che gestisce il comando notes*/
void cmd_notes(int sd, struct Partita *partita) {
    // notes -> visualizza le note
    // notes add -> aggiunge una nota
    // gestisco le note come una coda circolare
    char buffer[MAXNOTE * BUFLEN];
    int i, ret;
    bool add;
    uint16_t len;

    // recupero add
    ret = recv(sd, &add, sizeof(add), 0);
    if (ret == -1) {
        perror("notes -> Errore nella ricezione di add");
        return;
    }

    if (!add) {
        // concateno le note
            //char note[MAXNOTE+1][BUFLEN]; //coda circolare da primanota a ultimanota
            //int primanota;
            //int ultimanota;
        strcpy(buffer, "");
        for (i = partita->primanota; i != partita->ultimanota; i = (i + 1) % (MAXNOTE + 1)) {
            strcat(buffer, partita->note[i]);
            strcat(buffer, "\n");
        }

        printf("Note:\n%s", buffer);

        // invio la lunghezza della lista (senza \0)
        len = strlen(buffer);
        len = htons(len); // converto in network byte order
        ret = send(sd, &len, sizeof(len), 0);
        if (ret == -1) {
            perror("notes -> Errore nell'invio della lunghezza della lista");
            return;
        }
        len = ntohs(len); // converto in host byte order

        if (len != 0) {
            // invio la lista
            ret = send(sd, buffer, len, 0);
            if (ret == -1) {
                perror("notes -> Errore nell'invio della lista");
                return;
            }
            printf("Inviata lista note\n");
        }
        return;
    }

    // se add == true
    // recupero la nota
    ret = recv(sd, buffer, BUFLEN, 0);
    if (ret == -1) {
        perror("notes -> Errore nella ricezione della nota");
        return;
    }

    // se ho ricevuto qualcosa aggiungo una nota
    strcpy(partita->note[partita->ultimanota], buffer);
    partita->ultimanota = (partita->ultimanota + 1) % (MAXNOTE + 1);
    // se la coda è piena sovrascrivo la prima nota
    if (partita->ultimanota == partita->primanota)
        partita->primanota = (partita->primanota + 1) % (MAXNOTE + 1);
    
    printf("Aggiunta nota\n");
}