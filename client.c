#include "include.h"
#include "util.h"
#include "cmd_client.h"

int main(int argc, char *argv[]) {
    int sd, i = 1, ret;

    struct sockaddr_in srvaddr;
    uint16_t porta = 4242;

    char comando[WORDLEN], arg1[WORDLEN], arg2[WORDLEN], buffer[BUFLEN];
    int room = 0;

    // non controllo la porta

    sd = crea_addr_server(&srvaddr, htons(porta));

    // connessione al server
    while (true)
    {
        // tentativo di connessione
        ret = connect(sd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
        if (ret == -1)
        {
            printf("Tentativo di connessione al server fallito\n");
            sleep(i++);
        }
        else
        {
            printf("Connessione al server sulla porta: %hu\n\n", porta);
            break;
        }
    }

    // gestione di signup/login
    accesso(sd);

    while (true) { // ciclo di gioco
        // gestione start <room>
        room = cmd_start(sd);
        mostra_comandi_client();
        // gestione partita
        while (true) {
            // leggo comando cmd <arg1> <arg2>
            fgets(buffer, BUFLEN, stdin);
            ret = sscanf(buffer, "%19s %19s %19s", comando, arg1, arg2);

            if (ret < 1) {
                printf("Comando non valido, riprova\n\n");
                mostra_comandi_client();
                continue;
            }

            if (!strcmp(comando, "look")) {
                if (ret == 1) // look: la gestisco lato client
                    mostra_look(room);
                else if (cmd_look(sd, arg1)) // se la partita è finita
                    break;
            }
            else if (!strcmp(comando, "take")) {
                if (ret != 2) {
                    printf("Comando non valido, la sintassi giusta è:\n");
                    printf("take <object>\n\n");
                    continue;
                }

                if (cmd_take(sd, arg1))
                    break;
            }
            else if (!strcmp(comando, "use")) {
                if (ret == 1) {
                    printf("Comando non valido, la sintassi giusta è:\n");
                    printf("use <object1> [object2]\n\n");
                    continue;
                }

                if (cmd_use(sd, ret, arg1, arg2))
                    break;
            }
            else if (!strcmp(comando, "objs")) {
                if (cmd_objs(sd))
                    break;
            }
            else if (!strcmp(comando, "notes")) {
                if (ret == 1) { // notes
                    if (cmd_notes(sd, false))
                        break;
                }
                else if (ret == 2 && !strcmp(arg1, "add")) {
                    if (cmd_notes(sd, true))
                        break;
                }
                else {
                    printf("Comando non valido, la sintassi giusta è:\n");
                    printf("notes [add]\n\n");
                    continue;
                }
            }
            else if (!strcmp(comando, "end")) {
                // invio end
                ret = send(sd, comando, CMDLEN, 0);
                if (ret == -1) {
                    perror("Errore invio comando end");
                    exit(1);
                }

                printf("\nUscito correttamente dall'Escape Room. A presto!\n\n");
                close(sd);
                exit(0);
            }
            else {
                printf("Comando non valido, riprova\n\n");
                mostra_comandi_client();
                continue;
            }
        }
    }
}




    