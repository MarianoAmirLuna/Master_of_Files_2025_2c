#ifndef MODULES_SOCKETS_NETWORK_H
#define MODULES_SOCKETS_NETWORK_H

#include <stdlib.h>

#ifndef UTILS_STRUCTS_H
#include "../../utils/structs.h"
#endif

#ifndef LIBS_PACKET_H
#include "../../libs/packet.h"
#endif

#ifndef LIBS_LOGGER_H
#include "../../libs/logger.h"
#endif

#ifndef MODULES_SOCKETS_SERVER_H
#include "server.h"
#endif
#ifndef MODULES_SOCKETS_CLIENT_H
#include "client.h"
#endif

#include <commons/collections/list.h>
#include <execinfo.h>
#include <stdio.h>

/// @brief Escribe el buffer dentro del t_paquete, el SZ es un argumento de referencia para conocer el tamaño
/// @param packet Puntero de paquete
/// @param buffer Buffer de dato
/// @param sz Tamaño resultante del buffer del paquete
/// @return Paquete serializado ya preparado para utilizar el método send
void* write_buffer_and_serialize(t_packet* packet, char* buffer, int* sz){
    packet->buffer->size = strlen(buffer)+1;
    packet->buffer->stream= malloc(packet->buffer->size);
    memcpy(packet->buffer->stream, buffer, packet->buffer->size);
    int bytes = packet->buffer->size + 2*sizeof(int);
    (*sz) = bytes;
    return serialize_packet(packet, bytes);
}

int packet_bytes_size(t_packet* packet){
	return packet->buffer->size + 2*sizeof(int);
}

/// @brief Envia el paquete
/// @param packet Paquete
/// @param sock Socket
void send_packet(t_packet* packet, int sock){
    int bytes = packet->buffer->size + 2*sizeof(int);

    void* tosend = serialize_packet(packet, bytes);
	log_trace(logger, "Cantidad de bytes enviados: %d al socket %d", bytes, sock);
    if(send(sock, tosend,bytes, 0) == -1)
    {
        if(logger == NULL){
            printf("No se pudo enviar el paquete");
        }
        else{
            log_error(logger, "No se pudo enviar el paquete");
        }
    }
    free(tosend);
}

void send_and_free_packet(t_packet* packet, int sock){
	send_packet(packet, sock);
	free_packet(packet);
}

void send_basic_packet(int sock_client, int value){
    t_packet* p = create_packet();
    add_int_to_packet(p, value);
    send_and_free_packet(p, sock_client);
}

void send_basic_buffer_packet(int sock_client, void* data, int len){
    t_packet* p = create_packet();
    add_packet(p, data, len);
    send_and_free_packet(p, sock_client);
}

/*
void response_to_sock(int sock, response resp){
    t_packet* p = create_packet();
    add_int_to_packet(p, resp);
    send_and_free_packet(p, sock);
}*/

/// @brief Envia al socket el identificador de módulo
/// @param ocm 
/// @param sock 
void send_john_snow_packet(op_code_module ocm, int sock){
	t_packet* packet = create_packet();
	add_int_to_packet(packet, ocm);
	send_and_free_packet(packet, sock);
}

void send_opcode(op_code oc, int sock){
	t_packet* packet = create_packet();
	add_int_to_packet(packet, oc);
	send_and_free_packet(packet, sock);
}

void print_trace(){
	void* array[10];
	char ** strings;
	int size, i;
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if(strings != NULL){
		log_debug(logger, "Obtained %d stack frames.", size);
		for(i=0;i<size;i++){
			log_debug(logger, "->%s", strings[i]);
		}
		free(strings);
	}else{
		log_error(logger, "No pudo obtener backtrace :C %s:%d", __func__, __LINE__);
	}
}

/// @brief [BLOQUEANTE] Recibe la operación de un socket
/// @param socket_cliente Socket
/// @return Retorna el valor de la operación
int recv_operation(int sock_client){
    log_debug(logger, "Esperando operacion al socket %d", sock_client);
    int cod_op=-1;
	//print_trace();
    if(recv(sock_client, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    close(sock_client);
    return cod_op;
}



/// @brief [BLOQUEANTE] Recibe el buffe
/// @param size Tamaño del buffer
/// @param socket_cliente Socket
/// @return Buffer
void* recv_buffer(int* size, int sock_client){
    void* buffer;
    recv(sock_client, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(sock_client, buffer, *size, MSG_WAITALL);
    return buffer;
}

/// @brief [BLOQUEANTE] Recibe mensaje de un socket (esto sólo imprime, no recibe o maneja un char*)
/// @param socket_cliente Socket
void recv_message(int sock_client){
    int size = 0;
    void* buffer = recv_buffer(&size, sock_client);
    log_info(logger, "Me llego el mensaje: %s", buffer_to_string(buffer));
    free(buffer);
}

/// @brief [BLOQUEANTE] Recibe paquetes de un socket
/// @param socket_cliente 
/// @return 
t_list* recv_packet(int sock_client){
    int size_data= 0, sz=0,offset=0;
    t_list* values = list_create();
    void* buffer = recv_buffer(&size_data, sock_client);
	log_trace(logger, "Recibi %d cantidad de bytes del cliente: %d", size_data, sock_client);
    while(offset < size_data){
        memcpy(&sz, buffer+offset, sizeof(int));
        offset+=sizeof(int);
        void* value = malloc(sz);
        memcpy(value, buffer+offset, sz);
        offset+=sz;
        list_add(values, value);
    }
    free(buffer);
    return values;
}

t_list* recv_operation_packet(int sock_client){
	int op = recv_operation(sock_client);
	//if(op != PACKET && op!= IDENTIFICARSE_DISPATCH_MEMORIA && op != IDENTIFICARSE_DISPATCH_KERNEL){ //nunca pense que iba a odiar tanto una linea especifica
	if(op != PACKET){ //nunca pense que iba a odiar tanto una linea especifica
		log_error(logger, "Operacion invalida valor(%d) cliente %d, %s:%d se invocó exit(1)", op, sock_client, __func__, __LINE__); 
		exit(1);
	}
	return recv_packet(sock_client);
}

/// @brief [BLOQUEANTE en lado servidor] Hace un Handshake con un socket
/// @param sock Socket a hacer handshake
/// @param is_server_side Es lado Servidor o cliente? 1: Servidor, 0: Cliente
/// @return Error si es distinto de 0, Correcto si es igual a 0
int handshake(int sock, int is_server_side){
    int handshake=1,result=0,resultOk=0, resultError=-1;
    if(is_server_side){
		recv(sock, &handshake, sizeof(int), MSG_WAITALL);
		if(handshake == 1)
		{
			send(sock, &resultOk, sizeof(int), 0);
		}
		else{
			send(sock, &resultError, sizeof(int), 0);
			result = resultError;
		}
	}
	else{
		send(sock, &handshake, sizeof(int), 0);
		recv(sock, &result, sizeof(int), MSG_WAITALL);
	}
	if(result == 0){
		if(logger == NULL)
		{
			printf("HANDSHAKE CON SERVIDOR %d\n", sock);
		}else{
			log_info(logger,"HANDSHAKE CON SERVIDOR %d", sock);
		}
	}
	else{
		if(logger == NULL)
		{
			perror("Error handshake");
			printf("HANDSHAKE CON SERVIDOR %d", sock);
		}else{
			log_error(logger,"ERROR HANDSHAKE EN %d", sock);
		}
	}
	return result;
}


/// @brief Manejador de recibos/envios de distintas operaciones
/// @param sock Socket Socket
/// @param cbPack Callback Packet (void**)
/// @param argsPack Argumentos del Callback Packet
void loop_network(int sock, void(*cbPack)(void*), void* argsPack)
{
	int cod_op = PACKET;
	for(;;){
		cod_op = recv_operation(sock);
        if(cod_op == -1)
		{
			//print_trace();
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			int cl = close(sock);
            log_info(logger, "Cerrando Socket %d (%s:%d)", cl, __func__, __LINE__);
			break;
		}
		if(cod_op == PACKET)
		{
			(*cbPack)(argsPack);
			continue;
		}
	}
}


#endif