#include "mysocket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
Send_Message snd_msg;
Received_Message rcv_msg;
pthread_t R, S;
pthread_mutex_t send_message_mutex;
pthread_mutex_t receive_message_mutex;
pthread_cond_t recv_cond;
pthread_mutex_t recv_mutex;
int st_sockfd = -1;
int check = 0;
int cleaning_up = 0;

void *recv_thread(void *arg)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1)
    {
            pthread_mutex_lock(&recv_mutex);
            //printf("check = %d\n",check);
            while(check==0)
            {
                pthread_cond_wait(&recv_cond,&recv_mutex);
            }
            pthread_mutex_unlock(&recv_mutex);
            // Take only four characters and store it in a string and then convert it into int
            char *len_str = (char *)malloc(4 * sizeof(char));
            int x = 4;
            while (x > 0)
            {
                int ret = recv(st_sockfd, len_str + 4 - x, x, 0);
                //printf("%s %d\n",len_str, ret);
                if (ret < 0)
                {
                    printf("Error in receiving message");
                }
                x -= ret;
            }
            x = atoi(len_str);
            // Take the message of length x in temp_str
            char *temp_str = (char *)malloc(x * sizeof(char));
            int taken = 0;
            while (1)
            {
                int ret = recv(st_sockfd, temp_str + taken, x-taken, 0);
                if (ret < 0)
                {
                    printf("Error in receiving message");
                }
                taken=ret+taken;
                if (taken == x)
                {
                    break;
                }
            }
            // initialise rcv_msg.end to default values
            pthread_mutex_lock(&receive_message_mutex);
            rcv_msg.sockfd[rcv_msg.end] = st_sockfd;
            rcv_msg.len[rcv_msg.end] = x;
            // copy the message from temp_str to rcv_msg.msg
            for (int i = 0; i < x; i++)
            {
                rcv_msg.msg[rcv_msg.end][i] = temp_str[i];
            }
            //printf("Message added to table: %s\n",rcv_msg.msg[rcv_msg.end]);
            rcv_msg.count++;
            //printf("rcv_msg.count = %d\n",rcv_msg.count);
            rcv_msg.end = (rcv_msg.end + 1) % 10;
            free(temp_str);
            pthread_mutex_unlock(&receive_message_mutex);
        
    }
}
void *send_thread(void *arg)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1)
    {
        sleep(1);
        //printf("In send thread\n");
        pthread_mutex_lock(&send_message_mutex);
        //printf("%d\n",snd_msg.count);
        if (snd_msg.count > 0)
        {
            int sockfd = snd_msg.sockfd[snd_msg.st];
            char *msg;
            //malloc
            msg = (char *)malloc(5000 * sizeof(char));
            //copy from snd_msg.msg
            msg = snd_msg.msg[snd_msg.st];
            int len = snd_msg.len[snd_msg.st];
            snd_msg.count--;
            
            int x = 0;
            //print the message
            //printf("%s %d\n",msg,len);
            // send the message as long as complete length is not sent
            int max_len=1000;
            while (1)
            {
                int to_send;
                if(len-x<max_len)
                {
                   to_send = len-x;
                }
                else
                {
                   to_send = max_len;
                }
                int ret = send(sockfd, msg + x, to_send, 0);
                if (ret < 0)
                {
                    printf("Error in sending message");
                }
                x += ret;
                if (x == len)
                {
                    break;
                }
            }
            // free the snd_msg.msg
            for(int i=0;i<5000;i++)
            {
                snd_msg.msg[snd_msg.st][i] = '\0';
            }
            snd_msg.len[snd_msg.st] = 0;
            snd_msg.sockfd[snd_msg.st] = -1;
            snd_msg.st = (snd_msg.st + 1) % 10;
           free(msg);
        }
        pthread_mutex_unlock(&send_message_mutex);

    }
}
int my_socket(int domain, int type, int protocol)
{
    int fd;
    // Initialise the send_message table
    snd_msg.count = 0;
    snd_msg.st = 0;
    snd_msg.end = 0;
    // Initialise the receive_message table
    rcv_msg.count = 0;
    rcv_msg.st = 0;
    rcv_msg.end = 0;
    // Create threads R and S
    if (pthread_create(&R, NULL, recv_thread, NULL) != 0)
        return -1;
    if (pthread_create(&S, NULL, send_thread, NULL) != 0)
        return -1;
    pthread_mutex_init(&send_message_mutex, NULL);
    pthread_mutex_init(&receive_message_mutex, NULL);
    pthread_mutex_init(&recv_mutex, NULL);
    pthread_cond_init(&recv_cond, NULL);
    for(int i=0;i<10;i++)
    {
       snd_msg.msg[i] = (char *)malloc(5000*sizeof(char));
    }
    for(int i=0;i<10;i++)
    {
       rcv_msg.msg[i] = (char *)malloc(5000*sizeof(char));
    }
    if (type != SOCK_MyTCP)
    {
        return -1;
    }
    fd = socket(domain, SOCK_STREAM, protocol);
    if (fd < 0)
    {
        return -1;
    }
    // if (fd > 0)
    //     printf("Socket created successfully\n");
    st_sockfd = fd;

    return fd;
}

int my_bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)
{
    int ret;
    ret = bind(sockfd, my_addr, addrlen);
    //printf("Binding: %d\n", ret);
    // if (ret >= 0)
    //     printf("Binded successfully\n");
    return ret;
}
int my_listen(int sockfd, int backlog)
{
    int ret;
    ret = listen(sockfd, backlog);
    // if (ret >= 0)
    //     printf("Listening\n");
    return ret;
}
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;
    ret = accept(sockfd, addr, addrlen);
    st_sockfd = ret;
    pthread_mutex_lock(&recv_mutex);
    check = 1;
    pthread_mutex_unlock(&recv_mutex);
    pthread_cond_signal(&recv_cond);
    // printf("Signal sent from accept\n");
    // if (ret >= 0)
    //     printf("Accepted\n");
    return ret;
}
int my_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    int ret;
    ret = connect(sockfd, serv_addr, addrlen);
    st_sockfd = sockfd;
    pthread_mutex_lock(&recv_mutex);
    check = 1;
    pthread_mutex_unlock(&recv_mutex);
    pthread_cond_signal(&recv_cond);
    // if (ret >= 0)
    //     printf("Connected\n");
    return ret;
}
size_t my_send(int sockfd, const void *buf, size_t len, int flags)
{
    int chk = 0;
    while (1)
    {
        sleep(1);
        //printf("My_send acquired lock\n");
        pthread_mutex_lock(&send_message_mutex);
        //printf("my_send: %d %s\n", snd_msg.count, (char *)buf);
        if (snd_msg.count < 10)
        {
            chk = 1;
            const char *msg = (const char *)buf;
            int temp_len = (int)len;
            // Create a new char which will have first four characters describing the length
            char *new_msg = (char *)malloc((temp_len + 4) * sizeof(char));
            // Copy the length in the first four characters
            sprintf(new_msg, "%04d", temp_len);
            for (int i = 4; i < temp_len + 4; i++)
            {
                new_msg[i] = msg[i - 4];
            }
            // Copy the new message to the send_message table
            for (int i = 0; i < temp_len + 4; i++)
            {
                snd_msg.msg[snd_msg.end][i] = new_msg[i];
            }
            //printf("%s\n", new_msg);
            snd_msg.len[snd_msg.end] = temp_len + 4;
            chk = temp_len + 4;
            snd_msg.count++;
            snd_msg.sockfd[snd_msg.end] = sockfd;
            // print the snd_msg entries done here
            //printf("%s %d %d %d %d\n", snd_msg.msg[snd_msg.end], snd_msg.len[snd_msg.end], snd_msg.count, snd_msg.st, snd_msg.end);
            snd_msg.end = (snd_msg.end + 1) % 10;
        }
        pthread_mutex_unlock(&send_message_mutex);
        //printf("My_send released lock\n");
        if (chk != 0)
        {
            break;
            
        }
    }
    return chk;
}
size_t my_recv(int sockfd, void *buf, size_t len, int flags)
{
    int chk = 0;
    while (1)
    {
        sleep(1);
        pthread_mutex_lock(&receive_message_mutex);
        //printf("Count: %d\n", rcv_msg.count);
        if (rcv_msg.count > 0)
        {
            char *msg = (char *)buf;
            int temp_len = (int)len;
            // Copy the message from the receive_message table

            for (int i = 0; i < temp_len; i++)
            {
                msg[i] = rcv_msg.msg[rcv_msg.st][i];
            }
            //find min
            int mini = temp_len;
            if (rcv_msg.len[rcv_msg.st] < mini)
            {
                mini = rcv_msg.len[rcv_msg.st];
            }
            chk = mini;
            //printf("Message received: %s\n", msg);
            rcv_msg.count--;
            for(int i=0;i<5000;i++)
            {
                rcv_msg.msg[rcv_msg.st][i] = '\0';
            }
            rcv_msg.len[rcv_msg.st] = 0;
            rcv_msg.sockfd[rcv_msg.st] = -1;  
            rcv_msg.st = (rcv_msg.st + 1) % 10;
    
        }
        pthread_mutex_unlock(&receive_message_mutex);
        if (chk != 0)
        {
            break;
          
        }
    }
    return chk;
}
int my_close(int fd)
{
    int ret;
    sleep(5);
    ret = close(fd);
    if(cleaning_up==0)
    { 
        // Close the threads and destroy the mutex
        pthread_cancel(R);
        pthread_cancel(S);
        pthread_join(R,NULL);
        pthread_join(S,NULL);
        pthread_mutex_destroy(&send_message_mutex);
        pthread_mutex_destroy(&receive_message_mutex);
        pthread_mutex_destroy(&recv_mutex);
        pthread_cond_destroy(&recv_cond);
        // free the memory allocated to the send and receive message table
        for (int i = 0; i < 10; i++)
        {
            free(snd_msg.msg[i]);
            free(rcv_msg.msg[i]);
        }
        cleaning_up = 1;
    }
    return ret;
}
// int main()
// {
//     return 0;
// }