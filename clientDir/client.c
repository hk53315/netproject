/**************************************************************************************************/
/* Copyright (C) xxxxxxx.com, SSE@USTC, 2014-2015                                                 */
/*                                                                                                */
/*  FILE NAME             :  client.c                                                             */
/*  PRINCIPAL AUTHOR      :  WangXianggang                                                        */
/*  SUBSYSTEM NAME        :  hi                                                                   */
/*  MODULE NAME           :  lab3                                                                 */
/*  LANGUAGE              :  C                                                                    */
/*  TARGET ENVIRONMENT    :  ANY                                                                  */
/*  DATE OF FIRST RELEASE :  2014/12/03                                                           */
/*  DESCRIPTION           :  *********************************                                    */
/**************************************************************************************************/

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HELLO_WORLD_SERVER_PORT    7777
#define BUFFER_SIZE 8192
#define FILE_NAME_MAX_SIZE 512

time_t t_start, t_end;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./%s fileName\n",argv[0]);
        exit(1);
    }

    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建client_socket代表客户机socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构绑定起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton("127.0.0.1",&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        //printf("Can Not Connect To %s!\n",argv[1]);
        exit(1);
    }
    
    //获取新版本的信息
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    char *s = argv[1];
    if(send(client_socket,s,strlen(s),0) < 0)
    {
        printf("get file error!\n");
    }
    else
    {
        printf("getting file ...\n");
    }
    
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    length = recv(client_socket,buffer,BUFFER_SIZE,0);
    if(length < 0)
    {
        printf("Recieve Data From Server %s Failed!\n", argv[1]);
    }
    else
    {
        if(strcmp(buffer,"no")==0)
        {
            printf("no file valuid!");
        }
        else
        {
            printf("file path:%s\n",buffer);
        }
    }  
    
    send(client_socket,buffer,BUFFER_SIZE,0);
     
    FILE * fp = fopen(buffer,"w");
    if(NULL == fp )
    {
        printf("File:\t%s Can Not Open To Write\n", buffer);
        exit(1);
    }
    
    //从服务器接收数据到buffer中
    bzero(buffer,BUFFER_SIZE);

    t_start=time(NULL);

    while( length = recv(client_socket,buffer,BUFFER_SIZE,0))
    {
        if(length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", argv[1]);
            break;
        }
        int write_length = fwrite(buffer,sizeof(char),length,fp);
        if (write_length<length)
        {
            printf("File:\t%s Write Failed\n", buffer);
            break;
        }
        bzero(buffer,BUFFER_SIZE);    
    }
    printf("Recieve File: %s From Server[%s] Finished\n",buffer, argv[1]);
    
    t_end=time(NULL);
    printf("time is %.0fs\n", difftime(t_end,t_start));

    fclose(fp);
    //关闭socket
    close(client_socket);

    return 0;
}
