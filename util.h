#ifndef UTIL_H
    #define UTIL_H
    #include "include.h"
    #include "strutture.h"

    void mostra_comandi_server();
    int crea_addr_server(struct sockaddr_in *my_addr, uint16_t porta);
    int crea_socket_server(struct sockaddr_in *my_addr, uint16_t porta);

    void mostra_accesso();
    void nome_stanza(uint8_t room);
    void descrizione_stanza(uint8_t room);
    void mostra_stanze();
    void mostra_comandi_client();
    void mostra_look(uint8_t room);
    
#endif