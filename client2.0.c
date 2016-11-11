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

    if (strcmp(argv[4], "put") == 0)
    {
            int taille_hash = strlen(argv[5]);
      			int taille_base = (1+9+18)*8;
      			int taille_final = taille_hash + taille_base +128;
                        int taille_final_moins_un = taille_final--;
			//printf("taille hash : %d \n", taille_hash);
			//printf("taille finale : %d \n", taille_final);

      			char *buf;
                        buf = (char*)malloc(taille_final*sizeof(char));

    			struct sockaddr_in6 src;
                        char *str =(char*)malloc(128*sizeof(char));
                        strcpy(str, "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001");
      			if(inet_pton(AF_INET6, "0000:0000:0000:0000:0000:0000:0000:0001", &src.sin6_addr) != 1)
      			{
        				perror("inet fail  \n");
								close(sockfd);
								exit(EXIT_FAILURE);
    			}

                        //inet_ntop(AF_INET6, &(src.sin6_addr), str, INET6_ADDRSTRLEN);

      			//printf("hash : %s \n",argv[5]);
            //printf("port : %s \n",argv[2]);
            //printf("ipv6: %s \n",str);
             printf("110 - %d - 5064 - %s - 5518 - %s - addresse\n",taille_final_moins_un,argv[5],argv[3]);
	           snprintf(buf, taille_final, "110%d5064%s5518%s%s", taille_final_moins_un, argv[5], argv[3],str);

      			//printf("%s \n",buf); <- affichage du message qu'on envoie

                        if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &dest, addrlen) == -1)
                        {
                                perror("send to fail \n");
                                close(sockfd);
                                exit(EXIT_FAILURE);
                        }

            // 110  taille_final-1 50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]

    }
    
     // Ecoute d'un message du tracker
     
        int sockfd2; // descripteur
        char buf2[2048]; // buffer
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
        memset(buf2,'\0',2048);
        
        printf("listening on %d\n", atoi(argv[3]));

        // association de la socket avec l'adresse
        if( bind(sockfd2, (struct sockaddr *) &my_addr, addrlen2) == -1) // adresse
        {
          perror("bind fail\n");
          close(sockfd2);
          exit(EXIT_FAILURE);
        }

        // reception de la chaine de caracteres
                //memset(buf2,'\0',2048);
                if(recvfrom(sockfd2, buf2 , 2048, 0, (struct sockaddr *) &client, &addrlen2 ) == -1)
                {
                  perror("recvfrom fail \n");
                  close(sockfd2);
                  exit(EXIT_FAILURE);
                }
                printf("Retour ACK \n");
                printf("%s\n", buf2); // affichage du ack qu'on recoie

    // fin ecoute message

    // close the socket
    close(sockfd);

    return 0;
    
    
}

// ./sender-udp ::1 1024 Coucou

//Message put : [110] [1+2+(1+2+64)+(1+2+18)] [50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]
