#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

struct elem
{
  char hash[65];
  struct in6_addr client;
  short int port;
  time_t ttl;
  struct elem* suiv;
};

int nbelem;
struct elem *tab;

struct elem* supprimerElement(struct elem* list, time_t valeur)
{
  struct elem *tmp;
  struct elem *previous;

  if (list == NULL) // si la liste est NULL on s'arrete tout de suite
  return (list);

  previous = list;
  if (previous->ttl == valeur) // Verifie la tete de liste, cas particulier
  {
    list = previous->suiv;
    free(previous);
    return (list);
  }

  tmp = previous->suiv; // le cas n est gere on se place donc sur le cas n+1
  while(tmp != NULL) // On Mouline est on supprime si on trouve l'element
  {
    if (tmp->ttl == valeur)
    {
      previous->suiv = tmp->suiv;
      free(tmp);
      return (list);
    }
    previous = tmp; // pour ce souvenir dans la prochaine iteration du precedent
    tmp = tmp->suiv;
  }
  return list;
}


void* doSomeThing(void *arg)
{
  time_t seconds;
  int i=0;
  //struct elem *tab = (struct elem *) arg;
  struct elem *temp;
  temp = tab;

  while (1)
  {
    temp = tab;
    sleep(1);

    seconds = time(NULL);

    while (temp != NULL)
    {

      if(temp->ttl == seconds)
      {
        printf("Suppression element : temp->hash = %s \n",temp->hash);
        tab = supprimerElement(tab,temp->ttl);
        nbelem--;
      }
      temp = temp->suiv;
    }
  }


  return NULL;
}

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




  /*PARTIE 2 : socket de reception de donn�es*/

  int sockfd; // descripteur
  socklen_t addrlen; // socket
  struct sockaddr_in6 my_addr; // adresse ipv6
  struct sockaddr_in6 client;	// adresse ipv6 client

  int sockfd2;   //socket de la r�ponse
  struct sockaddr_in6 dest;     //adresse socket
  socklen_t addrlen2; //longueur de l'adresse

  // socket du cr�er
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
  int test = 0;
  nbelem = 0;
  struct elem* temp;

  // tracker

  time_t seconds;
  pthread_t t;
  int err;

  err = pthread_create(&t, NULL, &doSomeThing, (void*)&tab[0]);
  if (err != 0)
  printf("\ncan't create thread :[%s]", strerror(err));
  else
  printf("\n Thread created successfully\n");

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

    if (msg_type == 110)
    {
      memcpy(&msg_len, buf+sizeof(msg_type), sizeof(msg_len)); //taille message
      memcpy(&hash_type, buf+sizeof(msg_type)+sizeof(msg_len), sizeof(hash_type)); //type hash
      memcpy(&hash_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), sizeof(hash_len));   //taille hash
      memcpy(hash, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), sizeof(hash)); //hash
      memcpy(&client_type, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(client_type));   //type client
      memcpy(&client_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(client_len));   //taille client
      memcpy(&client_port, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(client_port));   //port client
      memcpy(&client_addr, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), sizeof(client_addr));   //adresse client


      if (nbelem == 0)
      {
        nbelem++;
        tab = (struct elem*)malloc(sizeof(struct elem));
        strcpy(tab->hash, hash);
        tab->client = client_addr;
        tab->port = client_port;
        tab->ttl = time(NULL)+10;
        tab->suiv=NULL;
        test = 1;
        printf("\n\nNouvelle entrée : \n");
        printf("Hash : %s\n", tab->hash);
        printf("Adresse client : %s\n", inet_ntop(AF_INET6, &tab->client, client_tst, sizeof(tab->client)));
        printf("Port : %d\n", tab->port);
        printf("nb éléments dans le tracker : %d\n\n", nbelem);
      }
      else
      {
        nbelem++;
        temp = tab;
        while (temp->suiv != NULL)
        {
          temp = temp->suiv;
        }
        struct elem* new =  (struct elem*)malloc(sizeof(struct elem));
        strcpy(new->hash, hash);
        new->client = client_addr;
        new->port = client_port;
        new->ttl = time(NULL)+10;
        new->suiv=NULL;

        temp->suiv = new;
        printf("\n\nNouvelle entrée : \n");
        printf("Hash : %s\n", new->hash);
        printf("Adresse client : %s\n", inet_ntop(AF_INET6, &new->client, client_tst, sizeof(new->client)));
        printf("Port : %d\n", new->port);
        printf("nb éléments dans le tracker : %d\n\n", nbelem);
      }

      void *rep = malloc(msg_len+1+2);
      msg_type = 111;
      memcpy(rep, &msg_type, sizeof(msg_type));    //type message
      memcpy(rep+sizeof(msg_type), &msg_len, sizeof(msg_len)); //taille message
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len), &hash_type, sizeof(hash_type)); //type hash
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), &hash_len, sizeof(hash_len));   //taille hash
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), hash, sizeof(hash)); //hash
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), &client_type, sizeof(client_type));   //type client
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), &client_len, sizeof(client_len));   //taille client
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), &client_port, sizeof(client_port)); //port client
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), &client_addr, sizeof(client_addr));   //adresse client

      if((sockfd2 = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
      {
        perror("socket fail \n");
        exit(EXIT_FAILURE);
      }

      dest.sin6_family = AF_INET6;
      dest.sin6_addr   = client_addr;
      dest.sin6_port   = htons(client_port);
      addrlen2         = sizeof(struct sockaddr_in6);

      if (sendto(sockfd2, rep, msg_len+1+2, 0, (struct sockaddr *) &dest, addrlen2) == -1)
      {
        perror("sendto ACK PUT");
        close(sockfd);
        exit(EXIT_FAILURE);
      }
      printf("ACK PUT envoyé \n");
    }


    if (msg_type == 112)
    {
      memcpy(&msg_len, buf+sizeof(msg_type), sizeof(msg_len)); //taille message
      memcpy(&hash_type, buf+sizeof(msg_type)+sizeof(msg_len), sizeof(hash_type)); //type hash
      memcpy(&hash_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), sizeof(hash_len));   //taille hash
      memcpy(hash, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), sizeof(hash)); //hash
      memcpy(&client_type, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash), sizeof(client_type));   //type client
      memcpy(&client_len, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type), sizeof(client_len));   //taille client
      memcpy(&client_port, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len), sizeof(client_port));   //port client
      memcpy(&client_addr, buf+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len)+sizeof(hash)+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), sizeof(client_addr));   //adresse client

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


      short int msg_len2 = 68;
      int nbclients = 0;
      void *rep = malloc(71);
      msg_type = 113;
      memcpy(rep, &msg_type, sizeof(msg_type)); //type message
      memcpy(rep+sizeof(msg_type), &msg_len, sizeof(msg_len)); //taille message
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len), &hash_type, sizeof(hash_type)); // type hash
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type), &hash_len, sizeof(hash_len)); // taille hash
      memcpy(rep+sizeof(msg_type)+sizeof(msg_len)+sizeof(hash_type)+sizeof(hash_len), hash, sizeof(hash)); // hash


      temp = tab;
      while (temp != NULL)
      {
        if (strcmp(temp->hash, hash) == 0)
        {
          nbclients++;
          rep = realloc(rep, 71+nbclients*21);
          memcpy(rep+71+(nbclients-1)*21, &client_type, sizeof(client_type));
          memcpy(rep+71+(nbclients-1)*21+sizeof(client_type), &client_len, sizeof(client_len));
          memcpy(rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len), &temp->port, sizeof(client_port));
          memcpy(rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), &temp->client, sizeof(client_addr));

          memcpy(&rclient_type, rep+71+(nbclients-1)*21, sizeof(rclient_type));
          memcpy(&rclient_len,  rep+71+(nbclients-1)*21+sizeof(client_type), sizeof(rclient_len));
          memcpy(&rclient_port, rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len), sizeof(rclient_port));
          memcpy(&rclient_addr, rep+71+(nbclients-1)*21+sizeof(client_type)+sizeof(client_len)+sizeof(client_port), sizeof(rclient_addr));

          /*printf("client_type : %u\n", rclient_type);
          printf("client_len : %d\n", rclient_len);
          printf("client_port : %d\n", rclient_port);
          printf("client : %s\n", inet_ntop(AF_INET6, &rclient_addr, rclient, sizeof(rclient_addr)));
          */
        }

        temp = temp->suiv;
      }

      if ( nbclients != 0)
      {
        nbelem++;
        temp = tab;
        while (temp->suiv != NULL)
        {
          temp = temp->suiv;
        }
        struct elem* new =  (struct elem*)malloc(sizeof(struct elem));
        strcpy(new->hash, hash);
        new->client = client_addr;
        new->port = client_port;
        new->ttl = time(NULL)+10;
        new->suiv=NULL;

        temp->suiv = new;
        printf("\n\nNouvelle entrée: \n");
        printf("Hash : %s\n", new->hash);
        printf("Adresse client : %s\n", inet_ntop(AF_INET6, &new->client, client_tst, sizeof(new->client)));
        printf("Port : %d\n", new->port);
        printf("nb éléments dans le tracker : %d\n\n", nbelem);
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
      printf("ACK GET envoyé \n");
    }

    if (msg_type == 114)
    {
      short int amsg_len;
      char ahash_type;
      short int ahash_len;
      char ahash[65];

      memcpy(&amsg_len, buf+sizeof(msg_type), sizeof(amsg_len)); //taille message
      memcpy(&ahash_type, buf+sizeof(msg_type)+sizeof(amsg_len), sizeof(ahash_type)); //type hash
      memcpy(&ahash_len, buf+sizeof(msg_type)+sizeof(amsg_len)+sizeof(ahash_type), sizeof(ahash_len));   //taille hash
      memcpy(ahash, buf+sizeof(msg_type)+sizeof(amsg_len)+sizeof(ahash_type)+sizeof(ahash_len), sizeof(ahash)); //hash

      printf("test : %s\n", ahash);

      temp = tab;
      while (strcmp(temp->hash, ahash) != 0)
      {
        temp = temp->suiv;
      }

      temp->ttl = time(NULL)+10;

      void *rep = malloc(amsg_len);
      char rmsg_type = 115;
      short int rmsg_len = amsg_len;
      char rhash_type = 55;
      short int rhash_len = 65;
      char rhash[65];

      memcpy(rep, &rmsg_type, sizeof(rmsg_type));
      memcpy(rep+sizeof(rmsg_type), &rmsg_len, sizeof(rmsg_len));
      memcpy(rep+sizeof(rmsg_type)+sizeof(rmsg_len), &rhash_type, sizeof(rhash_type));
      memcpy(rep+sizeof(rmsg_type)+sizeof(rmsg_len)+sizeof(rhash_type), &rhash_len, sizeof(rhash_len));
      memcpy(rep+sizeof(rmsg_type)+sizeof(rmsg_len)+sizeof(rhash_type)+sizeof(rhash_len), rhash, sizeof(rhash));

      dest.sin6_family = AF_INET6;
      dest.sin6_addr   = temp->client;
      dest.sin6_port   = temp->port;
      addrlen2         = sizeof(struct sockaddr_in6);

      if (sendto(sockfd2, rep, 71, 0, (struct sockaddr *) &dest, addrlen2) == -1)
      {
        perror("sendto ACK GET");
        close(sockfd);
        exit(EXIT_FAILURE);
      }
      printf("ACK KEEP_ALIVE envoyé \n");

    }
  }


  close(sockfd);
  free(buf);
  free(tab);
  return 0;
}
