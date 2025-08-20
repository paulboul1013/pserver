#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SERV_PORT 5134
#define MAXDATA 1024
#define CHUNK_SIZE 512
#define HEADER_SIZE 5 // 4 bytes for block_number, 1 byte for status

extern int errno;

int main(int argc, char **argv) {
    int fd;
    int size;
    char filename[MAXDATA];
    char recv_buf[MAXDATA];
    struct hostent *hp;
    struct sockaddr_in myaddr;
    struct sockaddr_in servaddr;
    int fdesc; // 輸出文件描述符
    int block_number;
    char status;
    char output_filename[] = "output.html"; // 固定輸出文件名

    // 檢查參數
    if (argc < 3) {
        fprintf(stderr, "Usage: %s host_name(IP address) filename\n", argv[0]);
        exit(2);
    }

    // 創建 UDP 套接字
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed!");
        exit(1);
    }

    // 綁定本地地址
    bzero((char*)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);

    if (bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed!");
        exit(1);
    }

    // 設置伺服器地址
    bzero((char*)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    hp = gethostbyname(argv[1]);
    if (hp == 0) {
        fprintf(stderr, "could not obtain address of %s\n", argv[1]);
        exit(1);
    }
    bcopy(hp->h_addr_list[0], (caddr_t)&servaddr.sin_addr, hp->h_length);

    // 發送文件名
    strncpy(filename, argv[2], MAXDATA-1);
    filename[MAXDATA-1] = '\0';
    size = sizeof(servaddr);
    if (sendto(fd, filename, strlen(filename), 0, (struct sockaddr*)&servaddr, size) == -1) {
        perror("write to server error!");
        exit(1);
    }

    // 打開輸出文件
    fdesc = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdesc == -1) {
        perror("could not open output file");
        close(fd);
        exit(1);
    }

    // 接收分塊數據
    while (1) {
        int nbytes = recvfrom(fd, recv_buf, MAXDATA, 0, (struct sockaddr*)&servaddr, &size);
        if (nbytes < HEADER_SIZE) {
            perror("read from server error!");
            close(fdesc);
            close(fd);
            exit(1);
        }

        // 解析頭部
        block_number = ntohl(*(int*)recv_buf);
        status = recv_buf[4];

        if (status == 1) {
            // 文件結束
            printf("File transfer completed, received %d blocks\n", block_number);
            break;
        } else if (status == 2) {
            // 錯誤訊息
            recv_buf[nbytes] = '\0';
            printf("Server error: %s\n", recv_buf + HEADER_SIZE);
            break;
        } else if (status == 0) {
            // 寫入文件數據
            if (write(fdesc, recv_buf + HEADER_SIZE, nbytes - HEADER_SIZE) < 0) {
                perror("could not write to output file");
                break;
            }
            printf("Received block %d\n", block_number);
        } else {
            printf("Unknown status %d\n", status);
            break;
        }
    }

    close(fdesc);
    close(fd);

    // 打開預設瀏覽器顯示 HTML 文件
    if (status == 1) {
        #ifdef __linux__
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "xdg-open %s", output_filename);
            system(cmd);
        #elif __APPLE__
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "open %s", output_filename);
            system(cmd);
        #elif _WIN32
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "start %s", output_filename);
            system(cmd);
        #else
            printf("Please open %s in a browser manually\n", output_filename);
        #endif
    }

    return 0;
}