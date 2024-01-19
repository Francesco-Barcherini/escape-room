#include "strutture.h"

/* Restituisce l'account cercando lo username */
struct Account* cerca_account(struct Account *accounts, char *username) {
    struct Account *corrente = accounts;

    // Cerca l'account nella lista
    while (corrente != NULL) {
        if (!strcmp(corrente->username, username)) {
            printf("Account %s trovato nella lista degli account\n", username);
            return corrente;
        }
        corrente = corrente->next;
    }

    // Account non trovato nella lista
    printf("Account %s non trovato nella lista degli account\n", username);
    return NULL;
}

/* Restituisce l'account cercando lo username e la password */
struct Account* verifica_account(struct Account *accounts, char *username, char *password) {
    struct Account *account;

    // Cerca l'account nella lista
    account = cerca_account(accounts, username);

    // Se l'account non esiste
    if (account == NULL)
        return NULL;

    // Se l'account esiste ma la password non è corretta
    if (strcmp(account->password, password)) {
        printf("Password errata per l'account %s\n", username);
        return NULL;
    }

    // Se l'account esiste e la password è corretta
    printf("Password corretta per l'account %s\n", username);
    return account;
}

/* Aggiunge un account alla lista dati username e password, false se account già presente, true altrimenti*/
bool aggiungi_account(struct Account **accounts, char *username, char *password) {
    struct Account *new_account;

    // Controllo se l'account esiste già
    new_account = cerca_account(*accounts, username);
    if (new_account) {
        printf("Creazione account fallita: account %s già esistente\n", username);
        return false;
    }

    // Creazione nuovo account
    // Il server viene chiuso con CTRL+C prima di aver deallocato la memoria con la free
    // Ci fidiamo del sistema operativo che deallocherà la memoria
    new_account = malloc(sizeof(struct Account));
    if (new_account == NULL) {
        perror("Errore nell'allocazione della memoria per il nuovo account");
        exit(1);
    }

    // inizializzo l'account
    strcpy(new_account->username, username);
    strcpy(new_account->password, password);
    new_account->logged = false;
    new_account->room = -1;
    new_account->esito = pre;

    // Aggiunta nuovo account in testa alla lista
    if (*accounts == NULL) {
        *accounts = new_account;
        new_account->next = NULL;
    } else {
        new_account->next = *accounts;
        *accounts = new_account;
    }

    printf("Creazione account %s riuscita\n\n", username);
    return true;
}

/* Gestisce il logout del client e aggiorna la stanza in cui era */
void logout_account(struct Account *account, struct Partita *rooms) {
    int room;

    room = account->room;
    // aggiorno l'account
    account->logged = false;
    account->room = -1;
    account->esito = pre;

    printf("Account %s disconnesso\n", account->username);

    if (room != -1) {
        // aggiorno la stanza
        rooms[room].connessi--;

        // se la stanza è vuota, la prossima volta la start creerà una nuova partita
        if (rooms[room].connessi == 0)
            printf("La stanza %d è vuota\n\n", room + 1);
    }
}



/* Inizializza partita*/
void init_partita(struct Partita *partita, int room) {
    memset(partita, 0, sizeof(struct Partita));

    // valori comuni a tutte le stanze
    partita->room = room;
    partita->connessi = 1;

    switch (room) // inizializzo le strutture dati in base alla stanza
    {
        case 0:
            partita->totToken = 3;
            partita->nOggetti = 6;
            //init oggetti
                //oggetto 0 -> lettere
                strcpy(partita->oggetti[0].nome, "Lettere");
                strcpy(partita->oggetti[0].desc, 
                    "Sembrano appartenere al vecchio prigioniero, iniziano così:\n"
                    "1603 - Caro JOHN, ...\n"
                    "1598 - SAM, amico mio...\n"
                    "1605 - Mia amata SCARLETT...\n"
                    "1597 - Ciao FLINT, vecchio lupo di mare...\n"
                );

                //oggetto 1 -> chiave
                strcpy(partita->oggetti[1].nome, "Chiave");
                strcpy(partita->oggetti[1].desc, 
                    "Una chiave arrugginita, ma sembra funzionante\n"
                );
                strcpy(partita->oggetti[1].descBloccato, 
                    "Il forziere è chiuso\n"
                );
                partita->oggetti[1].bloccato = true;
                partita->oggetti[1].seconds = 60;
                partita->oggetti[1].tipo = aperta;
                strcpy(partita->oggetti[1].enigma.domanda, "Il forziere è chiuso da un lucchetto a combinazione, inserisci 4 cifre: ");
                strcpy(partita->oggetti[1].enigma.risposta, "5348"); // Lunghezza dei 4 nomi, ordinati per date: FLINT-SAM-JOHN-SCARLETT -> 5348

                //oggetto 2 -> spada
                strcpy(partita->oggetti[2].nome, "Spada");
                strcpy(partita->oggetti[2].desc, 
                    "Il capitano Barbanera ha dimenticato la sua spada\n"
                );
                strcpy(partita->oggetti[2].descBloccato, 
                    "Il capitano Barbanera ha dimenticato la sua spada\n"
                );
                partita->oggetti[2].bloccato = true;
                partita->oggetti[2].token = true;
                partita->oggetti[2].tipo = aperta;
                strcpy(partita->oggetti[2].enigma.domanda, 
                    "La spada è bloccata da un sortilegio. Rispondi alla domanda per spezzarlo.\n"
                    "Quante lettere ha la risposta alla mia domanda? "
                );
                strcpy(partita->oggetti[2].enigma.risposta, "Tre");

                // oggetto 3 -> scellino
                strcpy(partita->oggetti[3].nome, "Scellino");
                strcpy(partita->oggetti[3].desc, 
                    "Uno scellino rotondo di qualche secolo fa\n"
                );

                // oggetto 4 -> mappa
                strcpy(partita->oggetti[4].nome, "Mappa");
                strcpy(partita->oggetti[4].desc, 
                    "La mappa conduce alla mitica città di Atlantide\n"
                );
                strcpy(partita->oggetti[4].descBloccato, 
                    "La mappa potrebbe contenere le indicazioni per un grande tesoro, ma è stretta tra le mani dello ++Scheletro++\n"
                );
                partita->oggetti[4].bloccato = true; // Si sblocca con use Scellino -> lo Scheletro vuole essere pagato
                partita->oggetti[4].token = true;
                partita->oggetti[4].tipo = use;

                // oggetto 5 -> serrature
                strcpy(partita->oggetti[5].nome, "Serrature");
                strcpy(partita->oggetti[5].desc, 
                    "Le ++Sbarre++ sono state aperte, puoi fuggire\n"
                );
                strcpy(partita->oggetti[5].descBloccato, 
                    "Ci sono due serrature, per la prima serve una **Chiave**, la seconda invece è una fessura alta e stretta\n"
                );
                partita->oggetti[5].bloccato = true; // Si sblocca con use Chiave Spada
                partita->oggetti[5].token = true;
                partita->oggetti[5].tipo = use;

            partita->maxPresi = 3;
            partita->nLocazioni = 3;
            // init locazioni
                //locazione 0 -> tavolino
                strcpy(partita->locazioni[0].nome, "Tavolino");
                strcpy(partita->locazioni[0].desc, 
                    "Il tavolino è consumato dalla salsedine. Sopra sono appoggiate delle vecchie **Lettere** e uno **Scellino**. Nel cassettino c'è un forziere chiuso in cui si riesce a intravedere una **Chiave**.\n"
                );

                // locazione 1 -> scheletro
                strcpy(partita->locazioni[1].nome, "Scheletro");
                strcpy(partita->locazioni[1].desc, 
                    "Lo scheletro è del vecchio prigioniero, meglio fuggire presto dalla cella. Ha ancora stretta tra le mani una **Mappa**, ma da buon pirata non la lascerà andare senza un giusto compenso.\n"
                );

                // locazione 2 -> sbarre
                strcpy(partita->locazioni[2].nome, "Sbarre");
                strcpy(partita->locazioni[2].desc, 
                    "Le sbarre sono arrugginite ma per uscire serve aprire le due **Serrature**. Il carceriere ha dimenticato la **Spada** appoggiata alle sbarre.\n"
                );
            // la fine è tra 10 minuti
            partita->fine = time(NULL) + 600;
            break;
        case 1:
            partita->totToken = 2;
            partita->nOggetti = 4;
            // init oggetti
                // oggetto 0 -> lettera
                strcpy(partita->oggetti[0].nome, "Lettera");
                strcpy(partita->oggetti[0].desc, 
                    "La donna ha scritto una lettera d'addio con l'inchiostro UV, strano per una vittima di suicidio\n"
                    "\"Caro fratello,\n"
                    "Addolorata\n"
                    "Rinuncio alla vita.\n"
                    "Lascio a te ogni bene,\n"
                    "Addio\"\n"
                );
                strcpy(partita->oggetti[0].descBloccato, 
                    "La lettera è totalmente bianca, eppure deve esserci scritto qualcosa\n"
                );
                partita->oggetti[0].bloccato = true; // Si sblocca con use Lampada -> la lettera è scritta con inchiostro invisibile
                partita->oggetti[0].token = true;
                partita->oggetti[0].tipo = use;

                // oggetto 1 -> lampada
                strcpy(partita->oggetti[1].nome, "Lampada");
                strcpy(partita->oggetti[1].desc, 
                    "La lampada è una generica abat-jour da ++Comodino++ ma emette raggi di luce ultra-violetta\n"
                );

                //oggetto 2 -> macchina da scrivere
                strcpy(partita->oggetti[2].nome, "Underwood");
                strcpy(partita->oggetti[2].desc, 
                    "Il rapporto è pronto, manca la prova schiacciante\n"
                );
                strcpy(partita->oggetti[2].descBloccato, 
                    "È una vecchia macchina da scrivere, ti servirà per redigere il rapporto. Attenzione: non puoi sbagliare l'idenità del colpevole!! Non prenderla prima di essere sicuro\n"
                );
                partita->oggetti[2].bloccato = true;
                partita->oggetti[2].token = true;
                partita->oggetti[2].tipo = multipla;
                strcpy(partita->oggetti[2].enigma.domanda, 
                    "Sicuro di aver risolto il caso detective?\n"
                    "1) Josè, il maggiordomo\n"
                    "2) Carla, la migliore amica\n"
                    "3) Enzo, il fratello\n"
                    "4) Ginevra, la sorella\n"
                    "Chi è l'assassino? Inserisci il numero: \n"
                );
                strcpy(partita->oggetti[2].enigma.risposta, "2"); // Nella lettera d'addio, le prime lettere di ogni riga formano la parola "CARLA"

                // oggetto 3 -> coltello
                strcpy(partita->oggetti[3].nome, "Coltello"); // distrattore, non serve per vincere
                strcpy(partita->oggetti[3].desc, 
                    "Il coltello è un'affilata lama messicana. È ancora sporco di sangue, è l'arma del delitto.\n"
                );
            partita->maxPresi = 2;
            partita->nLocazioni = 3;
            // init locazioni
                //locazione 0 -> letto
                strcpy(partita->locazioni[0].nome, "Letto");
                strcpy(partita->locazioni[0].desc, 
                    "Sul letto c'è ancora il cadavere della vittima. Accanto alla donna si vede una **Lettera**.\n"
                );

                // locazione 1 -> comodino
                strcpy(partita->locazioni[1].nome, "Comodino");
                strcpy(partita->locazioni[1].desc, 
                    "Il comodino è in disordine, sopra c'è una **Lampada** e hai appoggiato la tua macchina da scrivere, una vecchia **Underwood**, per redigere il rapporto.\n"
                );

                // locazione 2 -> pavimento
                strcpy(partita->locazioni[2].nome, "Pavimento");
                strcpy(partita->locazioni[2].desc, 
                    "Il pavimento è sporco di sangue, sotto il letto c'è un **Coltello**\n"
                );
            // la fine è tra 5 minuti
            partita->fine = time(NULL) + 300;
            break;
        default:
            // espandere per le altre stanze
            break;
    }
    printf("Partita avviata nella stanza %d\n", room + 1);
}

/* In caso di terminazione della partita, informa tutti gli altri account che stavano giocando nella stessa stanza*/
void update_esito(struct Account *accounts, struct Partita *partita, enum EsitoPartita esito) {
    struct Account *corrente = accounts;

    // Cerca nella lista tutti gli account che stavano giocando nella stessa stanza
    while (corrente != NULL) {
        if (corrente->room == partita->room) {
            corrente->esito = esito;
            corrente->room = -1;
        }
        corrente = corrente->next;
    }

    // reset partita, la prossima volta la start creerà una nuova partita
    partita->connessi = 0;

    printf("Partita conclusa nella stanza %d\n\n", partita->room + 1);
}

/* Gestisce l'invio dello stato della partita prima di gestire il comando*/
enum EsitoPartita invia_stato(int sd, struct Account *account, struct Account *accounts, struct Partita *stanze) {
    struct Partita *partita;
    uint8_t esito;
    int room, ret;

    esito = account->esito;
    room = account->room;

    if (room == -1) // la partita è finita a causa di un altro giocatore
        printf("invia_stato: partita conclusa\n\n");
    else
        printf("Check stato della partita %d\n", room + 1);

    if (esito == incorso) {//check timer prima di eseguire il comando
        partita = &stanze[room];
        if (time(NULL) >= partita->fine) { // tempo scaduto
            printf("Tempo scaduto nella stanza %d\n", room + 1);
            update_esito(accounts, partita, timer); // aggiorno gli altri account che stavano giocando nella stessa stanza
        }
    }

    esito = account->esito;

    // Invio esito
    ret = send(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("invia_stato -> Errore nell'invio dell'esito");
        exit(1);
    }

    return esito;    
}

/* Fa il check di vittoria o sconfitta e invia le statistiche a fine comando*/
void fine_comando(int sd, int room, struct Account *account, struct Account *accounts, struct Partita *stanze) {
    struct Partita *partita;
    char buffer[WORDLEN];
    int ret;

    printf("Check fine comando nella stanza %d\n\n", room + 1);

    partita = &stanze[room];

    if (partita->token == partita->totToken) { // vittoria
        printf("Partita vinta nella stanza %d\n", room + 1);
        update_esito(accounts, partita, vittoria);
    }
    else if (time(NULL) >= partita->fine) { // tempo scaduto
        printf("Tempo scaduto nella stanza %d\n", room + 1);
        update_esito(accounts, partita, timer);
    }

    if (!account) // se il client ha chiuso la connessione
        return;

    // Invio informazioni in formato text
    sprintf(buffer, "%d %d %d %ld", account->esito, partita->token, partita->totToken - partita->token, partita->fine - time(NULL));
    ret = send(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("fine_comando -> Errore nell'invio delle statistiche");
        exit(1);
    }
}

/* Gestisce l'interazione client-server per l'enigma, restituisce 0 se sbagliato, 1 se giusto, 2 se client disconnesso*/
uint8_t gestisci_enigma(int sd, struct Partita *partita, int obj, struct Account *accounts) {
    struct Oggetto *oggetto;
    char buffer[BUFLEN];
    char answer[WORDLEN];
    int ret, len;
    uint8_t esito = 0;

    printf("\nGestione enigma\n");

    oggetto = &partita->oggetti[obj];

    // invio il tipo di enigma
    ret = send(sd, &oggetto->tipo, sizeof(oggetto->tipo), 0);
    if (ret == -1) {
        perror("gestisci_enigma -> Errore nell'invio del tipo di enigma");
        exit(1);
    }

    // invio la domanda
    strcpy(buffer, oggetto->enigma.domanda);
    ret = send(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("gestisci_enigma -> Errore nell'invio della domanda");
        exit(1);
    }

    // ricevo la risposta
    // se l'enigma è di tipo aperta ricevo WORDLEN, se multipla ricevo 1
    len = oggetto->tipo == aperta ? WORDLEN : 1;

    ret = recv(sd, answer, len, 0);
    if (ret == -1) {
        perror("gestisci_enigma -> Errore nella ricezione della risposta");
        esito = 0;
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("gestisci_enigma -> Errore nell'invio dell'esito");
            exit(1);
        }
        return 0;
    }

    // se il client ha chiuso la connessione
    if (ret == 0) 
        return 2;

    // se aperta
    if (oggetto->tipo == aperta) {
        // se risposta corretta
        if (!strcmp(answer, oggetto->enigma.risposta)) {
            printf("Risposta all'enigma corretta\n");
            esito = 1;
            // aggiorno oggetto e partita
            oggetto->bloccato = false;
            partita->token += oggetto->token;
            partita->fine += oggetto->seconds;

        }
        else { // se risposta sbagliata
            printf("Risposta all'enigma sbagliata\n");
            esito = 0;
        }

        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("gestisci_enigma -> Errore nell'invio dell'esito");
            exit(1);
        }

        // se la risposta è giusta invio la stringa di sblocco con token e secondi guadagnati
        if (esito) {
            sprintf(buffer, "Oggetto %s sbloccato!", oggetto->nome);
            if (oggetto->token)
                strcat(buffer, "\t+1 token");
            if (oggetto->seconds) {
                sprintf(answer, "\t+%d secondi", oggetto->seconds);
                strcat(buffer, answer);
            }
            ret = send(sd, buffer, sizeof(buffer), 0);
            if (ret == -1) {
                perror("gestisci_enigma -> Errore nell'invio della stringa di sblocco");
                exit(1);
            }
        }

        return esito;
    }

    // se multipla ho one-shot
    if (oggetto->tipo == multipla) {
        // se risposta corretta
        if (!strcmp(answer, oggetto->enigma.risposta)) {
            printf("Risposta all'enigma corretta\n");
            esito = 1;
            // aggiorno oggetto e partita
            oggetto->bloccato = false;
            partita->token += oggetto->token;
            partita->fine += oggetto->seconds;
        }
        else // se risposta sbagliata ho perso
            printf("Risposta multipla sbagliata, stanza %d chiusa\n", partita->room + 1);
        
        // invio esito
        ret = send(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("gestisci_enigma -> Errore nell'invio dell'esito");
            exit(1);
        }

        if (!esito) // avverto gli altri giocatori che ho perso
            update_esito(accounts, partita, risposta);
        else {
            // se la risposta è giusta invio la stringa di sblocco con token e secondi guadagnati
            sprintf(buffer, "Oggetto %s sbloccato!", oggetto->nome);
            if (oggetto->token)
                strcat(buffer, "\t+1 token");
            if (oggetto->seconds) {
                sprintf(answer, "\t+%d secondi", oggetto->seconds);
                strcat(buffer, answer);
            }
            ret = send(sd, buffer, sizeof(buffer), 0);
            if (ret == -1) {
                perror("gestisci_enigma -> Errore nell'invio della stringa di sblocco");
                exit(1);
            }
        }

        return esito;
    }
    // altri tipi di enigma
    return 0;
}

/* Restituisce l'indice dell'oggetto, -1 se non trovato*/
int name2obj(struct Partita *partita, char *nome) {
    int i;

    for (i = 0; i < partita->nOggetti; i++) {
        if (!strcmp(partita->oggetti[i].nome, nome))
            return i;
    }

    return -1;
}