#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr; // sockaddr_in 구조체타입의 serv변수 생성
	pthread_t snd_thread, rcv_thread; // thread 변수 생성
	void * thread_return; // void포인터 생성

	// 4개의 argument 필요. message + IP + port + name
	if(argc!=4) 
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(name, "[%s]", argv[3]); // name에 [argv[3]]형식 지정해서 저장함
	sock=socket(PF_INET, SOCK_STREAM, 0); // socket을 생성함. TCP
	
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) // client쪽에서 connect 요청.
		error_handling("connect() error");
    // connect가 연결되면 그 순간 connection establishment됨. 그때부터 wrtie read가능

    printf("Welcome to GBC Network Client!\n"); // 완료된시점 후 print 출력
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock); // 두개의 쓰레드 생성.
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	// 하나는 send_msg에 대한 thread 나머진 recv_msg에대한거
	// 각각 파라미터를 void포인터로 넘겨줌. 이건 함수 안에서 형 변환할꺼
	// 2번째 인자는 default가 NULL임.

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	// 쓰레드 종료를 기다림. snd 쓰레드 rcv 쓰레드

	close(sock);  // sock 해제
	return 0;
}
	

// fgets로 사용자 입력을 받고, 그 입력 받은 메세지를 서버로 보낸다.
// 서버는 받은 메세지를 모든 클라이언트에게 뿌린다.
// 클라이언트는 서버에게 받은 메세지를 자신의 창에 fputs로 출력한다.

void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg); // 받은 파라미터 형 변환해주는 부분
	char name_msg[NAME_SIZE+BUF_SIZE];
	
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin); // fgets를 통해 입력을 받는다 stdin 사용자표준입력으로
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) // q나 Q라는 메세지라면?
		{
			close(sock); // 소켓 해제 
			exit(0); // 종료
		}
		// 입력한 메시지가 q나 Q가 아닐때 (위의 분기에 안걸리니까)

		sprintf(name_msg,"%s %s", name, msg); // name_msg에 name msg형식으로 저장함. %s %s형식.
		// name_msg = [name] asldkfjs;lakdfj 이렇게 저장됨.
		write(sock, name_msg, strlen(name_msg)); // sock에(서버) write해줌.
	}
	return NULL;
}

void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg); // 받은 파라미터 형 변환해주는 부분
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1); // read는 읽는데 반환값이 읽은 byte수다.
		// 서버에서 보낸 메시지를 읽음. 그걸 나중에 fputs함.
		if(str_len==-1) // 읽을게 없으면 -1 return
			return (void*)-1;
		name_msg[str_len]=0; // 끝에 null 넣어줌. for safety
		fputs(name_msg, stdout); // 메시지 읽은걸 출력.
	}
	return NULL;
}
	
void error_handling(char *msg) // 단순히 error message print 후 exit 하는 함수
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
