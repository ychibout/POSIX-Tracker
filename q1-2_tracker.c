#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

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
        
        /*PARTIE 2 : création du tableau client <-> hash*/
        
        char*** tab;
        int tab_length = 0;

        /*PARTIE 3 : socket de reception de données*/

        int sockfd; // descripteur
        char buf[2048]; // buffer
        socklen_t addrlen; // socket

        struct sockaddr_in6 tracker_addr; // adresse ipv6
        struct sockaddr_in6 client;	// adresse ipv4 client

        // socket à créer
        if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
        {
            perror("socket fail\n");
			exit(EXIT_FAILURE);
        }

        // initialisation de l'adresse locale
        tracker_addr.sin6_family      = AF_INET6; // domaine de l'adresse
        tracker_addr.sin6_port        = htons(atoi(argv[2])); // le port
        tracker_addr.sin6_addr        = in6addr_any; // on ecoute sur n'importe quel adresse ( anycast )
        addrlen                  = sizeof(struct sockaddr_in6); // longueur de l'adresse

        // association de la socket avec l'adresse
        if( bind(sockfd, (struct sockaddr *) &tracker_addr, addrlen) == -1) // adresse
        {
			perror("bind fail\n");
			close(sockfd);
			exit(EXIT_FAILURE);
        }

        // reception de la chaine de caracteres
        while (1)
        {
            memset(buf,'\0',2048);
            
            if (recvfrom(sockfd, buf , 2048, 0, (struct sockaddr *) &client, &addrlen ) == -1)
            {
				perror("recvfrom fail \n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            
            //parsing du message
            
            char msg_type[3];				
			char msg_length[4];
			char hash_type[2];
			char hash_length[4];
			char hash[64];	
			char client_type[2];
			char client_size[4];
			char client_port[4];
			char client_addresse[128];
            
            sscanf(buf,"%3s%4s%2s%4s%64s%2s%4s%4s%128s",msg_type,msg_length,hash_type,hash_length,hash,client_type,client_size,client_port,client_addresse);
            
            switch (atoi(msg_type))
            {
				case 110:	//cas d'un PUT
					//Allocation mémoire pour la nouvelle entrée
					tab_length++;
					tab = realloc(tab, tab_length*sizeof(char**));
					tab[tab_length-1] = realloc(tab, 3*sizeof(char*));
					
					//stockage adresse client
					tab[tab_length-1][0] = malloc(128*sizeof(char));
					tab[tab_length-1][0] = client_addresse;
					
					//stockage port client
					tab[tab_length-1][1] = malloc(4*sizeof(char));
					tab[tab_length-1][1] = client_port;
					
					//stockage hash
					tab[tab_length-1][2] = malloc(64*sizeof(char));
					tab[tab_length-1][2] = hash;
			}
        }

        close(sockfd);

        return 0;
}
