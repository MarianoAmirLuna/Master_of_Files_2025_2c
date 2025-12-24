#ifndef MODULES_SOCKETS_CLIENT_H
#define MODULES_SOCKETS_CLIENT_H

#ifndef INC_SOCKETS_H
#include "../../inc/sockets.h"
#endif
#include "commons/string.h"

#include "exts/error_ext.h"

/// @brief Conexión AL SERVIDOR
/// @param ip IP del Servidor a conectarse
/// @param port Puerto del Servidor a Conectarse
/// @return -1 si es error caso contrario el Socket
int client_connection(char* ip, int port){
    struct addrinfo hints;
    struct addrinfo *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    char* p = string_itoa(port);
    getaddrinfo(ip, p, &hints, &server_info);
    free(p);
    int sock =socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

    if(sock < 0){
        freeaddrinfo(server_info);
        return print_error("socket");
    }
    if(connect(sock, server_info->ai_addr, server_info->ai_addrlen) <0){
        freeaddrinfo(server_info);
        return print_error("connect");
    }
    freeaddrinfo(server_info);
    return sock;
}

/// @brief Cerrar conexión
/// @param sock Socket
/// @return 0: Success, Error > 0
int client_close (int sock){
    return close(sock);
}

#endif