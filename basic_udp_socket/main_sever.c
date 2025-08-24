#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define sever_port 8080

//convert the input string to uppercase
char *convert(char *src){
    char *iter=src;
    char *result=malloc(sizeof(src));
    char *it=result;
    if (iter==NULL){
        return iter;
    }
    
    while (*iter){
        *it++=*iter++ & ~0x20;
    }
    return result;
}

int main(int argc,char *argv[]){
    
    char buf[1024]={0};//buffer for receiving data

    //build  a socket
    int socket_fd=socket(PF_INET,SOCK_DGRAM,0);
    if (socket_fd<0){
        printf("Fail to create a socket\n");
    }

    //server address setting
    struct sockaddr_in server_addr={
        .sin_family=AF_INET,
        .sin_addr.s_addr=INADDR_ANY,
        .sin_port=htons(sever_port),
    };

    //bind the socket to the server address
    if (bind(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        printf("Fail to bind\n");
        close(socket_fd);
        exit(0);
    }

    printf("server is ready\n");

    //define a struct to store the client address
    struct sockaddr_in client_addr;
    int len=sizeof(client_addr);


    while (1){

        //receive data from client
        if (recvfrom(socket_fd,buf,sizeof(buf),0,(struct sockaddr *)&client_addr,&len)<0){
            break;
        }


        if (strcmp(buf,"exit")==0){
            printf("get exit order ,closing the server...\n");
            break;
        }


        char *conv = convert(buf);

        //show ip and port 
        printf("get message from [%s:%d]: ",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        //show convert to uppercase result
        printf("%s -> %s\n",buf,conv);

        //sent back to client
        sendto(socket_fd,conv,strlen(conv),0,(struct sockaddr *)&client_addr,len);

        //clear buffer
        memset(buf,0,sizeof(buf));
        free(conv);

    }

    //close the socket
    if (close(socket_fd)<0){
        printf("Fail to close the socket\n");
    }

    return 0;
}