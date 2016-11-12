/**
* @file sender-udp.c
* @author Julien Montavont
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* Simple program that creates an IPv4 UDP socket and sends a string
* to a remote host. The string, IPv4 addr and port number of the
* remote host are passed as command line parameters as follow:
* ./pg_name IPv4_addr port_number string
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

struct s_message {
    char msg_type;
    short int msg_length;
    char hash_type;
    short int hash_length;
    char hash[64];
    char client_type;
    short int client_length;
    short int client_port;
    struct in6_addr client;
};


int main(int argc, char **argv)
{
    int sockfd;
    socklen_t addrlen;
    struct sockaddr_in6 dest;

    // socket factory
    if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail \n");
        exit(EXIT_FAILURE);
    }

    // init remote addr structure and other params
    dest.sin6_family = AF_INET6;
    dest.sin6_port   = htons(atoi(argv[2]));
    addrlen         = sizeof(struct sockaddr_in6);

    if(inet_pton(AF_INET6, argv[1], &dest.sin6_addr) != 1)
    {
        perror("inet fail  \n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("%ld\n", sizeof(dest.sin6_family));

    struct s_message msg1;
    if (strcmp(argv[4], "put") == 0)
    {
        msg1.msg_type = 110;
    }
    if (strcmp(argv[4], "get") == 0)
    {
        msg1.msg_type = 112;
    }
    msg1.hash_type = 50;
    msg1.hash_length = 64;
    strcpy(msg1.hash, argv[5]);
    msg1.client_type = 55;
    msg1.client_length = 18;
    msg1.client_port = atoi(argv[3]);
    msg1.client = dest.sin6_addr;
    msg1.msg_length = msg1.hash_length + msg1.client_length;

    printf("%ld\n", sizeof(struct in6_addr));

    if (sendto(sockfd, &msg1, sizeof(msg1), 0, (struct sockaddr *) &dest, addrlen) == -1)
    {
        perror("send to fail \n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    //Attente du ACK_PUT

    int sockfd2; // descripteur
    socklen_t addrlen2; // socket

    struct sockaddr_in6 my_addr; // adresse ipv4
    struct sockaddr_in6 client;	// adresse ipv4 client

    // socket à créer
    if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail\n");
        exit(EXIT_FAILURE);
    }

    // initialisation de l'adresse locale
    my_addr.sin6_family      = AF_INET6; // domaine de l'adresse
    my_addr.sin6_port        = htons(atoi(argv[3])); // le port
    my_addr.sin6_addr        = in6addr_any; // on ecoute sur n'importe quel adresse ( anycast )
    addrlen2                  = sizeof(struct sockaddr_in6); // longueur de l'adresse

    printf("listening on %d\n", atoi(argv[3]));

    // association de la socket avec l'adresse
    if( bind(sockfd2, (struct sockaddr *) &my_addr, addrlen2) == -1) // adresse
    {
        perror("bind fail\n");
        close(sockfd2);
        exit(EXIT_FAILURE);
    }

    struct s_message ack;

    // reception de la chaine de caracteres
    if(recvfrom(sockfd2, &ack , sizeof(ack), 0, (struct sockaddr *) &client, &addrlen2 ) == -1)
    {
        perror("recvfrom fail \n");
        close(sockfd2);
        exit(EXIT_FAILURE);
    }

    if (ack.msg_type == 111)
    {
        printf("ACK PUT RECU\n");
    }

    if (ack.msg_type == 113)
    {
        char* addr_client = (char*) malloc (INET6_ADDRSTRLEN*sizeof(char));
        inet_ntop(AF_INET6, &(msg1.client), addr_client, INET6_ADDRSTRLEN);
        printf("ACK GET RECU\n addr : %s, port : %d\n", addr_client, ack.client_port);
    }

    close(sockfd);
    return 0;
}
