#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100 // BUFSIZE�� 100����
#define MAX_CLNT 256 // Ŭ���̾�Ʈ�� 256������.

// �Լ��� ��������. �� �Լ��� �Ʒ��� ����.
void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

// global variable part
int clnt_cnt=0;
int clnt_socks[MAX_CLNT]; // clnt�� �ϳ��� �ƴϱ� ������ 1���� �迭�� ����.
pthread_mutex_t mutx;
/// end part
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr; // sockaddr_in ����üŸ���� serv,clnt���� ����
	int clnt_adr_sz;

	pthread_t t_id; // thread�� ���� t_id ����
	
	if(argc!=2) // port�� ���������. argument�� �ϳ� �޾ƾ� ��.
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL); // mutex �ʱ�ȭ.
	serv_sock=socket(PF_INET, SOCK_STREAM, 0); // socket�� ����. serv_sock�� return ���� ��. TCP��.

	memset(&serv_adr, 0, sizeof(serv_adr)); // memset�� ���, 0���� �ʱ�ȭ
	serv_adr.sin_family=AF_INET;  // IPv4�� 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY); // �ּ�
	serv_adr.sin_port=htons(atoi(argv[1])); // ��Ʈ
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) // bind
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1) // listen
		error_handling("listen() error");
    
    printf("Welcome to GBC Network Server!\n");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr); // clnt�� adr size 
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz); // clnt sock
		// clinet�� connection ��û�� accept��.
		
		pthread_mutex_lock(&mutx); //lock~unlock������ atomic�ϰ� ����. (������ ����������)
		clnt_socks[clnt_cnt++]=clnt_sock; // �ټ��� client�� ���� �迭�ʱ�ȭ, �迭�� ���� ������ ���� clnt_sock
		pthread_mutex_unlock(&mutx); // unlock
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); // �����带 �����Ѵ�. clnt_sock�̶�� �Ķ���͸� �ѱ�� handle_clnt�Լ��� ��� �����ϴ� ������
		pthread_detach(t_id); // ������ �и�
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg); // �Ķ���ͷ� ���� ���� �� ��ȯ����. thread���� void���������� ����ϴ°ɷ� �˰�����. �׷��� ����ȯ�� �ʿ�.
	int str_len=0, i;
	char msg[BUF_SIZE];
	
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) // �����͸� ������ ���������� �д´�. read�Ȱ� msg�� ��.
		send_msg(msg, str_len); // �� �Լ� ����. �Ʒ��� ����.
	
	pthread_mutex_lock(&mutx); // mutex lock���� ���´�.
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client, �ټ��� Ŭ���̾�Ʈ��ŭ �ݺ��� ���ư�.
	{
		if(clnt_sock==clnt_socks[i]) // [i]��° Ŭ���̾�Ʈ = sock�� ���ȴٸ�
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx); // unlock
	close(clnt_sock); // socket ����
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx); // lock (�޼��� �Ȱ�ġ�� ���� atomic�ϰ� ������)
	for(i=0; i<clnt_cnt; i++) // �ټ��� Ŭ���̾�Ʈ��ŭ
		write(clnt_socks[i], msg, len); // msg�� ����.
	pthread_mutex_unlock(&mutx); // unlock
}
void error_handling(char * msg) // �׳� error �޼��� ������ְ� exit
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
