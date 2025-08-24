#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define server_port 8080
#define server_ip "127.0.0.1" //localhost

int main(){
    char buf[1024]={0}; //buffer for sending data
    char recvbuf[1024]={0}; //buffer for receiving data

    //build a socket
    int socket_fd=socket(PF_INET,SOCK_DGRAM,0);
    if (socket_fd<0){
        printf("Fail to create a socket\n");
        return -1;
    }

    //server address setting
    struct sockaddr_in server_addr={
        .sin_family=AF_INET,
        .sin_addr.s_addr=inet_addr(server_ip),
        .sin_port=htons(server_port),
    };

    int len=sizeof(server_addr);

    while (1){

        printf("input your message: ");
        scanf("%s",buf);

        //send data to server
        sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr *)&server_addr,len);

        //input exit to close client 
        if (strcmp(buf,"exit")==0){
            break;
        }

        //clear input buffer
        memset(buf,0,sizeof(buf));

        if (recvfrom(socket_fd,recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&server_addr,&len)<0){
            printf("recvfrom data from %s:%d, failed!\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
            break;
        }

        //show receive message from server
        printf("get receive message from [%s:%d]: %s\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), recvbuf);

        //clear receive buffer
        memset(recvbuf,0,sizeof(recvbuf));

        //clear sending buffer
        memset(buf,0,sizeof(buf));

    }

    if (close(socket_fd)<0){
        printf("Fail to close the socket\n");
    }

    return 0;
}