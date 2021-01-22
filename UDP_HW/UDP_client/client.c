// UDP를 이용한 암호 맞추기
// UDP_client
// client와 server는 UDP를 이용해서 통신한다.
// server를 실행 후 Client 실행 시 암호를 입력하는 문구가 나온다.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 30
void error_handling(char *message);

int main(int argc, char **argv)
{
    int sock;
    char message[BUFSIZE];
    int str_len, addr_size, i;

    struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;

    if(argc!=3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock=socket(PF_INET, SOCK_DGRAM, 0); // UDP
    if(sock == -1)
        error_handling("UDP Socket Error!");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
    
    char buf[BUFSIZE];
    int len;

    while(1)
    {
        printf("Input passwd :");

        addr_size = sizeof(from_addr);
        scanf("%s",buf);
        len = strlen(buf);
        buf[len] = '\0'; // for safety

        sendto(sock, buf, len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        str_len = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr*)&from_addr, &addr_size);


        if ( strcmp("Correct!", buf) == 0)
        {
            printf("Correct!\n");
            break;
        }

        else
            printf("Error!\n");

    }

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
