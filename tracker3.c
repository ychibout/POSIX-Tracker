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
        char buf[2048]; // buffer
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
        memset(buf,'\0',2048);

        // association de la socket avec l'adresse
        if( bind(sockfd, (struct sockaddr *) &my_addr, addrlen) == -1) // adresse
        {
          perror("bind fail\n");
          close(sockfd);
          exit(EXIT_FAILURE);
        }

        char*** tab;
        int taille_tab = 0;

        // reception de la chaine de caracteres
        while (1)
        {


                memset(buf,'\0',2048);
                if(recvfrom(sockfd, buf , 2048, 0, (struct sockaddr *) &client, &addrlen ) == -1)
                {
                  perror("recvfrom fail \n");
                  close(sockfd);
                  exit(EXIT_FAILURE);
                }

                printf("%s\n", buf);


		char msg_type[3];
		char msg_length[3];

		char hash_type[2];
		char hash_length[2];
		char hash[9];

		char client_type[2];
		char client_size[2];
		char client_port[5];
		char client_addresse[128];


		sscanf(buf,"%3s%3s%2s%2s%9s%2s%2s%5s%128s",msg_type,msg_length,hash_type,hash_length,hash,client_type,client_size,client_port,client_addresse);

        taille_tab++;
        tab = (char***) realloc (tab, taille_tab*sizeof(char**));
        tab[taille_tab-1] = (char**) malloc(3*sizeof(char*));
        tab[taille_tab-1][0] = (char*) malloc(9*sizeof(char));
        tab[taille_tab-1][1] = (char*) malloc(128*sizeof(char));
        tab[taille_tab-1][2] = (char*) malloc(5*sizeof(char));

        tab[taille_tab-1][0] = hash;
        tab[taille_tab-1][1] = client_addresse;
        tab[taille_tab-1][2] = client_port;

        //printf("test : %s, %s, %s\n", tab[taille_tab-1][0], tab[taille_tab-1][1], tab[taille_tab-1][2]);

		//printf("msg_type : %d \n",atoi(msg_type));
		/*printf("msg_length : %d \n",atoi(msg_length));
		printf("msg_type : %d \n",atoi(hash_type));
		printf("msg_type : %d \n",atoi(hash_length));*/

		//printf("msg_length : %d \n",atoi(msg_length));
		//printf("client_addresse : %s \n",client_addresse);
    printf("client_port : %d \n",atoi(client_port));

    // ENVOIE DU ACK

    if ( atoi(msg_type) == 110) // ACK PUT
    {
      int sockfd2;
    socklen_t addrlen2;
    struct sockaddr_in6 dest;

    // socket factory
    if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail \n");
	exit(EXIT_FAILURE);
    }

    // init remote addr structure and other params
    dest.sin6_family = AF_INET6;
    dest.sin6_port   = htons(atoi(client_port));
    addrlen2         = sizeof(struct sockaddr_in6);

    if(inet_pton(AF_INET6, argv[1], &dest.sin6_addr) != 1)
    {
        perror("inet fail  \n");
	close(sockfd2);
	exit(EXIT_FAILURE);
    }

            int taille_hash = strlen(hash);
      			int taille_base = (1+9+18)*8;
      			int taille_final = taille_hash + taille_base +128;
                        int taille_final_moins_un = taille_final--;
			//printf("taille hash : %d \n", taille_hash);
			//printf("taille finale : %d \n", taille_final);

      			char *buf2;
                        buf2 = (char*)malloc(taille_final*sizeof(char));

    			struct sockaddr_in6 src;
                        char *str =(char*)malloc(128*sizeof(char));
                        strcpy(str, "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001");
      			if(inet_pton(AF_INET6, "0000:0000:0000:0000:0000:0000:0000:0001", &src.sin6_addr) != 1)
      			{
        				perror("inet fail  \n");
								close(sockfd2);
								exit(EXIT_FAILURE);
    			}

                        //inet_ntop(AF_INET6, &(src.sin6_addr), str, INET6_ADDRSTRLEN);

      			//printf("hash : %s \n",argv[5]);
            //printf("port : %s \n",argv[2]);
            //printf("ipv6: %s \n",str);
	           snprintf(buf2, taille_final, "111%d5064%s5518%s%s", taille_final_moins_un, hash, client_port,str);

      			//printf("%s \n",buf); <- affichage du message qu'on envoie

                        if (sendto(sockfd2, buf2, strlen(buf2), 0, (struct sockaddr *) &dest, addrlen2) == -1)
                        {
                                perror("send to fail \n");
                                close(sockfd);
                                exit(EXIT_FAILURE);
                        }
                        printf("ACK PUT envoyé \n");

            // 110  taille_final-1 50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]


    }
    // FIN DU ACK

    if ( atoi(msg_type) == 112) // ACK GET
    {
      int sockfd2;
    socklen_t addrlen2;
    struct sockaddr_in6 dest;

    // socket factory
    if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail \n");
    exit(EXIT_FAILURE);
    }

    // init remote addr structure and other params
    dest.sin6_family = AF_INET6;
    dest.sin6_port   = htons(atoi(client_port));
    addrlen2         = sizeof(struct sockaddr_in6);

    if(inet_pton(AF_INET6, argv[1], &dest.sin6_addr) != 1)
    {
        perror("inet fail  \n");
    close(sockfd2);
    exit(EXIT_FAILURE);
    }

            int taille_hash = strlen(hash);
            int taille_base = (1+9+18)*8;
            int taille_final = taille_hash + taille_base +128;
            int taille_final_moins_un = taille_final--;
            //printf("taille hash : %d \n", taille_hash);
            //printf("taille finale : %d \n", taille_final);

            char** liste_clients;
            int taille_liste = 0;
            int taille_client = 2+2+5+128;
            int y;

            for (y = 0; y < taille_tab; y++)
            {
                if (strcmp(tab[y][0], hash) == 0)
                {
                    taille_liste++;

                    liste_clients = (char**)realloc(liste_clients, taille_liste*sizeof(char*));
                    liste_clients[taille_liste-1] = (char*)malloc(2+2+5+128*sizeof(char));
                    snprintf(liste_clients[taille_liste-1],taille_client,"5518%5s%128s",tab[y][2],tab[y][1]);
                }
            }

                char *buf2;
                        buf2 = (char*)malloc(taille_final*sizeof(char));

                struct sockaddr_in6 src;
                        char *str =(char*)malloc(128*sizeof(char));
                        strcpy(str, "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001");
                if(inet_pton(AF_INET6, "0000:0000:0000:0000:0000:0000:0000:0001", &src.sin6_addr) != 1)
                {
                        perror("inet fail  \n");
                                close(sockfd2);
                                exit(EXIT_FAILURE);
                }

               snprintf(buf2, taille_final, "113%d5064%s%s", taille_final_moins_un, hash,liste_clients[taille_liste-1]);

                //printf("%s \n",buf); <- affichage du message qu'on envoie

                        if (sendto(sockfd2, buf2, strlen(buf2), 0, (struct sockaddr *) &dest, addrlen2) == -1)
                        {
                                perror("send to fail \n");
                                close(sockfd);
                                exit(EXIT_FAILURE);
                        }
                        printf("ACK GET envoyé \n");

            // 110  taille_final-1 50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]


    }




        }

        // print the received char

        // close the socket
        close(sockfd);

        return 0;
}
