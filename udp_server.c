#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SERV_PORT 5134
#define MAXNAME 1024
#define CHUNK_SIZE 512
#define HEADER_SIZE 5 // 4 bytes for block_number, 1 byte for status

extern int errno;

int main() {
    int socket_fd;
    int nbytes;
    char buf[MAXNAME];
    char send_buf[CHUNK_SIZE + HEADER_SIZE];
    struct sockaddr_in myaddr; // 伺服器地址
    struct sockaddr_in client_addr; // 客戶端地址
    int length;
    int fdesc; // 文件描述符
    int block_number;

    // 創建 UDP 套接字
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // 設置伺服器地址
    bzero((char*)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(SERV_PORT);

    // 綁定地址
    if (bind(socket_fd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind error");
        exit(1);
    } else {
        printf("bind success\n");
    }

    // 持續監聽
    length = sizeof(client_addr);
    printf("Server is ready to receive!!\n");
    printf("Can strike Ctrl-C to stop the server \n");
    while (1) {
        // 接收客戶端發來的文件名
        if ((nbytes = recvfrom(socket_fd, buf, MAXNAME-1, 0, (struct sockaddr*)&client_addr, (socklen_t*)&length)) < 0) {
            perror("could not read datagram!!");
            continue;
        }
        buf[nbytes] = '\0'; // 確保字符串終止
        printf("Received filename from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buf);

        // 打開文件
        fdesc = open(buf, O_RDONLY);
        if (fdesc == -1) {
            // 文件打開失敗，發送錯誤訊息
            bzero(send_buf, CHUNK_SIZE + HEADER_SIZE);
            *(int*)send_buf = htonl(0); // block_number = 0
            send_buf[4] = 2; // status = 2 (error)
            snprintf(send_buf + HEADER_SIZE, CHUNK_SIZE, "Error: Could not open file %s", buf);
            if (sendto(socket_fd, send_buf, HEADER_SIZE + strlen(send_buf + HEADER_SIZE), 0, (struct sockaddr*)&client_addr, length) < 0) {
                perror("could not send error datagram!!");
            }
            continue;
        }

        // 分塊讀取並發送文件
        block_number = 0;
        while (1) {
            bzero(send_buf, CHUNK_SIZE + HEADER_SIZE);
            nbytes = read(fdesc, send_buf + HEADER_SIZE, CHUNK_SIZE);
            if (nbytes < 0) {
                // 讀取錯誤
                *(int*)send_buf = htonl(block_number); // block_number
                send_buf[4] = 2; // status = 2 (error)
                snprintf(send_buf + HEADER_SIZE, CHUNK_SIZE, "Error: Could not read file %s", buf);
                if (sendto(socket_fd, send_buf, HEADER_SIZE + strlen(send_buf + HEADER_SIZE), 0, (struct sockaddr*)&client_addr, length) < 0) {
                    perror("could not send error datagram!!");
                }
                break;
            } else if (nbytes == 0) {
                // 文件結束
                *(int*)send_buf = htonl(block_number); // block_number
                send_buf[4] = 1; // status = 1 (end of file)
                if (sendto(socket_fd, send_buf, HEADER_SIZE, 0, (struct sockaddr*)&client_addr, length) < 0) {
                    perror("could not send end datagram!!");
                }
                break;
            } else {
                // 發送文件塊
                *(int*)send_buf = htonl(block_number); // block_number
                send_buf[4] = 0; // status = 0 (data)
                if (sendto(socket_fd, send_buf, HEADER_SIZE + nbytes, 0, (struct sockaddr*)&client_addr, length) < 0) {
                    perror("could not send data datagram!!");
                    break;
                }
                block_number++;
            }
        }
        close(fdesc);
        printf("Sent file to client\n");
        printf("Can strike Ctrl-C to stop the server \n");
    }

    close(socket_fd);
    return 0;
}