//UDP를 이용한 암호 맞추기.
//UDP_server
//암호는 ghost29


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 30

char passwd[7] = "ghost29";
const char x_message[] = "Error!";
const char o_message[] = "Correct!";

void error_handling(char *message);

int main(int argc, char **argv)
{
    passwd[strlen(passwd)]=0;
    int serv_sock;
    char message[BUFSIZE];
    int str_len, num=0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    if(argc!=2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
    if(serv_sock == -1)
        error_handling("UDP Socket Error!");

    //printf("UDP socket Complete\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");

    sleep(3);

    //printf("bind Complete!\n");

    while(1) 
    {
        memset(message, 0, sizeof(message));
        clnt_addr_size=sizeof(clnt_addr);
        sleep(1);
        str_len = recvfrom(serv_sock, message, BUFSIZE, 0,
                (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        
        //printf("Recieve Num : %d/%s \n", num++,message);
        
        if (strcmp(message,passwd)) // correct
        {
            //printf("%s\n",x_message);
            sendto(serv_sock, x_message, strlen(x_message), 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));

        }
        else
        {
            //printf("%s\n",o_message);
            sendto(serv_sock, o_message, strlen(o_message), 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
        }
    }
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}                     
