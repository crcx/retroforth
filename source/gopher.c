#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
  perror(msg);
  exit(0);
}

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


/* <addr> <server> <port> <selector> */
void ngaGopherUnit() {
  char server[1025];
  char selector[4097];
  CELL port, dest;
  strcpy(selector, string_extract(stack_pop()));
  port = stack_pop();
  strcpy(server, string_extract(stack_pop()));
  dest = stack_pop();
  gopher_fetch(server, port, selector, dest);
}
