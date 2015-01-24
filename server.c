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

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define HELLO_WORLD_SERVER_PORT    7777 
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 8192
#define FNAME_SIZE 20
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char **argv)
{

    char filepath[FNAME_SIZE];

    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    //创建用于internet的流协议socket
    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        exit(1);
    }
    
    //把socket和socket地址结构绑定起来
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        exit(1);
    }
    
    //server_socket用于监听
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!"); 
        exit(1);
    }

    //定义客户端的socket地址结构client_addr
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    /* 接受一个到server_socket代表的socket的一个连接
     * 如果没有连接请求,就等待到有连接请求--这是accept函数的特性
     * accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
     * new_server_socket代表了服务器和客户端之间的一个通信通道
     * accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
     */
    int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
    if ( new_server_socket < 0)
    {
        printf("Server Accept Failed!\n");
        exit(1);
    }
    char buffer[BUFFER_SIZE];
        
    bzero(buffer, BUFFER_SIZE);
    length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
    if (length <= 0)
    {
        printf("Server Recieve Data Failed!\n");
        exit(1);
    }
        
    strcpy(filepath,buffer);

    if(strcmp(buffer,"")!=0)
    {
        if(send(new_server_socket,filepath,strlen(filepath),0) < 0)
        {
            printf("send filepath info Failed!\n");
        }
        else
        {
            printf("filepath:%s\n",filepath);
        }
    }
        
    length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
    if (length <= 0)
    {
        printf("Server Recieve file name Failed!\n");
        exit(1);
    }

    char file_name[FILE_NAME_MAX_SIZE+1];
    bzero(file_name, FILE_NAME_MAX_SIZE+1);

    strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
        
    FILE * fp = fopen(file_name,"r");
    printf("send %s to client...\n",file_name);
    if(NULL == fp )
    {
        printf("File:\t%s Not Found\n", buffer);
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
        int file_block_length = 0;
        while( (file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
        {
            //printf("file_block_length = %d\n",file_block_length);
            //发送buffer中的字符串到new_server_socket,实际是给客户端
            if(send(new_server_socket,buffer,file_block_length,0)<0)
            {
                printf("Send File:\t%s Failed\n", file_name);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
        }
        fclose(fp);
        printf("File:\t%s Transfer Finished\n",file_name);
    }
    //关闭与客户端的连接
    close(new_server_socket);
    close(server_socket);

    return 0;
}
