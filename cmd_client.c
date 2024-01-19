#include "cmd_client.h"
#include "util.h"

bool enigmaRisolto = false; // mi serve per capire se ho risolto l'enigma e quindi posso usare l'oggetto nella use

/* Controlla lo stato della partita e il tempo, restituisce true se la partita è finita*/
bool gestisci_invia_stato(int sd) {
    uint8_t stato;
    enum EsitoPartita esito; // stato della partita
    int ret;

    // ricezione stato  
    ret = recv(sd, &stato, sizeof(stato), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dello stato");
        exit(1);
    }

    esito = (enum EsitoPartita)stato;

    if (esito == incorso)
        return false;

    // la partita è terminata
    switch (esito)
    {
        case vittoria:
            printf("Complimenti, hai vinto! Ottimo lavoro di squadra!\n\n");
            break;
        case timer:
            printf("Tempo scaduto: hai perso :(\n\n");
            break;
        case risposta:
            printf("Risposta sbagliata a un'enigma a tentativi limitati: hai perso :(\n\n");
            break;
        default:
            break;
    }

    return true;
}

/* Riceve e mostra a video le statistiche della partita e l'esito del comando, ritorna true se la partita è finita*/
bool gestisci_fine_comando(int sd) {
    char buffer[BUFLEN];
    enum EsitoPartita esito; // stato della partita
    int ret, token, tokenRimanenti;
    time_t seconds;

    // ricezione esito e info sulla partita
    ret = recv(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito del comando");
        exit(1);
    }
    sscanf(buffer, "%d %d %d %ld", (int *)&esito, &token, &tokenRimanenti, &seconds);
    
    if (esito == timer)
        seconds = 0;
    
    printf("Token raccolti: %d\tToken rimanenti: %d\tTempo rimanente: %02ld:%02ld\n\n", token, tokenRimanenti, seconds/60, seconds % 60);

    if (esito == incorso)
        return false;

    // la partita è terminata
    switch (esito)
    {
        case vittoria:
            printf("Complimenti, hai vinto!\n\n");
            break;
        case timer:
            printf("Tempo scaduto: hai perso :(\n\n");
            break;
        case risposta:
            printf("Risposta sbagliata: hai perso :(\n\n");
            break;
        default:
            break;
    }

    return true;
}

/* Funzione che gestisce l'enigma, restituisce true se la partita finisce*/
bool enigma(int sd) {
    char buffer[BUFLEN], risposta[BUFLEN];
    int ret;
    uint8_t esito;
    enum TipoEnigma tipo;

    // ricezione tipo enigma
    ret = recv(sd, &tipo, sizeof(tipo), 0);
    if (ret == -1) {
        perror("Errore nella ricezione del tipo di enigma");
        exit(1);
    }

    // ricezione domanda
    ret = recv(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("Errore nella ricezione della domanda");
        exit(1);
    }

    if (tipo == aperta) { // il server si aspetta una risposta lunga WORDLEN
        printf("Enigma:\n%s\n", buffer);
        fgets(risposta, sizeof(risposta), stdin);
        sscanf(risposta, "%19s", buffer);

        // invio risposta
        ret = send(sd, buffer, WORDLEN, 0);
        if (ret == -1) {
            perror("Errore nell'invio della risposta");
            exit(1);
        }

        // ricezione esito
        ret = recv(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("Errore nella ricezione dell'esito dell'enigma");
            exit(1);
        }

        if (esito == 0) {// risposta sbagliata
            printf("Enigma non risolto\n\n");
            enigmaRisolto = false;
        }
        else {
            // risposta corretta -> ricezione stringa di sblocco
            ret = recv(sd, buffer, sizeof(buffer), 0);
            if (ret == -1) {
                perror("Errore nella ricezione della stringa di sblocco");
                exit(1);
            }

            printf("%s\n\n", buffer);
            enigmaRisolto = true;
        }
        
        return false;
    }
    else if (tipo == multipla) { // il server si aspetta una risposta lunga 1  
        printf("Enigma (attenzione, hai un solo tentativo):\n%s", buffer); // la risposta multipla ha un solo tentativo, altrimenti si perde
        while (true) {
            fgets(risposta, sizeof(risposta), stdin);
            sscanf(risposta, "%19s", buffer);
            if (strlen(buffer) > 1 || buffer[0] < '1' || buffer[0] > '4') {
                printf("Formato della risposta non valido, riprova:\n");
                continue;
            }
            break;
        }
        // invio risposta
        ret = send(sd, buffer, 1, 0);
        if (ret == -1) {
            perror("Errore nell'invio della risposta");
            exit(1);
        }

        // ricezione esito
        ret = recv(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("Errore nella ricezione dell'esito dell'enigma");
            exit(1);
        }

        if (esito == 0) { // risposta sbagliata
            printf("Enigma non risolto\n\n");
            return true;
        }

        // risposta corretta -> ricezione stringa di sblocco
        ret = recv(sd, buffer, sizeof(buffer), 0);
        if (ret == -1) {
            perror("Errore nella ricezione della stringa di sblocco");
            exit(1);
        }

        printf("%s\n\n", buffer);
        enigmaRisolto = true;
        return false;
    }
    else {
        // nuovi tipi di domanda
    }
    return false;
}

/* Funzione che gestisce la signup*/
void signup(int sd) {
    char username[WORDLEN], password[WORDLEN], comando[CMDLEN];
    int ret;
    uint8_t esito; // 0 ok, 1 errore, 2 username già esistente

    strcpy(comando, "sign");

    // lettura username
    printf("Inserisci username: ");
    scanf("%19s", username);

    // lettura password
    printf("Inserisci password: ");
    scanf("%19s", password);

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);

    // invio username
    ret = send(sd, username, sizeof(username), 0);
    if (ret == -1) {
        perror("Errore nell'invio dello username");
        return;
    }

    // invio password
    ret = send(sd, password, sizeof(password), 0);
    if (ret == -1) {
        perror("Errore nell'invio della password");
        return;
    }

    // ricezione esito
    ret = recv(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito della registrazione");
        return;
    }

    if (esito == 0)
        printf("Registrazione effettuata con successo, ora effettua il login con le tue credenziali\n\n");
    else if (esito == 1)
        printf("Errore di connessione: riprova\n\n");
    else if (esito == 2)
        printf("Esiste già un account con questo username: riprova con un altro username o effettua il login\n\n");
    else
        printf("Errore di connessione con il server: riprova\n\n");
}

/* Funzione che gestisce il login, restituisce true se va a buon fine*/
bool login(int sd) {
    char username[WORDLEN], password[WORDLEN], comando[CMDLEN];
    int ret;
    uint8_t esito; // 0 ok, 1 errore, 2 non esistente o pass errata, 3 già loggato

    strcpy(comando, "login");

    // lettura username
    printf("Inserisci username: ");
    scanf("%19s", username);

    // lettura password
    printf("Inserisci password: ");
    scanf("%19s", password);

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);

    // invio username
    ret = send(sd, username, sizeof(username), 0);
    if (ret == -1) {
        perror("Errore nell'invio dello username");
        return false;
    }

    // invio password
    ret = send(sd, password, sizeof(password), 0);
    if (ret == -1) {
        perror("Errore nell'invio della password");
        return false;
    }

    // ricezione esito
    ret = recv(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito del login");
        return false;
    }

    if (esito == 0) {
        printf("Bentornato nell'escape room!\nSvela i misteri, risolvi gli enigmi e fuggi dalla stanza prima che sia troppo tardi!\n\n");
        return true;
    }

    if (esito == 1)
        printf("Errore di connessione: riprova\n\n");
    else if (esito == 2)
        printf("Account non trovato o password errata: riprova\n\n");
    else if (esito == 3)
        printf("Account già online: disconnetti l'account e riprova\n\n");
    else
        printf("Errore di connessione con il server: riprova\n\n");

    return false;
}

/* Funzione che gestisce signup e login*/
void accesso(int sd) {
    char comando[WORDLEN];

    while (true)
    {
        mostra_accesso();

        // lettura comando
        scanf("%6s", comando);

        if (!strcmp(comando, "signup")) // signup
            signup(sd);
        else if (!strcmp(comando, "login")) { //login
            if (login(sd))
                break; // login andato a buon fine, esco dalla procedura di accesso
        }
        else
            printf("Comando non valido, riprova\n\n");
    }
}

/* Funzione che gestisce il comando start, restituisce la room o -1 in caso di errore*/
int cmd_start(int sd) {
    char buffer[BUFLEN], comando[WORDLEN];
    uint8_t esito, room;
    int ret;

    while (true) {
        // invia stanze
        mostra_stanze();

        // lettura comando
        fgets(buffer, BUFLEN, stdin);
        ret = sscanf(buffer, "%5s %hhu", comando, &room);
        if (ret != 2) {
            printf("Comando non valido, riprova\n\n");
            continue;
        }

        if (strcmp(comando, "start")) {
            printf("Comando non valido, riprova\n\n");
            continue;
        }

        if (room <= 0 || room > NSTANZE) {
            printf("Seleziona una stanza valida\n\n");
            continue;
        }

        // invio comando
        ret = send(sd, comando, CMDLEN, 0);
        if (ret == -1) {
            perror("Errore nell'invio del comando start");
            exit(1);
        }

        // invio room
        ret = send(sd, (void *)&room, sizeof(uint8_t), 0);
        if (ret == -1) {
            perror("Errore nell'invio della room");
            exit(1);
        }

        // ricezione esito
        ret = recv(sd, &esito, sizeof(esito), 0);
        if (ret == -1) {
            perror("Errore nella ricezione dell'esito del comando start");
            exit(1);
        }

        if (esito == 0) {
            printf("Stanza %hhu selezionata\n\n", room);
            printf("********************************************************************\n");
            printf("*                                                                  *\n");  
            nome_stanza(room);
            printf("*                                                                  *\n");
            printf("********************************************************************\n\n");

            descrizione_stanza(room);

            gestisci_fine_comando(sd);

            return (int)room;
        }
        else if (esito == 1)
            printf("Errore di connessione: riprova\n\n");
        else
            printf("Errore di connessione con il server: riprova\n\n");
    }
}

/* Funzione che gestisce look con argomento, restituisce true se la partita è finita*/
bool cmd_look(int sd, char *arg) {
    char buffer[BUFLEN], comando[CMDLEN];
    int ret;
    bool fine;

    strcpy(comando, "look");

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);
    if (ret == -1) {
        perror("Errore nell'invio del comando look");
        exit(1);
    }

    fine = gestisci_invia_stato(sd); // dopo aver inviato il comando ricevo lo stato della partita
    if (fine)
        return true;

    // invio argomento
    ret = send(sd, arg, WORDLEN, 0);
    if (ret == -1) {
        perror("Errore nell'invio dell'argomento");
        exit(1);
    }

    // ricezione esito
    ret = recv(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito del comando look");
        exit(1);
    }

    if (!strcmp(buffer, "notfound"))
        printf("Oggetto %s non presente nella stanza\n\n", arg);
    else
        printf("%s\n", buffer);

    return gestisci_fine_comando(sd); // alla fine del comando controllo lo stato della partita e stampo le statistiche
}

/* Funzione che gestisce take con argomento, restituisce true se la partita è finita*/
bool cmd_take(int sd, char *arg) {
    char comando[CMDLEN];
    int ret;
    uint8_t esito;
    bool fine = false;

    strcpy(comando, "take");

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);
    if (ret == -1) {
        perror("Errore nell'invio del comando take");
        exit(1);
    }

    fine = gestisci_invia_stato(sd); // dopo aver inviato il comando ricevo lo stato della partita
    if (fine)
        return true;

    // invio argomento
    ret = send(sd, arg, WORDLEN, 0);
    if (ret == -1) {
        perror("Errore nell'invio dell'argomento");
        exit(1);
    }

    // ricezione esito
    ret = recv(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito del comando take");
        exit(1);
    }

    switch (esito)
    {
        case 0:
            printf("Errore di connessione: riprova\n\n");
            break;
        case 1:
            printf("Oggetto %s non presente nella stanza\n\n", arg);
            break;
        case 2:
            printf("Oggetto %s depositato\n\n", arg);
            break;
        case 3:
            printf("Oggetto %s bloccato\n\n", arg);
            break;
        case 4:
            printf("Oggetto %s bloccato da un enigma\n\n", arg);
            fine = enigma(sd); // se il gioco è terminato fine = true;
            break;
        case 5:
            printf("Inventario pieno, deposita un oggetto\n\n");
            break;
        case 6:
            printf("Oggetto %s raccolto\n\n", arg);
            break;
        default:
            break;
    }

    fine |= gestisci_fine_comando(sd); // il gioco finisce se ho risposto male a una domanda a tentativi o per i soliti motivi (timer, vittoria)

    return fine;
}

/* Funzione che gestisce use al variare degli argomenti, restituisce true se termina la partita*/
bool cmd_use(int sd, int argc, char *arg1, char *arg2) {
    char comando[CMDLEN], buffer[BUFLEN];
    int ret;
    uint8_t esito;
    bool fine = false;

    strcpy(comando, "use");

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);
    if (ret == -1) {
        perror("Errore nell'invio del comando use");
        exit(1);
    }

    fine = gestisci_invia_stato(sd); // dopo aver inviato il comando ricevo lo stato della partita
    if (fine)
        return true;

    // creo una stringa obj1 [obj2]
    if (argc == 2)
        sprintf(buffer, "%s", arg1);
    else
        sprintf(buffer, "%s %s", arg1, arg2);

    // invio argomenti
    ret = send(sd, buffer, 2*WORDLEN, 0);
    if (ret == -1) {
        perror("Errore nell'invio degli argomenti");
        exit(1);
    }

    // ricezione esito
    ret = recv(sd, &esito, sizeof(esito), 0);
    if (ret == -1) {
        perror("Errore nella ricezione dell'esito del comando use");
        exit(1);
    }

    switch (esito)
    {
        case 0:
            printf("Errore di connessione: riprova\n\n");
            break;
        case 1:
            printf("Oggetto %s non presente nella stanza\n\n", arg1);
            break;
        case 2:
            printf("L'oggetto %s non è nell'inventario: prova 'take %s'\n\n", arg1, arg1);
            break;
        case 3:
            printf("Oggetto %s non presente nella stanza\n\n", arg2);
            break;
        case 4:
            printf("Oggetto %s bloccato\n\n", arg2);
            break;
        case 5:
            printf("Oggetto %s bloccato da un enigma\n\n", arg2);
            fine = enigma(sd); // se il gioco è terminato fine = true;
            if (fine) {
                gestisci_fine_comando(sd);
                return true;
            }

            if (!enigmaRisolto) { // se non ho risolto l'enigma non posso usare l'oggetto
                printf("use fallita: oggetto %s non sbloccato\n\n", arg2);
                return gestisci_fine_comando(sd); // alla fine del comando controllo lo stato della partita e stampo le statistiche
            }

            // recupero l'esito della use
            ret = recv(sd, &esito, sizeof(esito), 0);
            if (ret == -1) {
                perror("Errore nella ricezione dell'esito del comando use");
                exit(1);
            }

            if (esito == 6) 
                printf("use: non è successo niente\n\n");
            else if (esito == 7) {
                // ho sbloccato un oggetto: recupero e stampo la stringa di sblocco
                ret = recv(sd, buffer, sizeof(buffer), 0);
                if (ret == -1) {
                    perror("Errore nella ricezione della stringa di sblocco");
                    exit(1);
                }
                printf("%s\n\n", buffer);
            }
            break;
        case 6:
            printf("use: non è successo niente\n\n");
            break;
        case 7:
            // ho sbloccato un oggetto: recupero e stampo la stringa di sblocco
            ret = recv(sd, buffer, sizeof(buffer), 0);
            if (ret == -1) {
                perror("Errore nella ricezione della stringa di sblocco");
                exit(1);
            }
            printf("%s\n\n", buffer);
            break;
        default:
            break;
    }

    return gestisci_fine_comando(sd); // alla fine del comando controllo lo stato della partita e stampo le statistiche
}

/* Funzione che gestisce il comando objs, restituisce true se la partita è finita*/
bool cmd_objs(int sd) {
    char comando[CMDLEN], buffer[BUFLEN];
    int ret;
    bool fine;

    strcpy(comando, "objs");

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);
    if (ret == -1) {
        perror("Errore nell'invio del comando objs");
        exit(1);
    }

    fine = gestisci_invia_stato(sd); // dopo aver inviato il comando ricevo lo stato della partita
    if (fine)
        return true;

    // ricezione lista
    ret = recv(sd, buffer, sizeof(buffer), 0);
    if (ret == -1) {
        perror("Errore nella ricezione della lista degli oggetti");
        exit(1);
    }

    printf("\nInventario:\n%s\n", buffer);

    return gestisci_fine_comando(sd); // alla fine del comando controllo lo stato della partita e stampo le statistiche
}

/* 
Funzionalità a piacere: i giocatori in una stanza condividono un blocco note in cui possono annotarsi delle osservazioni visibili dagli altri giocatori
Nel blocco note ci sono massimo MAXNOTES note, gestite come una coda FIFO
notes -> stampa le note presenti nel blocco note
notes add -> aggiunge una nota al blocco note
*/
/* Funzione che gestisce il comando notes, restituisce true se la partita è finita */
bool cmd_notes(int sd, bool add) {
    // add -> leggo la nuova nota, altrimenti stampo le note presenti
    char comando[CMDLEN], buffer[MAXNOTE * BUFLEN];
    uint16_t len;
    int ret;
    bool fine;

    strcpy(comando, "notes");

    // invio comando
    ret = send(sd, comando, sizeof(comando), 0);
    if (ret == -1) {
        perror("Errore nell'invio del comando notes");
        exit(1);
    }

    fine = gestisci_invia_stato(sd); // dopo aver inviato il comando ricevo lo stato della partita
    if (fine)
        return true;

    // invio add
    ret = send(sd, &add, sizeof(add), 0);
    if (ret == -1) {
        perror("Errore nell'invio di add");
        exit(1);
    }

    if (add) {
        printf("Inserisci la nota (max %d caratteri):\n", BUFLEN - 1);
        fgets(buffer, BUFLEN, stdin);
        printf("\n");
        len = strlen(buffer);
        // sostituisco \n con \0
        buffer[len - 1] = '\0';

        // invio nota
        ret = send(sd, buffer, BUFLEN, 0);
        if (ret == -1) {
            perror("Errore nell'invio della nota");
            exit(1);
        }
    }
    else {
        // ricezione lunghezza note
        ret = recv(sd, &len, sizeof(len), 0);
        if (ret == -1) {
            perror("Errore nella ricezione della lunghezza delle note");
            exit(1);
        }
        
        len = ntohs(len); // converto in formato host
        strcpy(buffer, "");

        if (len != 0) {
            // ricezione note
            ret = recv(sd, buffer, len, 0);
            if (ret == -1) {
                perror("Errore nella ricezione delle note");
                exit(1);
            }

            buffer[ret] = '\0';
        }

        printf("\nNote:\n%s\n", buffer);
    }

    return gestisci_fine_comando(sd); // alla fine del comando controllo lo stato della partita e stampo le statistiche
}