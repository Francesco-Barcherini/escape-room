#include "include.h"
#include "strutture.h"
#include "util.h"
#include "cmd_server.h"

int main(int argc, char *argv[]){
    int listener, newsd, ret;

    char buffer[BUFLEN], comando[CMDLEN];

    struct sockaddr_in servaddr, cliaddr;
    socklen_t addrlen = sizeof(cliaddr);
    uint16_t port, cmdport;

    fd_set master, readfds;
    int i, j, fdmax;

    struct Partita stanze[NSTANZE]; 
    struct Account *accounts = NULL;
    struct Account *socketToAccount[MAXCLIENT+4]; // +4 per stdin, stdout, stderr e listener

    bool running = false, stopped = false;

    enum EsitoPartita esito;

    int room;

    // non controllo ./server <porta> perché dipende dalla start

    while (true) // ciclo di start&stop del server
    {
        // inizializzo i dati
        memset(stanze, 0, sizeof(stanze));
        memset(socketToAccount, 0, sizeof(socketToAccount));
        stopped = false;

        // inizializzo set dei file descriptor
        FD_ZERO(&master);
        FD_ZERO(&readfds);

        // aggiungo stdin
        FD_SET(STDIN_FILENO, &master);
        fdmax = STDIN_FILENO;

        printf("Server inizializzato\n\n");
        
        // guida dei comandi
        mostra_comandi_server();

        while (true) // ciclo della select
        {
            // copio il set dei file descriptor
            readfds = master;

            // aspetto che qualche fd sia pronto
            ret = select(fdmax+1, &readfds, NULL, NULL, NULL);
            if(ret == -1){
                perror("Errore nella select");
                exit(1);
            }

            // ciclo sui file descriptor
            for(i = 0; i <= fdmax; i++){
                if(FD_ISSET(i, &readfds)){
                    // se è stdin
                    if(i == STDIN_FILENO){
                        // leggo il comando
                        fgets(buffer, BUFLEN, stdin);
                        
                        // parsing del comando
                        ret = sscanf(buffer, "%s %hu", comando, &cmdport);

                        if (ret == 2 && !strcmp(comando, "start")) // start <port>
                        {
                            // controllo che il server non sia già in esecuzione
                            if(running){
                                printf("Il server è già in esecuzione sulla porta %hu\n\n", port);
                                continue;
                            }

                            // creo il socket
                            port = cmdport;
                            listener = crea_socket_server(&servaddr, htons(port));

                            // aggiungo il socket al set
                            FD_SET(listener, &master);
                            if(listener > fdmax)
                                fdmax = listener;

                            // metto il socket in ascolto
                            ret = listen(listener, MAXCLIENT);
                            if(ret == -1){
                                perror("Errore nella listen");
                                exit(1);
                            }
                            printf("Server in ascolto sulla porta %hu\n\n", port);

                            // setto il flag di running
                            running = true;
                        }
                        else if (ret == 1 && !strcmp(comando, "stop")) // stop
                        {
                            // controllo che il server sia in esecuzione
                            if(!running){
                                printf("Il server non è in esecuzione\n\n");
                                continue;
                            }

                            // se c'è almeno una partita attiva non posso chiudere il server
                            for(j = 0; j < NSTANZE; j++){
                                // una partita è attiva se c'è almeno un giocatore connesso
                                if(stanze[j].connessi > 0){
                                    printf("C'è una partita in corso nella room %d, impossibile chiudere il server\n", j+1);
                                    break;
                                }
                            }
                            if (j < NSTANZE) // se c'è una partita attiva
                                continue;

                            // posso chiudere il server
                            // chiudo tutte le connessioni
                            for(j = 3; j <= fdmax; j++){
                                if(FD_ISSET(j, &master)) {
                                    // chiudo j
                                    close(j);

                                    // aggiorno logged e room
                                    if (socketToAccount[j] != NULL) {
                                        socketToAccount[j]->logged = false;
                                        socketToAccount[j]->room = -1;
                                    }

                                    printf("Chiuso socket %d\n", j);
                                }
                            }

                            // setto il flag di running
                            running = false;
                            stopped = true;

                            printf("Server chiuso correttamente\n\n");

                            break;
                        }
                        else // comando sbagliato
                        {
                            printf("Comando non valido\n\n");
                            mostra_comandi_server();
                        }
                    }
                    else if(i == listener){ // se è il listener
                        // accetto la connessione
                        newsd = accept(listener, (struct sockaddr*)&cliaddr, &addrlen);
                        if(newsd == -1){
                            perror("Errore nella accept");
                            exit(1);
                        }
                        printf("Nuova connessione accettata sulla porta %hu: creato socket %d\n", port, newsd);

                        // aggiungo il socket al set
                        FD_SET(newsd, &master);
                        if(newsd > fdmax)
                            fdmax = newsd;
                    }
                    else{ // se è un client
                        // ricevo il comando
                        ret = recv(i, comando, sizeof(comando), 0);
                        if(ret == -1){
                            perror("Errore nella ricezione del comando");
                            continue;
                        }
                        if(ret == 0){ // se il client ha chiuso la connessione
                            // chiudo il socket
                            close(i);
                            FD_CLR(i, &master);
                            printf("Chiuso socket %d\n", i);

                            if (socketToAccount[i]) {// se il client è loggato
                                logout_account(socketToAccount[i], stanze);
                                socketToAccount[i] = NULL;
                            }
                            continue;
                        }
                        // il client ha inviato un comando

                        if (!strcmp(comando, "sign")) // sign <username> <password>
                            cmd_sign(i, &accounts);
                        else if (!strcmp(comando, "login"))  // login <username> <password>
                            socketToAccount[i] = cmd_login(i, accounts);
                        else if (!strcmp(comando, "start")) { // start <room>
                            if (cmd_start(i, socketToAccount[i], stanze)) // room assegnata -> invio le info sulla room
                                fine_comando(i, socketToAccount[i]->room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "look")) { // look <locazione/oggetto>
                            esito = invia_stato(i, socketToAccount[i], accounts, stanze);
                            if (esito != incorso)
                                continue;
                            cmd_look(i, stanze[socketToAccount[i]->room]);
                            fine_comando(i, socketToAccount[i]->room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "take")) { // take <object>
                            esito = invia_stato(i, socketToAccount[i], accounts, stanze);
                            if (esito != incorso)
                                continue;
                            room = socketToAccount[i]->room;
                            if (cmd_take(i, &stanze[room], accounts)) { // client disconnesso
                                // chiudo il socket
                                close(i);
                                FD_CLR(i, &master);
                                printf("Chiuso socket %d\n", i);
                                logout_account(socketToAccount[i], stanze);
                                socketToAccount[i] = NULL;
                            }
                            fine_comando(i, room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "use")) { // use <object1> <object2>
                            esito = invia_stato(i, socketToAccount[i], accounts, stanze);
                            if (esito != incorso)
                                continue;
                            room = socketToAccount[i]->room;
                            if (cmd_use(i, &stanze[room], accounts)) { //chiusura socket
                                // chiudo il socket
                                close(i);
                                FD_CLR(i, &master);
                                printf("Chiuso socket %d\n", i);
                                logout_account(socketToAccount[i], stanze);
                                socketToAccount[i] = NULL;
                            }
                            fine_comando(i, room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "objs")) { // objs
                            esito = invia_stato(i, socketToAccount[i], accounts, stanze);
                            if (esito != incorso)
                                continue;
                            cmd_objs(i, stanze[socketToAccount[i]->room]);
                            fine_comando(i, room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "notes")) { // note
                            esito = invia_stato(i, socketToAccount[i], accounts, stanze);
                            if (esito != incorso)
                                continue;
                            cmd_notes(i, &stanze[socketToAccount[i]->room]);
                            fine_comando(i, room, socketToAccount[i], accounts, stanze);
                        }
                        else if (!strcmp(comando, "end")) { // end
                            logout_account(socketToAccount[i], stanze);
                            socketToAccount[i] = NULL;
                            close(i);
                            FD_CLR(i, &master);
                            printf("Chiuso socket %d\n", i);
                        }
                    }
                
                }
            }
            // se ho chiuso il server esco dal ciclo della select
            if(stopped)
                break;            
        }
        
    }
    
}


