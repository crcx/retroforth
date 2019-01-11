/* RETRO --------------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2019 Charles Childers

  Gopher Support

  I'm a big fan of Gopher, so RRE provides support for fetching files
  via the Gopher protocol.
  ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

typedef void (*Handler)(void);

/*---------------------------------------------------------------------
  First, a few things that relate to the Nga virtual machine that RETRO
  runs on.
  ---------------------------------------------------------------------*/

#define CELL         int32_t      /* Cell size (32 bit, signed integer */
#define IMAGE_SIZE   524288 * 48  /* Amount of RAM. 12MiB by default.  */
#define ADDRESSES    2048         /* Depth of address stack            */
#define STACK_DEPTH  512          /* Depth of data stack               */

extern CELL sp, rp, ip;             /* Stack & instruction pointers    */
extern CELL data[STACK_DEPTH];      /* The data stack                  */
extern CELL address[ADDRESSES];     /* The address stack               */
extern CELL memory[IMAGE_SIZE + 1]; /* The memory for the image        */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */

/*---------------------------------------------------------------------
  Function prototypes.
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
char *string_extract(CELL at);
CELL string_inject(char *str, CELL buffer);

/*---------------------------------------------------------------------
  The first Gopher related function is `error()`, which prints an
  error message and exits if there is a problem.
  ---------------------------------------------------------------------*/

void error(const char *msg) {
  perror(msg);
  exit(0);
}


/*---------------------------------------------------------------------
  `gopher_fetch()` is the part that does all the real work.
  ---------------------------------------------------------------------*/

void gopher_fetch(char *host, CELL port, char *selector, CELL dest) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char data[128 * 1024 + 1];
  char buffer[1025];

  portno = (int)port;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  server = gethostbyname(host);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero(data, 128 * 1024 + 1);
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
     (char *)&serv_addr.sin_addr.s_addr,
     server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  n = write(sockfd,selector,strlen(selector));
  if (n < 0)
     error("ERROR writing to socket");

  n = write(sockfd,"\n",strlen("\n"));
  if (n < 0)
     error("ERROR writing to socket");

  n = 1;
  while (n > 0) {
    bzero(buffer,1025);
    n = read(sockfd,buffer,1024);
    strcat(data, buffer);
  }

  close(sockfd);
  string_inject(data, dest);
  stack_push(strlen(data));
}



/*---------------------------------------------------------------------
  The last Gopher function, `ngaGopherUnit()` pulls the values needed
  from the stack and passes them to the `gopher_fetch()` function.

  This will take the following from the stack (TOS to bottom):

  Selector   (NULL terminated string)
  Port       (Number)
  Server     (NULL terminated string)
  Buffer     (Pointer to memory that will hold the received file)
  ---------------------------------------------------------------------*/

void ngaGopherUnit() {
  CELL port, dest;
  char server[1025], selector[4097];
  strlcpy(selector, string_extract(stack_pop()), 4096);
  port = stack_pop();
  strlcpy(server, string_extract(stack_pop()), 1024);
  dest = stack_pop();
  gopher_fetch(server, port, selector, dest);
}

Handler GopherActions[1] = {
  ngaGopherUnit
};

void io_gopher_query() {
  stack_push(0);
  stack_push(5);
}

void io_gopher_handler() {
  GopherActions[stack_pop()]();
}
