/**************************************************************
                _              __            _   _
       _ __ ___| |_ _ __ ___  / _| ___  _ __| |_| |__
      | '__/ _ \ __| '__/ _ \| |_ / _ \| '__| __| '_ \
      | | |  __/ |_| | | (_) |  _| (_) | |  | |_| | | |
      |_|  \___|\__|_|  \___/|_|  \___/|_|   \__|_| |_|
                                                for nga

      (c) Charles Childers, Luke Parrish, Marc Simpsonn,
          Jay Skeer, Kenneth Keating

**************************************************************/

#include "retro.h"

#ifdef ENABLE_SOCKETS

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*---------------------------------------------------------------------
  BSD Sockets
  ---------------------------------------------------------------------*/

int SocketID[16];
struct sockaddr_in Sockets[16];

struct addrinfo hints, *res;

int socket_get_handle(NgaState *vm, CELL socket, int *handle) {
  if (socket < 0 || socket >= 16 || SocketID[socket] == 0) {
    printf("\nERROR (nga/sockets): Invalid socket handle %lld\n", (long long)socket);
    ACTIVE.ip = IMAGE_SIZE;
    ACTIVE.rp = 0;
    return 0;
  }
  *handle = SocketID[socket];
  return 1;
}

V socket_getaddrinfo(NgaState *vm) {
  char host[1025], port[6];
  strlcpy(port, string_extract(vm, stack_pop(vm)), 5);
  strlcpy(host, string_extract(vm, stack_pop(vm)), 1024);
  getaddrinfo(host, port, &hints, &res);
}

V socket_get_host(NgaState *vm) {
  struct hostent *hp;
  struct in_addr **addr_list;

  hp = gethostbyname(string_extract(vm, stack_pop(vm)));
  if (hp == NULL) {
    vm->memory[stack_pop(vm)] = 0;
    return;
  }

  addr_list = (struct in_addr **)hp->h_addr_list;
  string_inject(vm, inet_ntoa(*addr_list[0]), stack_pop(vm));
}

V socket_create(NgaState *vm) {
  int i;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && sock != 0) {
      SocketID[i] = sock;
      stack_push(vm, (CELL)i);
      sock = 0;
    }
  }
}

V socket_bind(NgaState *vm) {
  int handle;
  CELL sock, port;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  sock = stack_pop(vm);
  port = stack_pop(vm);
  if (!socket_get_handle(vm, sock, &handle)) return;

  getaddrinfo(NULL, string_extract(vm, port), &hints, &res);
  stack_push(vm, (CELL) bind(handle, res->ai_addr, res->ai_addrlen));
  stack_push(vm, errno);
}

V socket_listen(NgaState *vm) {
  int handle;
  CELL sock = stack_pop(vm);
  int backlog = stack_pop(vm);
  if (!socket_get_handle(vm, sock, &handle)) return;
  stack_push(vm, listen(handle, backlog));
  stack_push(vm, errno);
}

V socket_accept(NgaState *vm) {
  int i;
  int handle;
  CELL sock = stack_pop(vm);
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  if (!socket_get_handle(vm, sock, &handle)) return;
  int new_fd = accept(handle, (struct sockaddr *)&their_addr, &addr_size);

  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && new_fd != 0) {
      SocketID[i] = new_fd;
      stack_push(vm, (CELL)i);
      new_fd = 0;
    }
  }
  stack_push(vm, errno);
}

V socket_connect(NgaState *vm) {
  int handle;
  CELL sock = stack_pop(vm);
  if (!socket_get_handle(vm, sock, &handle)) return;
  stack_push(vm, (CELL)connect(handle, res->ai_addr, res->ai_addrlen));
  stack_push(vm, errno);
}

V socket_send(NgaState *vm) {
  int handle;
  CELL sock = stack_pop(vm);
  char *buf = string_extract(vm, stack_pop(vm));
  if (!socket_get_handle(vm, sock, &handle)) return;
  stack_push(vm, send(handle, buf, strlen(buf), 0));
  stack_push(vm, errno);
}

V socket_recv(NgaState *vm) {
  char buf[8193];
  int handle;
  CELL sock = stack_pop(vm);
  CELL limit = stack_pop(vm);
  CELL dest = stack_pop(vm);
  if (!socket_get_handle(vm, sock, &handle)) return;
  if (limit < 0 || limit >= (CELL)sizeof(buf)) {
    printf("\nERROR (nga/sockets): Invalid receive length %lld\n", (long long)limit);
    ACTIVE.ip = IMAGE_SIZE;
    ACTIVE.rp = 0;
    return;
  }
  int len = recv(handle, buf, (size_t)limit, 0);
  if (len > 0)  buf[len] = '\0';
  if (len > 0)  string_inject(vm, buf, dest);
  stack_push(vm, len);
  stack_push(vm, errno);
}

V socket_close(NgaState *vm) {
  int handle;
  CELL sock = stack_pop(vm);
  if (!socket_get_handle(vm, sock, &handle)) return;
  close(handle);
  SocketID[sock] = 0;
}

Handler SocketActions[] = {
  socket_get_host,
  socket_create, socket_bind,    socket_listen,
  socket_accept, socket_connect, socket_send,
  socket_recv,   socket_close,   socket_getaddrinfo
};

V invalid_socket_action(NgaState *vm, CELL action) {
  printf("\nERROR (nga/sockets): Invalid socket action %lld\n", (long long)action);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V io_socket(NgaState *vm) {
  CELL action = stack_pop(vm);
  CELL actions = sizeof(SocketActions) / sizeof(SocketActions[0]);
  if (action >= 0 && action < actions) {
    SocketActions[action](vm);
  } else {
    invalid_socket_action(vm, action);
  }
}

V query_socket(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_SOCKETS);
}

#endif
