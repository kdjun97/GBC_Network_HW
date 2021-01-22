#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100 // BUFSIZE는 100까지
#define MAX_CLNT 256 // 클라이언트는 256개까지.

// 함수를 선언해줌. 이 함수는 아래에 있음.
void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

// global variable part
int clnt_cnt=0;
int clnt_socks[MAX_CLNT]; // clnt가 하나가 아니기 때문에 1차원 배열로 선언.
pthread_mutex_t mutx;
/// end part
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr; // sockaddr_in 구조체타입의 serv,clnt변수 생성
	int clnt_adr_sz;

	pthread_t t_id; // thread를 위한 t_id 변수
	
	if(argc!=2) // port를 적어줘야함. argument를 하나 받아야 함.
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL); // mutex 초기화.
	serv_sock=socket(PF_INET, SOCK_STREAM, 0); // socket을 생성. serv_sock에 return 값이 들어감. TCP임.

	memset(&serv_adr, 0, sizeof(serv_adr)); // memset을 사용, 0으로 초기화
	serv_adr.sin_family=AF_INET;  // IPv4로 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY); // 주소
	serv_adr.sin_port=htons(atoi(argv[1])); // 포트
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) // bind
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1) // listen
		error_handling("listen() error");
    
    printf("Welcome to GBC Network Server!\n");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr); // clnt의 adr size 
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz); // clnt sock
		// clinet의 connection 요청을 accept함.
		
		pthread_mutex_lock(&mutx); //lock~unlock까지는 atomic하게 동작. (무조건 독립적으로)
		clnt_socks[clnt_cnt++]=clnt_sock; // 다수의 client를 위한 배열초기화, 배열의 값은 위에서 만든 clnt_sock
		pthread_mutex_unlock(&mutx); // unlock
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); // 쓰레드를 생성한다. clnt_sock이라는 파라미터를 넘기고 handle_clnt함수를 계속 동작하는 쓰레드
		pthread_detach(t_id); // 쓰레드 분리
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg); // 파라미터로 받은 값을 형 변환해줌. thread에서 void포인터형을 줘야하는걸로 알고있음. 그래서 형변환이 필요.
	int str_len=0, i;
	char msg[BUF_SIZE];
	
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) // 데이터를 읽을게 없을때까지 읽는다. read된건 msg에 들어감.
		send_msg(msg, str_len); // 이 함수 실행. 아래에 있음.
	
	pthread_mutex_lock(&mutx); // mutex lock으로 묶는다.
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client, 다수의 클라이언트만큼 반복문 돌아감.
	{
		if(clnt_sock==clnt_socks[i]) // [i]번째 클라이언트 = sock이 열렸다면
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx); // unlock
	close(clnt_sock); // socket 해제
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx); // lock (메세지 안겹치기 위해 atomic하게 가야함)
	for(i=0; i<clnt_cnt; i++) // 다수의 클라이언트만큼
		write(clnt_socks[i], msg, len); // msg를 써줌.
	pthread_mutex_unlock(&mutx); // unlock
}
void error_handling(char * msg) // 그냥 error 메세지 출력해주고 exit
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
