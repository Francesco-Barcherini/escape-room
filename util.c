#include "include.h"
#include "util.h"

/* Funzione che mostra a video i comandi disponibili per il server*/
void mostra_comandi_server(){
    printf("************************** COMANDI SERVER **************************\n");
    printf("Digita un comando:\n\n");
    printf("1)  start <port> --> Avvia il server di gioco\n");
    printf("2)  stop --> Termina il server\n");
    printf("********************************************************************\n\n");
}

/* Funzione che chiama socket() e inizializza l'indirizzo del server*/
int crea_addr_server(struct sockaddr_in *my_addr, uint16_t porta){
    int sd;

    // Creazione socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        perror("Errore nella creazione del socket");
        exit(1);
    }
    printf("Socket %d creato\n", sd);
    
    // Inizializzazione
    memset(my_addr, 0, sizeof(*my_addr));

    my_addr->sin_port = porta;
    my_addr->sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &my_addr->sin_addr);
    printf("Indirizzo server: 127.0.0.1:%hu\n", ntohs(my_addr->sin_port));

    return sd;
}

/* Funzione che chiama socket() e bind()*/
int crea_socket_server(struct sockaddr_in *my_addr, uint16_t porta){
    int sd, ret;

    // Creazione socket
    sd = crea_addr_server(my_addr, porta);

    ret = bind(sd, (struct sockaddr*)my_addr, sizeof(*my_addr));
    if(ret == -1){
        perror("Errore nella bind");
        exit(1);
    }
    printf("Assegnato indirizzo e porta al socket %d\n", sd);

    return sd;
}


/* Funzione che mostra a video i comandi disponibili per l'accesso*/
void mostra_accesso(){
    printf("***************************** ACCESSO ******************************\n");
    printf("Digita un comando:\n\n");
    printf("1)  signup --> Registra un nuovo utente\n");
    printf("2)  login --> Effettua il login\n");
    printf("********************************************************************\n\n");
}

/* Funzione che mostra a video il nome della stanza*/
void nome_stanza(uint8_t room){
    switch (room)
    {
        case 1:
            printf("*                      La prigione dei pirati                      *\n");
            break;
        case 2:
            printf("*                             Sherlock                             *\n");
            break;
        default:
            // nuove stanze
            break;
    }
}

/* Funzione che stampa la descrizione iniziale della stanza room*/
void descrizione_stanza(uint8_t room){
    switch (room)
    {
        case 1:
            printf("Sei stato catturato dal perfido pirata Barbanera e rinchiuso nella sua prigione. Hai poco tempo prima che il feroce capitano ti getti in pasto agli squali. Apri le sbarre e torna a solcare i sette mari in cerca di enormi tesori!\n\n");
            break;
        case 2:
            printf("Sei Sherlock Holmes, il più grande investigatore di tutti i tempi. Una donna è morta in Baker Street lasciando una lettera d'addio, ma c'è qualcosa di strano: indaga, raccogli le prove e scopri il nome dell'assassino\n\n");
            break;
        default:
            // nuove stanze
            break;
    }
}

/* Funzione che mostra a video la lista delle stanze*/
void mostra_stanze(){
    printf("****************************** STANZE ******************************\n\n");
    printf("Seleziona una stanza:\n\n");
    printf("1)  La prigione dei pirati\n");
    printf("2)  Sherlock\n\n");
    printf("Digita il comando:\n");
    printf("start <room> --> Avvia la stanza selezionata\n");
    printf("********************************************************************\n\n");
}

/* Funzione che mostra a video i comandi disponibili per il client*/
void mostra_comandi_client() {
    printf("***************************** COMANDI ******************************\n");
    printf("Digita un comando:\n\n");
    printf("1)  look [location | object]\n");
    printf("2)  take <object>\n");
    printf("3)  use <object1> [object2]\n");
    printf("4)  objs\n");
    printf("5)  notes [add]\n");
    printf("6)  end\n");
    printf("********************************************************************\n\n");
}

/* Funzione che stampa l'output della look senza argomenti*/
void mostra_look(uint8_t room){
    switch (room)
    {
        case 1:
            printf("Sei nella prigione della nave di Barbanera, delle imponenti ++Sbarre++ chiuse da due **Serrature** ti separano dalla libertà. In mezzo alla parete c'è un ++Tavolino++ con numerosi oggetti. In un angolo giacciono i resti del vecchio inquilino della prigione: lo ++Scheletro++ tiene in mano qualcosa di interessante.\n\n");
            break;
        case 2:
            printf("La vittima è morta nella sua camera, il corpo è sul ++Letto++. Appoggi sul ++Comodino++ la tua macchina da scrivere **Underwood**, con cui devi redigere il rapporto. Sul ++Pavimento++ c'è qualche traccia di sangue.\n\n");
            break;
        default:
            // nuove stanze
            break;
    }
}