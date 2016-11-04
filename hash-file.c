/*
 * compile with:
 * cc sha256.c -lcrypto
 */
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>

char* my_sha256 (char* arg_file)
{
	unsigned char buffer[BUFSIZ];
	FILE *f;
	SHA256_CTX ctx;
	size_t len;
	char *mb = malloc(2*sizeof(char));
	char *res = malloc(65*sizeof(char));
        int i = 0;

	f = fopen(arg_file, "r");
	if (!f)
	{
		fprintf(stderr, "Erreur lors de la lecture du fichier\n");
	}

	SHA256_Init(&ctx);

	do {
		len = fread(buffer, 1, BUFSIZ, f);
		SHA256_Update(&ctx, buffer, len);
	} while (len == BUFSIZ);

	SHA256_Final(buffer, &ctx);

	fclose(f);

	for (len = 0; len < SHA256_DIGEST_LENGTH; ++len)
        {
		snprintf(mb, 3, "%02x", buffer[len]);
		res[i] = mb[0];
		res[i+1] = mb[1];
		i+=2;
        }

	res[64] = '\0';
	free(mb);
	return res;
	free(res);
}

int decoup (char* arg_file) {
   FILE *fpl;
   FILE *fpe;
   int len;
   int i;
   int j = 1;
   char fname[10];
   char c;

   fpl = fopen(arg_file, "r");
   if( fpl == NULL )  {
      perror ("Error opening file");

      return(-1);
   }

   fseek(fpl, 0, SEEK_END);
   len = ftell(fpl);
   fseek(fpl, 0, SEEK_SET);

   sprintf(fname, "%d.txt", j);
   fpe = fopen(fname, "ab+");
   c = fgetc(fpl);
   fputc(c, fpe);

   for (i = 1; i < len; i++)
   {
	   if (i%1000000 == 0)
	   {
		   fclose(fpe);
		   j++;
		   sprintf(fname, "%d.txt", j);
		   fpe = fopen(fname, "ab+");
	   }

	   c = fgetc(fpl);
	   fputc(c, fpe);
   }

   fclose(fpl);
   fclose(fpe);

   return(j);
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage : %s <file>", argv[0]);
		return 1;
	}

	char fname[10];
	char** chunk_tab;
	char* principal = malloc(65*sizeof(char));

	principal = my_sha256(argv[1]);
	printf("FILE HASH : %s\n", principal);

	int i;
	int j = decoup(argv[1]);
	int k;

	chunk_tab = malloc(j*sizeof(char*));

	for (i = 0; i < j; i++)
	{
		chunk_tab[i] = malloc(65*sizeof(char));
	}

	for (i = 0; i < j; i++)
	{
		sprintf(fname, "%d.txt", (i+1));
		chunk_tab[i] = my_sha256(fname);
		printf("CHUNK %d : %s\n", (i+1), chunk_tab[i]);
	}

	return 0;
}
