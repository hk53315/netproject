#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAXLINE 100;
void *threadsend(void *vargp);
void *threadrecv(void *vargp);

int main()
{

	int *clientfdp;
	clientfdp = (int *)malloc(sizeof(int));
	*clientfdp = socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serveraddr;
	bzero((char *)&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(6666);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(*clientfdp,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	{
       	printf("connect error\n");
        exit(1);
	}

	printf("connected\n");
	char temp[100];
    	char *tempsend = "hello";
    	int connfd = *((int *)clientfdp);
    	while(1)
    	{
	    int idata = 0;
	    send(connfd,tempsend,strlen(tempsend),0); 
	    printf("client:hello\n");
	    idata = recv(connfd,temp,100,0);
	    sleep(2);
	    if(idata > 0)
	    {	
	    	printf("server:hi\n");
	    }
    	}
	return EXIT_SUCCESS;
}

