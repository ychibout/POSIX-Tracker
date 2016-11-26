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
#define _GNU_SOURCE    # required for NI_NUMERICHOST
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <netdb.h>


int main(int argc, char **argv)
{
   int sockfd;
   socklen_t addrlen;
   struct sockaddr_in6 dest;



   // création du socket
   if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
   {
      perror("socket fail \n");
      exit(EXIT_FAILURE);
   }




   //création adresse destination
   dest.sin6_family = AF_INET6;
   dest.sin6_port   = htons(atoi(argv[2]));
   addrlen         = sizeof(struct sockaddr_in6);

   if(inet_pton(AF_INET6, argv[1], &dest.sin6_addr) != 1)
   {
      perror("inet fail  \n");
      close(sockfd);
      exit(EXIT_FAILURE);
   }




   //fichier
   unsigned char hash_type = 50;
   short int hash_len = strlen(argv[5])+1;

   char hash[strlen(argv[5])+1];
   strcpy(hash, argv[5]);



   //client
   unsigned char client_type = 55;
   short int client_len = sizeof(dest.sin6_addr) + 2;
   short int client_port = atoi(argv[3]);
   struct in6_addr client_addr = dest.sin6_addr;



   //message
   unsigned char msg_type;
   if (strcmp(argv[4], "put") == 0)
   {
      msg_type = 110;
   }

   if (strcmp(argv[4], "get") == 0)
   {
      msg_type = 112;
   }
   short int msg_len = 1+2+hash_len+1+2+client_len;
   int total_len = 1+2+msg_len;
   void* buf = malloc(total_len);




   //placement dans buf

   memcpy(buf, &msg_type, sizeof(msg_type));    //type message
   memcpy(buf+sizeof(msg_type), &msg_len, sizeof(msg_len)); //taille message
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len), &hash_type, sizeof(hash_type)); //type hash
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), &hash_len, sizeof(hash_len));   //taille hash
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), hash, sizeof(hash)); //hash
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), &client_type, sizeof(client_type));   //type client
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), &client_len, sizeof(client_len));   //taille client
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), &client_port, sizeof(client_port)); //port client
   memcpy(buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), &client_addr, sizeof(client_addr));   //adresse client



   //test de bonne mise en place

   char pmsg_type;
   short int pmsg_len;
   char phash_type;
   short int phash_len;
   char* phash[65];
   char pclient_type;
   short int pclient_len;
   short int pclient_port;
   char* client = (char*)malloc(50);
   struct in6_addr pclient_addr;

   memcpy(&pmsg_type, buf, sizeof(pmsg_type));    //type message
   memcpy(&pmsg_len, buf+sizeof(msg_type), sizeof(pmsg_len)); //taille message
   memcpy(&phash_type, buf+sizeof(msg_type)+sizeof(msg_len), sizeof(phash_type)); //type hash
   memcpy(&phash_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), sizeof(phash_len));   //taille hash
   memcpy(phash, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), sizeof(phash)); //hash
   memcpy(&pclient_type, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(pclient_type));   //type client
   memcpy(&pclient_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(pclient_len));   //taille client
   memcpy(&pclient_port, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(pclient_port));
   memcpy(&pclient_addr, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(pclient_port), sizeof(pclient_addr));

   printf("\n\nMessage à envoyer : \n");
   printf("msg_type : %u\n", pmsg_type);
   printf("msg_len : %d\n", pmsg_len);
   printf("hash_type : %u\n", phash_type);
   printf("hash_len : %d\n", phash_len);
   printf("hash : %s\n", phash);
   printf("client_type : %u\n", pclient_type);
   printf("client_len : %d\n", pclient_len);
   printf("client_port : %d\n", pclient_port);
   printf("client : %s\n", inet_ntop(AF_INET6, &pclient_addr, client, sizeof(pclient_addr)));


   //Envoi du message

   if (sendto(sockfd, buf, total_len, 0, (struct sockaddr *) &dest, addrlen) == -1)
   {
      perror("send to fail \n");
      close(sockfd);
      exit(EXIT_FAILURE);
   }


   int sockfd2; // descripteur
   socklen_t addrlen2; // socket

   struct sockaddr_in6 my_addr; // adresse ipv4

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

   printf("\nlistening on %d\n", atoi(argv[3]));

   // association de la socket avec l'adresse
   if( bind(sockfd2, (struct sockaddr *) &my_addr, addrlen2) == -1) // adresse
   {
      perror("bind fail\n");
      close(sockfd2);
      exit(EXIT_FAILURE);
   }

   if (!strcmp(argv[4],"put") || !strcmp(argv[4],"get"))
   {
      void* ack = malloc(total_len+1000);

      // reception de la chaine de caracteres
      if(recvfrom(sockfd2, ack , total_len+1000, 0, (struct sockaddr *) &my_addr, &addrlen2 ) == -1)
      {
         perror("recvfrom fail \n");
         close(sockfd2);
         exit(EXIT_FAILURE);
      }

      char rmsg_type;
      short int rmsg_len;
      char rhash_type;
      short int rhash_len;
      char* rhash[65];
      char rclient_type;
      short int rclient_len;
      short int rclient_port;
      char* rclient = (char*)malloc(50);
      struct in6_addr rclient_addr;

      memcpy(&rmsg_type, ack, sizeof(pmsg_type));    //type message
      memcpy(&rmsg_len, ack+sizeof(msg_type), sizeof(pmsg_len)); //taille message
      memcpy(&rhash_type, ack+sizeof(msg_type)+sizeof(msg_len), sizeof(phash_type)); //type hash
      memcpy(&rhash_len, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), sizeof(phash_len));   //taille hash
      memcpy(rhash, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), sizeof(phash)); //hash
      memcpy(&rclient_type, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(pclient_type));   //type client
      memcpy(&rclient_len, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(pclient_len));   //taille client
      memcpy(&rclient_port, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(pclient_port));
      memcpy(&rclient_addr, ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(pclient_port), sizeof(pclient_addr));

      printf("\n\nRéponse tracker :\n");
      printf("msg_type : %u\n", rmsg_type);
      printf("msg_len : %d\n", rmsg_len);
      printf("hash_type : %u\n", rhash_type);
      printf("hash_len : %d\n", rhash_len);
      printf("hash : %s\n", rhash);
      printf("client_type : %u\n", rclient_type);
      printf("client_len : %d\n", rclient_len);
      printf("client_port : %d\n", rclient_port);
      printf("client : %s\n", inet_ntop(AF_INET6, &rclient_addr, client, sizeof(rclient_addr)));
      
      int s = 21;
      while ( rmsg_len > 89 )
      {
        memcpy(&rclient_type, s+ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(pclient_type));   //type client
        memcpy(&rclient_len, s+ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(pclient_len));   //taille client
        memcpy(&rclient_port, s+ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(pclient_port));
        memcpy(&rclient_addr, s+ack+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(pclient_port), sizeof(pclient_addr));
        s+=21;
        rmsg_len-=21;
        printf("\nclient_type : %u\n", rclient_type);
        printf("client_len : %d\n", rclient_len);
        printf("client_port : %d\n", rclient_port);
        printf("client : %s\n", inet_ntop(AF_INET6, &rclient_addr, client, sizeof(rclient_addr)));
      }
 

   }

   free(buf);
   close(sockfd);
   return 0;
}

//Message put : [110] [1+2+(1+2+64)+(1+2+18)] [50 64 b8799e375b7cce6d5c6e2651bcce0eb0458f663457287ddf0f4cd93e8327c3fb] [55 18 argv[2] inet_pton(AF_INET6, ::1, buf)]
