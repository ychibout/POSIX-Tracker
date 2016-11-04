#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        //PARTIE 1 : résolution d'adresse d'argv[1]

        struct addrinfo hints, *res;
        int status;
        void *addr;
        char iptracker[INET6_ADDRSTRLEN];

        if (argc != 3)
        {
                fprintf(stderr, "Usage : %s <domainname> <port>\n", argv[0]);
                return 1;
        }

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
        {
                fprintf(stderr, "error gettaddrinfo");
                return 2;
        }

        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
        addr = &(ipv6->sin6_addr);

        inet_ntop(res->ai_family, addr, iptracker, sizeof iptracker);

        printf("listening on %s port %d\n", iptracker, atoi(argv[2]));

        /*PARTIE 2 : socket de reception de données*/

        int sockfd; // descripteur
        char buf[1024]; // buffer
        socklen_t addrlen; // socket

        struct sockaddr_in6 my_addr; // adresse ipv4
        struct sockaddr_in6 client;	// adresse ipv4 client

        // socket à créer
        if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
        {
            perror("socket fail\n");
    	exit(EXIT_FAILURE);
        }

        // initialisation de l'adresse locale
        my_addr.sin6_family      = AF_INET6; // domaine de l'adresse
        my_addr.sin6_port        = htons(atoi(argv[2])); // le port
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
        while (1)
        {
                memset(buf,'\0',1024);
                if(recvfrom(sockfd, buf , 1024, 0, (struct sockaddr *) &client, &addrlen ) == -1)
                {
                  perror("recvfrom fail \n");
                  close(sockfd);
                  exit(EXIT_FAILURE);
                }

                printf("%s\n", buf);

        }

        // print the received char

        // close the socket
        close(sockfd);

        return 0;
}
