#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

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
    /*PARTIE 1 : résolution d'adresse d'argv[1]*/

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
    socklen_t addrlen; // socket

    struct sockaddr_in6 my_addr; // adresse ipv6
    struct sockaddr_in6 client;	// adresse ipv6 client

    // socket à créer
    if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket fail\n");
        exit(EXIT_FAILURE);
    }

    // initialisation de l'adresse locale d'écoute
    my_addr.sin6_family      = AF_INET6; // domaine de l'adresse
    my_addr.sin6_port        = htons(atoi(argv[2])); // le port
    my_addr.sin6_addr        = in6addr_any; // on ecoute sur n'importe quel adresse ( anycast )
    addrlen                  = sizeof(struct sockaddr_in6); // longueur de l'adresse

    printf("taille : %ld\n", sizeof(iptracker));

    // association de la socket avec l'adresse
    if( bind(sockfd, (struct sockaddr *) &my_addr, addrlen) == -1) // adresse
    {
        perror("bind fail\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //tableau client <=> adresse <=> port
    char*** tab_hash;
    tab_hash = (char***) malloc(sizeof(char**));
    int taille_tab = 1;

    struct s_message msg1;  //buffer rempli par recvfrom

    // reception de la chaine de caracteres
    while (1)
    {
        if(recvfrom(sockfd, &msg1, sizeof(msg1), 0, (struct sockaddr *) &client, &addrlen ) == -1)
        {
            perror("recvfrom");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (msg1.msg_type == 110)   //cas d'un PUT
        {
            if (taille_tab != 1)
            {
                taille_tab++;
                tab_hash = (char***) realloc (tab_hash, taille_tab*sizeof(char**));
            }
            tab_hash[taille_tab-1] = (char**) malloc(3*sizeof(char*));
            tab_hash[taille_tab-1][0] =(char*) malloc(64*sizeof(char)); //hash
            tab_hash[taille_tab-1][1] = (char*)malloc(sizeof(struct in6_addr)*sizeof(char));    //adresse
            tab_hash[taille_tab-1][2] = (char*)malloc(5*sizeof(char));   //port

            char* addr_client = (char*) malloc (INET6_ADDRSTRLEN*sizeof(char));
            inet_ntop(AF_INET6, &(msg1.client), addr_client, INET6_ADDRSTRLEN); //pour stockage adresse dans tableau

            char* port_client = (char*) malloc(sizeof(short int));
            snprintf(port_client, 6, "%d", msg1.client_port);   //pour stockage port dans tableau

            tab_hash[taille_tab-1][0] = msg1.hash;
            tab_hash[taille_tab-1][1] = addr_client;
            tab_hash[taille_tab-1][2] = port_client;

            printf("hash : %s \n",tab_hash[taille_tab-1][0]);
            printf("adresse client : %s \n",tab_hash[taille_tab-1][1]);
            printf("port client : %s \n\n",tab_hash[taille_tab-1][2]);
            printf("port client2 : %d \n\n",msg1.client_port);


            //ACK PUT

            int sockfd2;
            socklen_t addrlen2;
            struct sockaddr_in6 dest;

            // socket de réponse au client
            if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
            {
                perror("socket fail \n");
                exit(EXIT_FAILURE);
            }

            dest.sin6_family = AF_INET6;
            dest.sin6_port   = htons(atoi(tab_hash[taille_tab-1][2]));
            addrlen2         = sizeof(struct sockaddr_in6);

            if(inet_pton(AF_INET6, tab_hash[taille_tab-1][1], &dest.sin6_addr) != 1)  //adresse tracker
            {
                perror("inet ACK PUT\n");
                close(sockfd2);
                exit(EXIT_FAILURE);
            }

            struct s_message ack_put;
            ack_put.msg_type = 111;
            ack_put.hash_type = 50;
            ack_put.hash_length = 64;
            strcpy(ack_put.hash, tab_hash[taille_tab-1][0]);
            ack_put.client_type = 55;
            ack_put.client_length = 18;
            ack_put.client_port = atoi(tab_hash[taille_tab-1][2]);
            ack_put.client = dest.sin6_addr;
            ack_put.msg_length = ack_put.hash_length + ack_put.client_length;

            if (sendto(sockfd2, &ack_put, sizeof(ack_put), 0, (struct sockaddr *) &dest, addrlen2) == -1)
            {
                perror("sendto ACK PUT");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            printf("ack put envoyé\n");
        }



        if (msg1.msg_type == 112)   //cas d'un GET
        {
            int sockfd2;
            socklen_t addrlen2;
            struct sockaddr_in6 dest;

            // socket de réponse au client
            if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
            {
                perror("socket fail \n");
                exit(EXIT_FAILURE);
            }

            dest.sin6_family = AF_INET6;
            dest.sin6_port   = htons(atoi(tab_hash[taille_tab-1][2]));
            addrlen2         = sizeof(struct sockaddr_in6);

            if(inet_pton(AF_INET6, tab_hash[taille_tab-1][1], &dest.sin6_addr) != 1)  //adresse tracker
            {
                perror("inet ACK GET\n");
                close(sockfd2);
                exit(EXIT_FAILURE);
            }

            int i = 0;
            while (strcmp(tab_hash[i][0], msg1.hash) && i < taille_tab)
            {
                i++;
            }

            struct s_message ack;
            ack.msg_type = 113;
            ack.hash_type = 50;
            ack.hash_length = 64;
            strcpy(ack.hash, tab_hash[i][0]);
            ack.client_type = 55;
            ack.client_length = 18;
            ack.client_port = atoi(tab_hash[i][2]);
            inet_pton(AF_INET6, tab_hash[i][1], &ack.client);
            ack.msg_length = ack.hash_length + ack.client_length;

            if (sendto(sockfd2, &ack, sizeof(ack), 0, (struct sockaddr *) &dest, addrlen2) == -1)
            {
                perror("sendto ACK GET");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }


        /*
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
perror("sendto ACK GET");
close(sockfd);
exit(EXIT_FAILURE);
}
printf("ACK GET envoyé \n");

// 110  taille_final-1 50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]
}
*/
}
close(sockfd);
return 0;
}
