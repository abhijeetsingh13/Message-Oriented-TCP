#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SOCK_MyTCP 4003
// Create table for storing Send_Message with atmax 10 entries

typedef struct{
    int sockfd[10];
    char *msg[10];
    int len[10];
    int count;
    int st,end;
}Send_Message;
typedef struct{
    int sockfd[10];
    char *msg[10];
    int len[10];
    int count;
    int st,end;
}Received_Message;


int my_socket(int domain,int type,int protocol);
int my_bind(int sockfd,const struct sockaddr *my_addr,socklen_t addrlen);
int my_listen(int sockfd,int backlog);
int my_accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen);
int my_connect(int sockfd,const struct sockaddr *serv_addr,socklen_t addrlen);
size_t my_send(int sockfd,const void *buf,size_t len,int flags);
size_t my_recv(int sockfd,void *buf,size_t len,int flags);
int my_close(int fd);



