/**
 * @file receiver-udp.c
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
 * Simple program that creates an IPv4 UDP socket and waits for the
 * reception of a string. The program takes a single parameter which
 * is the local communication port. The IPv4 addr associated to the
 * socket will be all available addr on the host (use INADDR_ANY
 * maccro).
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


int main(int argc, char **argv)
{
    int sockfd; // descripteur
    char buf[1024]; // buffer
    socklen_t addrlen; // socket

    struct sockaddr_in6 my_addr; // adresse ipv4
    struct sockaddr_in6 client;	// adresse ipv4 client

    // check the number of args on command line
    if(argc != 2)
    {
        printf("Usage: %s local_port\n", argv[0]);
	exit(-1);
    }

    // socket à créer
    if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail\n");
	exit(EXIT_FAILURE);
    }

    // initialisation de l'adresse locale
    my_addr.sin6_family      = AF_INET6; // domaine de l'adresse
    my_addr.sin6_port        = htons(atoi(argv[1])); // le port
    my_addr.sin6_addr        = in6addr_any; // on ecoute sur n'importe quel adresse ( anycast )
    addrlen                  = sizeof(struct sockaddr_in6); // longueur de l'adresse
    memset(buf,'\0',1024);

    // association de la socket avec l'adresse
    if( bind(sockfd, (struct sockaddr *) &my_addr, addrlen) == -1) // adresse
    {
      perror("bind fail\n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    // reception de la chaine de caracteres
    if(recvfrom(sockfd, buf , 1024, 0, (struct sockaddr *) &client, &addrlen ) == -1)
    {
      perror("recvfrom fail \n");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    // print the received char
    printf("%s\n", buf);

    // close the socket
    close(sockfd);

    return 0;
}
