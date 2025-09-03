#ifndef MODULES_SOCKETS_SERVER_H
#define MODULES_SOCKETS_SERVER_H

#ifndef EXTS_ERROR_H
#include "../../exts/error_ext.h"
#endif

#ifndef INC_SOCKETS_H
#include "../../inc/sockets.h"
#endif
#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// @brief Levanta/Crea un servidor
/// @param port Puerto del servidor
/// @return Socket del Servidor
int server_connection(int port){
    int sock_serv=-1;
    struct addrinfo hints, *servinfo;
    int opt = 1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    char* pp = string_itoa(port);
    getaddrinfo(NULL, pp, &hints, &servinfo);
    free(pp);
    sock_serv = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock_serv < 0)
        return print_error("socket");
    if(setsockopt(sock_serv, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt)) < 0)
        return print_error("setsock");
    if(bind(sock_serv, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        return print_error("bind");
    if(listen(sock_serv, SOMAXCONN) < 0)
        return print_error("listen");
    freeaddrinfo(servinfo);
    return sock_serv;
}

/// @brief [BLOQUEANTE] Espera la conexión/entrada de un cliente
/// @param sock Socket 
/// @param port Puerto
/// @return El socket del cliente que entró
int wait_client(int sock, int port){
    int sock_client = 0;
    struct addrinfo hints, *clientaddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE;
    char* pp = string_itoa(port);
    getaddrinfo(NULL, pp, &hints, &clientaddr);
    free(pp);
    sock_client = accept(sock, NULL,NULL);
    //free(hints);
    free(clientaddr);
    if(sock_client == -1)
        return print_error("wait client");
    return sock_client;
}

#endif