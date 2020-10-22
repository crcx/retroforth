/*---------------------------------------------------------------------
  RETRO is a personal, minimalistic forth with a pragmatic focus

  This implements Nga, the virtual machine at the heart of RETRO. It
  includes a number of I/O interfaces, extensive commentary, and has
  been refined by over a decade of use and development.

  Copyright (c) 2008 - 2020, Charles Childers

  Portions are based on Ngaro, which was additionally copyrighted by
  the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  C Headers
  ---------------------------------------------------------------------*/

#include <ctype.h> 
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "config.h"

/*---------------------------------------------------------------------
  BSD Sockets
  ---------------------------------------------------------------------*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

int SocketID[16];
struct sockaddr_in Sockets[16];

struct addrinfo hints, *res;

void socket_getaddrinfo() {
  char host[1025], port[6];
  bsd_strlcpy(port, string_extract(stack_pop()), 5);
  bsd_strlcpy(host, string_extract(stack_pop()), 1024);
  getaddrinfo(host, port, &hints, &res);
}

void socket_get_host() {
  struct hostent *hp;
  struct in_addr **addr_list;

  hp = gethostbyname(string_extract(stack_pop()));
  if (hp == NULL) {
    memory[stack_pop()] = 0;
    return;
  }

  addr_list = (struct in_addr **)hp->h_addr_list;
  string_inject(inet_ntoa(*addr_list[0]), stack_pop());
}

void socket_create() {
  int i;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && sock != 0) {
      SocketID[i] = sock;
      stack_push((CELL)i);
      sock = 0;
    }
  }
}

void socket_bind() {
  int sock, port;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  sock = stack_pop();
  port = stack_pop();

  getaddrinfo(NULL, string_extract(port), &hints, &res);
  stack_push((CELL) bind(SocketID[sock], res->ai_addr, res->ai_addrlen));
  stack_push(errno);
}

void socket_listen() {
  int sock = stack_pop();
  int backlog = stack_pop();
  stack_push(listen(SocketID[sock], backlog));
  stack_push(errno);
}

void socket_accept() {
  int i;
  int sock = stack_pop();
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  int new_fd = accept(SocketID[sock], (struct sockaddr *)&their_addr, &addr_size);

  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && new_fd != 0) {
      SocketID[i] = new_fd;
      stack_push((CELL)i);
      new_fd = 0;
    }
  }
  stack_push(errno);
}

void socket_connect() {
  stack_push((CELL)connect(SocketID[stack_pop()], res->ai_addr, res->ai_addrlen));
  stack_push(errno);
}

void socket_send() {
  int sock = stack_pop();
  char *buf = string_extract(stack_pop());
  stack_push(send(SocketID[sock], buf, strlen(buf), 0));
  stack_push(errno);
}

void socket_sendto() {
}

void socket_recv() {
  char buf[8193];
  int sock = stack_pop();
  int limit = stack_pop();
  int dest = stack_pop();
  int len = recv(SocketID[sock], buf, limit, 0);
  if (len > 0)  buf[len] = '\0';
  if (len > 0)  string_inject(buf, dest);
  stack_push(len);
  stack_push(errno);
}

void socket_recvfrom() {
}

void socket_close() {
  int sock = stack_pop();
  close(SocketID[sock]);
  SocketID[sock] = 0;
}

Handler SocketActions[] = {
  socket_get_host,
  socket_create, socket_bind,    socket_listen,
  socket_accept, socket_connect, socket_send,
  socket_sendto, socket_recv,    socket_recvfrom,
  socket_close, socket_getaddrinfo
};

void io_socket() {
  SocketActions[stack_pop()]();
}

void query_socket() {
  stack_push(0);
  stack_push(7);
}
