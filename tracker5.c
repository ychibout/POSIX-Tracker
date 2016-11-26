#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct elem {
     char hash[65];
     struct in6_addr client;
     short int port;
};

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
     socklen_t addrlen; // socket
     struct sockaddr_in6 my_addr; // adresse ipv6
     struct sockaddr_in6 client;	// adresse ipv6 client

     int sockfd2;   //socket de la réponse
     struct sockaddr_in6 dest;     //adresse socket
     socklen_t addrlen2; //longueur de l'adresse

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

     // association de la socket avec l'adresse
     if( bind(sockfd, (struct sockaddr *) &my_addr, addrlen) == -1) // adresse
     {
          perror("bind fail\n");
          close(sockfd);
          exit(EXIT_FAILURE);
     }




     //PARSE

     //hash
     unsigned char hash_type;
     short int hash_len;
     char hash[65];



     //client
     unsigned char client_type;
     short int client_len;
     short int client_port;
     char* client_tst = (char*)malloc(50);
     struct in6_addr client_addr; // ICI ADD CLIENT ET NON DST



     //msg
     unsigned char msg_type;
     short int msg_len;
     void* buf = malloc(100);


     //tableau de réception
     struct elem *tab = (struct elem*)malloc(sizeof(struct elem));
     int fait = 0;
     int nbelem = 1;


     // reception de la chaine de caracteres
     while (1)
     {
          if(recvfrom(sockfd, buf , 100, 0, (struct sockaddr *) &client, &addrlen ) == -1)
          {
               perror("recvfrom fail \n");
               close(sockfd);
               exit(EXIT_FAILURE);
          }


          memcpy(&msg_type, buf, sizeof(msg_type));    //type message
          memcpy(&msg_len, buf+sizeof(msg_type), sizeof(msg_len)); //taille message
          memcpy(&hash_type, buf+sizeof(msg_type)+sizeof(msg_len), sizeof(hash_type)); //type hash
          memcpy(&hash_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), sizeof(hash_len));   //taille hash
          memcpy(hash, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), sizeof(hash)); //hash
          memcpy(&client_type, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(client_type));   //type client
          memcpy(&client_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(client_len));   //taille client
          memcpy(&client_port, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(client_port));   //port client
          memcpy(&client_addr, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), sizeof(client_addr));   //adresse client

          if (msg_type == 110)
          {

               if (fait == 0)
               {
                    strcpy(tab[0].hash, hash);
                    tab[0].client = client_addr;
                    tab[0].port = client_port;
                    fait = 1;
                    printf("\n\nNouvelle entrée : \n");
                    printf("Hash : %s\n", tab[0].hash);
                    printf("Adresse client : %s\n", inet_ntop(AF_INET6, &tab[0].client, client_tst, sizeof(tab[0].client)));
                    printf("Port : %d\n", tab[0].port);
                    printf("nb éléments dans le tracker : %d\n\n", nbelem);
               }
               else
               {
                    nbelem++;
                    tab = (struct elem*)realloc(tab, nbelem*sizeof(struct elem));
                    strcpy(tab[nbelem-1].hash, hash);
                    tab[nbelem-1].client = client_addr;
                    tab[nbelem-1].port = client_port;
                    printf("\n\nNouvelle entrée : \n");
                    printf("Hash : %s\n", tab[nbelem-1].hash);
                    printf("Adresse client : %s\n", inet_ntop(AF_INET6, &tab[nbelem-1].client, client_tst, sizeof(tab[nbelem-1].client)));
                    printf("Port : %d\n", tab[nbelem-1].port);
                    printf("nb éléments dans le tracker : %d\n\n", nbelem);
               }

               void *rep = malloc(msg_len+1+2);
               msg_type = 111;
               memcpy(rep, &msg_type, sizeof(msg_type));    //type message
               memcpy(rep+sizeof(msg_type), &msg_len, sizeof(msg_len)); //taille message
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len), &hash_type, sizeof(hash_type)); //type hash
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), &hash_len, sizeof(hash_len));   //taille hash
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), tab[nbelem-1].hash, sizeof(hash)); //hash
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), &client_type, sizeof(client_type));   //type client
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), &client_len, sizeof(client_len));   //taille client
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), &tab[nbelem-1].port, sizeof(client_port)); //port client
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), &tab[nbelem-1].client, sizeof(client_addr));   //adresse client

               if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
               {
                    perror("socket fail \n");
                    exit(EXIT_FAILURE);
               }

               dest.sin6_family = AF_INET6;
               dest.sin6_addr   = tab[nbelem-1].client;
               dest.sin6_port   = htons(tab[nbelem-1].port);
               addrlen2         = sizeof(struct sockaddr_in6);

               if (sendto(sockfd2, rep, msg_len+1+2, 0, (struct sockaddr *) &dest, addrlen2) == -1)
               {
                    perror("sendto ACK PUT");
                    close(sockfd);
                    exit(EXIT_FAILURE);
               }
          }

          if (msg_type == 112)
          {
               short int msg_len2 = 68;
               int nbclients = 0;
               void *rep = malloc(71);
               msg_type = 113;
               memcpy(rep, &msg_type, sizeof(msg_type)); //type message
               /*msg_len */ memcpy(rep+sizeof(msg_type), &msg_len, sizeof(msg_len)); //taille message
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len), &hash_type, sizeof(hash_type)); // type hash
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), &hash_len, sizeof(hash_len)); // taille hash
               memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), hash, sizeof(hash)); // hash
               
               int i;
               for (i = 0; i < nbelem; i++)
               {
                    if (strcmp(tab[i].hash, hash) == 0)
                    {
                         nbclients++;
                         rep = realloc(rep, 71+nbclients*21);
                         memcpy(rep+71+(nbclients-1)*21, &client_type, sizeof(client_type));
                         memcpy(rep+71+(nbclients-1)*21+sizeof(client_type), &client_len, sizeof(client_len));
                         memcpy(rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len), &tab[i].port, sizeof(client_port));
                         memcpy(rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), &tab[i].client, sizeof(client_addr));
                    }
                    
               }
              // printf("Nb clients : %d \n",nbclients);
               msg_len2+=21*nbclients;
              // printf("msg_len %u \n",msg_len2);
               memcpy(rep+sizeof(msg_type), &msg_len2, sizeof(msg_len2)); 

               if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
               {
                    perror("socket fail \n");
                    exit(EXIT_FAILURE);
               }

               dest.sin6_family = AF_INET6;
               dest.sin6_addr   = client_addr;
               dest.sin6_port   = htons(client_port);
               addrlen2         = sizeof(struct sockaddr_in6);

               if (sendto(sockfd2, rep, 71+nbclients*21, 0, (struct sockaddr *) &dest, addrlen2) == -1)
               {
                    perror("sendto ACK GET");
                    close(sockfd);
                    exit(EXIT_FAILURE);
               }
          }
     }


     close(sockfd);
     free(buf);
     free(tab);
     return 0;
}
