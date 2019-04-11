#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h> //�̺߳�����
#include <windows.h> //windows api��
#include <string.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib") //���� ws2_32.dll�����ú�֮��Ͳ��ù���Ŀ������

#define SERVER_PORT 2418  //ѧ�ź���λ���������˿�
#define PAC_TYPE_LEN 1    //���ݰ������ֶγ���Ϊ1
#define PAC_CONTENT_LEN 3 //���ݰ����ݳ����ֶ�Ϊ3

#define PAC_ERROR 0
#define PAC_RIGHT 1

//�������ݰ����
#define REQ_TIME "0" //ʱ������
#define REQ_NAME "1" //��������
#define REQ_LIST "2" //�б�����
#define REQ_MSG "3"  //��Ϣ����
//��Ӧ���ݰ����
#define RES_TIME "4" //ʱ����Ӧ
#define RES_NAME "5" //������Ӧ
#define RES_LIST "6" //�б���Ӧ
#define RES_MSG "7"  //��Ϣ��Ӧ
//ָʾ���ݰ���ʽ
#define COM_MSG "8" //��Ϣָʾ

#define MAX_NUM 20 //����������20��������

int pacRecv(struct clientlist *pM, char *ptr, int length);        //���߳���Ϣ���պ���
int pacSend(struct clientlist *pM, char *pactype);                //���߳���Ϣ���ͺ���
int pacTransMessage(struct clientlist *pM, char *ptr, int *dest); //������Ϣ�������ݰ��ĺ������ݽ���
void getTime(char *timestring);                                   //��ȡ������ϵͳʱ��
unsigned int __stdcall exitCheck(PVOID pM);                       //�����źż���߳�
unsigned int __stdcall ThreadFun(PVOID pM);                       //��Ӧ�ͻ����̺߳���

bool exitsignal = false;		 //ȫ�ֵĽ����˳��ź�
int clientcount = 0;			 //�Ѵ����߳���
struct clientlist
{
	struct sockaddr_in saClient; //�ͻ��˵�ַ�˿ںŵ���Ϣ
	SOCKET sServer;              //�����Ϊ�ÿͻ��˴������׽���
	int clientnumebr;            //�ͻ��˱��
	bool isalive = false;        //�û����Ƿ���
	bool prosignal = false;      //�ͻ��˽ṹ���ڲ����˳��źţ������˳������ͻ���
} saClientlist[MAX_NUM];

//�����̺߳���
unsigned int __stdcall ThreadFun(PVOID pM)
{
	struct  clientlist* curClient = (struct clientlist*)pM;
	int ret; //������Ϣ����ֵ
	int nLeft = 0;
	char hlomsg[] = "9005Hello";		//9�൱�������ݰ���־λ,005��ʾ���ݳ���
	char message[1000] = "";
	char pactype[PAC_TYPE_LEN + 1];      //���ݰ������ַ���1λ�����һλ\0��־�ַ�������
	char paclength[PAC_CONTENT_LEN + 1]; //���ݰ����ݳ����ֶ�3λ�����һλ\0��־�ַ�������
	pactype[PAC_TYPE_LEN] = '\0';        //��ǰд���ַ���������
	paclength[PAC_CONTENT_LEN] = '\0';   //��ǰд���ַ���������

	//˵һ����Ҫ���Է��ͻ��˷���Hello��Ϣ��
	printf("Server say Hello to client: %s:%d\n", inet_ntoa((curClient->saClient.sin_addr)), ntohs((curClient->saClient).sin_port));
	printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");

	//���̷߳���һ��Hello��Ϣ���ͻ���
	ret = send(((struct clientlist *)pM)->sServer, (char *)&hlomsg, sizeof(hlomsg), 0);
	if (ret == SOCKET_ERROR)
	{
		printf("Send() failed!\n"); //������Ϣʧ��
	}

	while (true)
	{
		//����յ���ǰ�߳��˳����źŻ��յ�ȫ���߳��˳��ź�
		if (curClient->prosignal == true || exitsignal == true)
		{                                       //������߳��յ������źţ�����ֹ�������̣߳����߳���return��ʽ��ֹ����Ϊ��ȫ�ķ�ʽ
			curClient->isalive = false;
			closesocket(curClient->sServer);	//�ر������׽���
			return -1;
		}

		ret = pacRecv(curClient, (char *)&pactype, PAC_TYPE_LEN); //ָ����Ϣ���ܺ���ȥ���������ַ����ȵ����ݰ������ֶ�

		//��pacRecv����û�м�⵽Զ���˳��źŵ�ʱ����ȥ�������ݰ�����Ȼ��෢һ��
		if (!(curClient->prosignal == true || exitsignal == true))
			pacSend((struct clientlist *)pM, pactype);
	}

	return 0;
}

//��Ϣ���պ�������һ�������������׽��֣��ڶ����������ַ�ָ�룬����������ָʾҪ���յ���Ϣ���ͣ����ĸ�������ʾ��������Ϣ�ĳ���
int pacRecv(struct clientlist *pM, char *ptr, int length)
{
	int nLeft = length; //��ȡ�����ַ�����ֻ��Ҫһ���ַ�����
	int ret;            //��Ϣ����ֵ
	int rflag;          //��־����ֵ

	while (nLeft > 0)
	{
		//��������
		ret = recv(pM->sServer, ptr, nLeft, 0);
		if (ret == SOCKET_ERROR || ret == 0)
		{
			pM->prosignal = true; //֪ͨ�߳��˳�
			printf("Client Number:%d has closed the connection!\n", pM->clientnumebr);
			rflag = PAC_ERROR;
			break;
		}

		nLeft -= ret;
		ptr += ret;
	}

	if (!nLeft)            //�����ʽ�׽��ִ��������Ѿ����
		rflag = PAC_RIGHT; //ָʾ��ȷ���أ������ַ�����������message����
	return rflag;
}

//��Ϣ���ͺ��������������������������Ӧ�Ķ���
int pacSend(struct clientlist *pM, char *pactype)
{
	int rflag;       //ָʾ�����Ƿ�ɹ�
	int ret;         //���Է���ֵ
	int length = 0;  //��Ӧ���ݰ������ֶ�ҲҪ���ͳ�ȥ��Ĭ�ϳ���Ϊ0
	char slength[5]; //��¼���ȵ��ַ���,����Ϊ5���ֽ�
	char message[1000] = { 0 };
	char newmsg[100] = { 0 };
	char fmessage[1050] = { 0 };
	int dest;
	char timestring[25] = { 0 }; //��ȡʱ���ֶ���Ϣ

	if (strcmp(pactype, REQ_TIME) == 0) //���������ʱ������ݰ���ϵͳ���û�ȡʱ��
	{
		getTime(timestring);
		strcat(message, RES_TIME);        //��־ʱ����Ӧ���ݰ�
		length = strlen(timestring);      //���ݳ���
		sprintf(slength, "%03d", length); //����תΪ��λ�ַ�����������ǰ�油0
		strcat(message, slength);
		strcat(message, timestring); //������ɣ��õ���װ�õ����ݰ�message
		
		printf("Server time: %s\n", timestring);
		printf("Client Number %d requestes system time on server!\n", pM->clientnumebr); //server�ϴ�ӡ˵�ĸ��ͻ��������˷������ϵ�ϵͳʱ��
		ret = send(pM->sServer, (char *)&message, strlen(message), 0);                   //��������ʱ������ݰ�
		printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");
	}
	else if (strcmp(pactype, REQ_NAME) == 0) //�����������������ֵ����ݰ���ϵͳ���û�ȡ����
	{
		char hostname[100] = { 0 };
		if (gethostname(hostname, sizeof(hostname)) < 0)
		{
			//���󲢷���
		}
		printf("Hostname on server:%s\n", hostname);
		length = strlen(hostname);                                                    //��hostname�ĳ���
		sprintf(slength, "%03d", length);                                             //������ת����λ�ַ���������ǰ�油0
		strcat(message, RES_NAME);                                                    //�����Ӧ���ݰ���־λ
		strcat(message, slength);                                                     //�����Ӧ���ݰ������ֶ�
		strcat(message, hostname);                                                    //�����Ӧ���ݰ������ֶ�
		printf("Client Number %d requestes hostname on server!\n", pM->clientnumebr); //server�ϴ�ӡ�ĸ��ͻ��������˷�����������
		printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");
		ret = send(pM->sServer, (char *)&message, strlen(message), 0);                //�����������ֵ����ݰ�
	}
	else if (strcmp(pactype, REQ_LIST) == 0)
	{                               //��������������б�����ݰ����г����е����ӷ������б�
		int ccount = 0;				//���ŵ�������
		strcat(fmessage, RES_LIST); //��־�б���Ӧ���ݰ�
		for (int i = 0; i < MAX_NUM; i++)
		{
			if (saClientlist[i].isalive == true)
			{ //����Ǵ��Ŀͻ��ˣ�����ͻ�����Ϣ
				if (ccount == 0) {	//����ǵ�һ��Ҫ��ӡ��
					sprintf(newmsg, "%d-%s-%d", saClientlist[i].clientnumebr, inet_ntoa(saClientlist[i].saClient.sin_addr), ntohs(saClientlist
						[i].saClient.sin_port));
					strcat(message, newmsg);
				}
				else {	//���ǰ���Ѿ�����һЩ�ֶ�
					sprintf(newmsg, "_%d-%s-%d", saClientlist[i].clientnumebr, inet_ntoa(saClientlist[i].saClient.sin_addr), ntohs(saClientlist
						[i].saClient.sin_port));
					strcat(message, newmsg);
				}
				ccount++;
			}
		}
		sprintf(slength, "%03d", strlen(message));
		strcat(fmessage, slength);
		strcat(fmessage, message);
		printf("Client Number %d requestes clientlist on server!\n", pM->clientnumebr); //server�ϴ�ӡ˵�ĸ��ͻ������������ӷ������б�
		printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");
		ret = send(pM->sServer, (char *)&fmessage, strlen(fmessage), 0);
	}
	else if (strcmp(pactype, REQ_MSG) == 0) //�������Ϣת����������ݰ�����Ҫ�����ֶκ����ݣ�������ȡ�����ֶ�
	{
		getTime(timestring);
		length = strlen(timestring);                                        //���ݳ���
		pacTransMessage(pM, (char *)&message, &dest);                       //������sServer�׽��ֽ�����Ϣ������ŵ�message���沢����
		strcat(fmessage, COM_MSG);                                          //��־Ϊָʾ���ݰ�
		sprintf(slength, "%03d", strlen(message) + 4 + strlen(timestring)); //3λ��ӡ��Ϣ����
		strcat(fmessage, slength);                                          //�ַ������ӣ����ϳ�����
		sprintf(newmsg, "%02d", pM->clientnumebr);                          //Դ�ͻ��˵ı��
		strcat(fmessage, newmsg);                                           //��������ֽ�
		strcat(fmessage, "_");                                              //���»��߷ָ�
		strcat(fmessage, timestring);                                       //ʱ���ֶ�
		strcat(fmessage, "_");
		strcat(fmessage, message); //�������ݣ���ȫ���ݰ�
		printf("Client number: %d say %s to client number: %d\n", pM->clientnumebr, message, dest);	//�������ϴ�ӡһ�·�����ʲô��Ϣ

		if (saClientlist[dest].isalive == true)
		{
			ret = send(saClientlist[dest].sServer, (char *)&fmessage, strlen(fmessage), 0); //�������ݰ���Ŀ������
			if (ret == SOCKET_ERROR)
				printf("Send failed!\n");
			sprintf(message, "%s", "7013Send Succeed!");
			ret = send(pM->sServer, (char *)&message, strlen(message), 0); //������Ϣ���ͻ��˱�ʾ���ͳɹ�
			if (ret == SOCKET_ERROR)
				printf("Send failed!\n");
		}
		else
		{
			sprintf(message, "%s", "7012Send Failed!");
			ret = send(pM->sServer, (char *)&message, strlen(message), 0); //������Ϣ���ͻ��˱�ʾ����ʧ��
		}
	}
	else //���ɶ�����ǣ����������ݰ���ʽ����
		;
	rflag = 0;
	return rflag;
}

//��ȡת���������ݰ��ĺ�������
int pacTransMessage(struct clientlist *pM, char *ptr, int *dest)
{
	int nLeft = PAC_CONTENT_LEN + 2; //�����ֶ���λ������λ��ʾĿ�ĵ�ַ
	int ret;
	int rflag;
	char sig[6] = { 0 }; //��ȡ�ź���Ϣ
	char *sigptr = sig;

	//������һ��ѭ�������պ�����λ���ֱ��ǣ�ǰ��λ��ʾ���ݰ����ȣ�����λ��ʾĿ�ĵ�ַ
	while (nLeft > 0)
	{
		//��������
		ret = recv(pM->sServer, sigptr, nLeft, 0);
		//Զ�˶Ͽ����ӵ��ź�
		if (ret == SOCKET_ERROR || ret == 0)
		{
			pM->prosignal = true;
			//printf("Recv failed!\n");
			printf("Client Number:%d has closed the connection!\n", pM->clientnumebr);
			rflag = PAC_ERROR;
			break;
		}

		nLeft -= ret;
		sigptr += ret;
	}

	*dest = atoi(sig + 3); //ȡĿ�Ŀͻ��˱��
	sig[3] = '\0';
	nLeft = atoi(sig);

	//�����ڶ���ѭ������ȡ��ת����Ϣ������
	while (nLeft > 0)
	{
		//��������
		ret = recv(pM->sServer, ptr, nLeft, 0);
		if (ret == SOCKET_ERROR)
		{
			pM->prosignal = true;
			printf("Client Number:%d has closed the connection!\n", pM->clientnumebr);
			rflag = PAC_ERROR;
			break;
		}

		if (ret == 0)
		{
			pM->prosignal = true;
			printf("Client Number:%d has closed the connection!\n", pM->clientnumebr);
			rflag = PAC_ERROR;
		}
		nLeft -= ret;
		ptr += ret;
	}

	if (!nLeft)            //�����ʽ�׽��ִ��������Ѿ����
		rflag = PAC_RIGHT; //ָʾ��ȷ���أ������ַ�����������message����

	return rflag;
}

//��ȡϵͳʱ�䣬����ָ�룬д���ַ�������
void getTime(char *timestring)
{
	time_t now;
	struct tm *tm_now;
	time(&now);
	tm_now = localtime(&now);
	char year[5], month[3], day[3], hour[3], minute[3], second[3];
	itoa(tm_now->tm_year + 1900, year, 10);
	itoa(tm_now->tm_mon + 1, month, 10);
	itoa(tm_now->tm_mday, day, 10);
	itoa(tm_now->tm_hour, hour, 10);
	itoa(tm_now->tm_min, minute, 10);
	itoa(tm_now->tm_sec, second, 10);
	strcat(timestring, year);
	strcat(timestring, "-"); //����ʱ�䵥λ֮����һ��'-'���ָ�
	strcat(timestring, month);
	strcat(timestring, "-");
	strcat(timestring, day);
	strcat(timestring, "-");
	strcat(timestring, hour);
	strcat(timestring, "-");
	strcat(timestring, minute);
	strcat(timestring, "-");
	strcat(timestring, second);
	return;
}

//����˳��źŵ��̣߳�������̼߳�⵽�˳��źţ�֪ͨ�����߳̽����Լ��������˳�
unsigned int __stdcall exitCheck(PVOID pM)
{
	char flag;
	while (true)
	{
		scanf_s("%c", &flag);
		if (flag == 'q')
		{
			exitsignal = true; //֪ͨȫ���߳̽���
			return 0;
		}
	}
	return 0;
}

void main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret, length;                       //ret���ո�����������ֵ
	SOCKET sListen, sServer;               //�����׽��ֺ������׽��֣������׽���Ҫ�������׽���������
	struct sockaddr_in saServer, saClient; //�������Ϳͻ��˵ĵ�ַ��Ϣ
	char *ptr;                             //����������
	const int THREAD_NUM = MAX_NUM;        //����������ӵĿͻ�����
	HANDLE handle[THREAD_NUM];             //���ؾ������
	HANDLE exitprocess;                    //����˳��źŵ��߳�
	int clientcount = 0;
	SOCKET clientlist[THREAD_NUM];             //������ٸ��߳̾��������ٸ�socket
	struct sockaddr_in clientaddr[THREAD_NUM]; //��¼�ͻ��˵�ַ��Ϣ���������ٸ��߳̾�������ڶ��ٸ��ͻ���

	//WinSock��ʼ��
	wVersionRequested = MAKEWORD(2, 2); //ϣ��ʹ�õ�WinSock DLL�汾
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return;
	}

	//ȷ��WinSock DLL֧�ְ汾2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid WinSock Version!\n");
		return;
	}

	//����socket��ʹ��TCPЭ�飬����֤�Ƿ���Ч�����޸�
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}

	//����������Ϣ
	saServer.sin_family = AF_INET;                     //��ַ����
	saServer.sin_port = htons(SERVER_PORT);            //ת���������ֽ���
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //ָʾ�����ַ

	//��
	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);
		WSACleanup();
		return;
	}

	//������������
	ret = listen(sListen, MAX_NUM); //���ӵȴ����г���Ϊ20����������������
	if (ret == SOCKET_ERROR)
	{
		printf("listen() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);
		WSACleanup();
		return;
	}

	//��ʾ���
	printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");
	printf("Waiting for client connecting!\n");
	printf("tips : Ctrl+c to quit!\n");
	printf("* * * * * * * * * * * * * * * * * * * * * * * *\n");

	length = sizeof(saClient);

	//�����ȴ��ͻ�������
	while (true)
	{
		//����ܵ��߳��˳��ź�Ϊtrue��������ѭ�������������ڵȴ����ӵĵط�
		if (exitsignal == true)
			break;
		sServer = accept(sListen, (struct sockaddr *)&saClient, &length);
		if (sServer == INVALID_SOCKET)
		{
			printf("accept() failed! code:%d Please connect again!\n", WSAGetLastError());
			continue; //�������ʧ�ܣ��������һ��ѭ���ȴ�
		}
		printf("Accepted Client: %s:%d\n", inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port)); //������ӳɹ�����ӡ���ӳɹ��Ŀͻ�����Ϣ
		saClientlist[clientcount].sServer = sServer;
		saClientlist[clientcount].clientnumebr = clientcount;                                                  //client���
		saClientlist[clientcount].isalive = true;                                                              //��־Ϊ����client
		saClientlist[clientcount].saClient = saClient;                                                         //���client�ĵ�ַ�ṹ��Ϣ
		handle[clientcount] = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, &saClientlist[clientcount], 0, NULL); //�߳̾��
		clientcount++;                                                                                         //�ͻ��˼�����1
	}

	//���߳�Ҫ�ȴ������߳�ִ��������˳������ô˺����ȴ������߳�ִ����
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE); //�������߳�δ��ֹ���������ֹ���߳����ڽ���

	closesocket(sListen); //�رռ����׽���
	closesocket(sServer); //�ر������׽���
	WSACleanup();         //�ر�Winsock DLL
}