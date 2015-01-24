/**************************************************************************************************/
/* Copyright (C) xxxxxxx.com, SSE@USTC, 2014-2015                                                 */
/*                                                                                                */
/*  FILE NAME             :  server.c                                                             */
/*  PRINCIPAL AUTHOR      :  WangXianggang                                                        */
/*  SUBSYSTEM NAME        :  hi                                                                   */
/*  MODULE NAME           :  lab3                                                                 */
/*  LANGUAGE              :  C                                                                    */
/*  TARGET ENVIRONMENT    :  ANY                                                                  */
/*  DATE OF FIRST RELEASE :  2014/12/03                                                           */
/*  DESCRIPTION           :  *********************************                                    */
/**************************************************************************************************/

/*
 * Revision log:
 *
 * Created by Wxg, 2014/12/23
 */


#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>


#define MAXLINE 5
#define LISTENQ 5
#define SERV_PORT 7777
#define false 0
#define true 1
 
int bWrite = false;
int count = 0;

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
         perror("fcntl(sock,GETFL)");
         exit(1);
    }
    opts= opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
         perror("fcntl(sock,SETFL,opts)");
         exit(1);
    } 
}

static void sig_pro(int signum)
{
    printf("recv signal:%d\n", signum);
}




int main(int argc, char* argv[])
{

    int i, n, listenfd, connfd, nfds;
    char line[MAXLINE + 1];
    socklen_t clilen;                     //epoll_event    
    struct epoll_event ev,events[20];   
    int epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;

    struct sigaction sa;
    sa.sa_flags = SA_RESTART;      //SA_RESART:auto restart the sys call
    sa.sa_handler = sig_pro;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
  
    listenfd= socket(AF_INET, SOCK_STREAM, 0);
    //setting socket not block 
    setnonblocking(listenfd);
    ev.data.fd=listenfd;
    //type of event
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family= AF_INET;
    serveraddr.sin_addr.s_addr= htonl(INADDR_ANY);  
    serveraddr.sin_port=htons(SERV_PORT);
    bind(listenfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(listenfd,LISTENQ);

    printf("Listening...\n");
 
    while(1)
    {       
         //waiting epoll
         nfds=epoll_wait(epfd,events,20,500);
         //deal with epoll
        for(i = 0; i < nfds; ++i)
        {
            if(events[i].data.fd < 0)
            {
                continue;
            }
     
            if(events[i].data.fd == listenfd)            //Listening   
            {
                printf("[conn] events=%d\n", events[i].events);   

                if(events[i].events&EPOLLIN)     //new conneciton
                {
                     do
                     {
                         clilen= sizeof(struct sockaddr);
                         connfd= accept(listenfd,(struct sockaddr *)&clientaddr, &clilen);
                         if(connfd > 0)
                         {
                             printf("[conn] ip=%s port=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));   
                             setnonblocking(connfd);
                             ev.data.fd=connfd;
                             ev.events=EPOLLIN|EPOLLET;
                             epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
                         }
                         else
                         {
                             printf("[conn] errno=%d\n", errno);
                             
                             if(errno == EAGAIN)    //no connection
                             {
                                  break;
                             }
                             else if (errno == EINTR) 
                             {
                                 ;
                             }
                             else
                             {
                                 printf("[conn] close listen because accept fail and errno not equale again or eintr\n");
                           
                                 //re listening and fd
                                 close(events[i].data.fd);
                                 epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                                  
                                   //relistening
                                 listenfd= socket(AF_INET, SOCK_STREAM, 0); 
                                 setnonblocking(listenfd);
                                 ev.data.fd=listenfd;
                                 ev.events=EPOLLIN|EPOLLET; 
                                 epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
                                 bind(listenfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
                                 listen(listenfd,LISTENQ);
                                 break;
                             }
                          }
                      }while (1);
                 }
                 else if (events[i].events&EPOLLERR || events[i].events&EPOLLHUP)   //exception occur
                 {
                      printf("[conn] close listen because epollerr or epollhup %d\n", errno);
                     
                      close(events[i].data.fd);
                      epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
 
                      //re listen
                      listenfd= socket(AF_INET, SOCK_STREAM, 0);
                      setnonblocking(listenfd);
                      ev.data.fd=listenfd;
                      ev.events=EPOLLIN|EPOLLET; 
                      epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
                      bind(listenfd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
                      listen(listenfd,LISTENQ);
                 }
             }
             else  //event connected
             {
                
                 if(events[i].events&EPOLLIN)   //has data
                 {
                      do
                      {
                          n= read(events[i].data.fd, line, MAXLINE);
                          if(n > 0)    //read data succ
                          {
                              ++count;
                              line[n]= '\0';
 
                              if(n < MAXLINE)
                              {
                                   printf("[data] n > 0, recv data,len=%d,data=%s count = %d\n", n, line, count);
                              }
                              else
                              {
                                   printf("[data] n > 0, recv data,len=%d,data=%s count = %d\n", n, line, count);
                              }
                              /* reply hi to client */
                              write(events[i].data.fd, "hi", 2);
                          }
                          else if (n < 0) //read failure
                          {
                              if (errno == EAGAIN)    //no data
                             {
                                 printf("[data] waiting next date\n");
                                 break;
                             }
                             else if(errno == EINTR)
                             {
                                 printf("[data] n < 0, interrupt, errno=%d\n", errno);
                             }
                             else  //client has closed
                             {
                                 printf("[data] n < 0, the client has closed, errno=%d\n", errno);
                            
                                 close(events[i].data.fd);
                                 epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                                 break;
                             }
                         }
                         else if (n == 0) //client has closed
                         {
                             printf("[data] n = 0, the client has closed, errno=%d\n", errno);
                                
                             close(events[i].data.fd);
                             epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                             break;     
                         }
                     }while (1);
                 }
                 else if (events[i].events&EPOLLOUT)           //write permit
                 {
                      printf("[data] epoll out\n");
                     
                      if(events[i].data.u64 >> 32 == 0x01)       //assume 0x01 show closed
                      {
                          close(events[i].data.fd);          
                          epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                      }
                      else  
                      {
                          bWrite= true;
                      }
                 }
                 else if (events[i].events&EPOLLERR || events[i].events&EPOLLHUP)   //Exception occur
                 {
                      printf("[data] conn closed because epollerr or epollhup\n");
                     
                      close(events[i].data.fd);
                      epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                 }
             }
         }   
    }

return 0;
}
