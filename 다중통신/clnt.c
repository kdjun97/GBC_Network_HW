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
	struct sockaddr_in serv_addr; // sockaddr_in ����üŸ���� serv���� ����
	pthread_t snd_thread, rcv_thread; // thread ���� ����
	void * thread_return; // void������ ����

	// 4���� argument �ʿ�. message + IP + port + name
	if(argc!=4) 
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(name, "[%s]", argv[3]); // name�� [argv[3]]���� �����ؼ� ������
	sock=socket(PF_INET, SOCK_STREAM, 0); // socket�� ������. TCP
	
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) // client�ʿ��� connect ��û.
		error_handling("connect() error");
    // connect�� ����Ǹ� �� ���� connection establishment��. �׶����� wrtie read����

    printf("Welcome to GBC Network Client!\n"); // �Ϸ�Ƚ��� �� print ���
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock); // �ΰ��� ������ ����.
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	// �ϳ��� send_msg�� ���� thread ������ recv_msg�����Ѱ�
	// ���� �Ķ���͸� void�����ͷ� �Ѱ���. �̰� �Լ� �ȿ��� �� ��ȯ�Ҳ�
	// 2��° ���ڴ� default�� NULL��.

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	// ������ ���Ḧ ��ٸ�. snd ������ rcv ������

	close(sock);  // sock ����
	return 0;
}
	

// fgets�� ����� �Է��� �ް�, �� �Է� ���� �޼����� ������ ������.
// ������ ���� �޼����� ��� Ŭ���̾�Ʈ���� �Ѹ���.
// Ŭ���̾�Ʈ�� �������� ���� �޼����� �ڽ��� â�� fputs�� ����Ѵ�.

void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg); // ���� �Ķ���� �� ��ȯ���ִ� �κ�
	char name_msg[NAME_SIZE+BUF_SIZE];
	
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin); // fgets�� ���� �Է��� �޴´� stdin �����ǥ���Է�����
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) // q�� Q��� �޼������?
		{
			close(sock); // ���� ���� 
			exit(0); // ����
		}
		// �Է��� �޽����� q�� Q�� �ƴҶ� (���� �б⿡ �Ȱɸ��ϱ�)

		sprintf(name_msg,"%s %s", name, msg); // name_msg�� name msg�������� ������. %s %s����.
		// name_msg = [name] asldkfjs;lakdfj �̷��� �����.
		write(sock, name_msg, strlen(name_msg)); // sock��(����) write����.
	}
	return NULL;
}

void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg); // ���� �Ķ���� �� ��ȯ���ִ� �κ�
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1); // read�� �дµ� ��ȯ���� ���� byte����.
		// �������� ���� �޽����� ����. �װ� ���߿� fputs��.
		if(str_len==-1) // ������ ������ -1 return
			return (void*)-1;
		name_msg[str_len]=0; // ���� null �־���. for safety
		fputs(name_msg, stdout); // �޽��� ������ ���.
	}
	return NULL;
}
	
void error_handling(char *msg) // �ܼ��� error message print �� exit �ϴ� �Լ�
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
