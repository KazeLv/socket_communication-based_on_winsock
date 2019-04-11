#include <winsock2.h>
#include <cstdlib>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <deque>
#include <string>

#define SERVER_PORT 0614
#define LENGTH_BITS 3

using namespace std;

enum MsgType {
	//�������ݰ����
	REQ_TIME,		//ʱ������0
	REQ_NAME,		//��������1
	REQ_LIST,		//�б�����2
	REQ_MSG,		//��Ϣ����3
	//��Ӧ���ݰ����
	RES_TIME,		//ʱ����Ӧ��4
	RES_NAME,		//������Ӧ��5
	RES_LIST,		//�б���Ӧ��6
	RES_MSG,		//��Ϣ��Ӧ��7
	//ָʾ���ݰ����
	COM_MSG,		//��Ϣָʾ��8
	
	RES_CON			//������Ӧ��9
};

struct Msg {		//���ݰ��ṹ
	MsgType msgType;		//���ݰ�����
	string content;			//���ݰ�����
};

class SocketClient{
private:
	bool bIsConnected;		//���ӱ�־λ
	WORD wVersionReq;		//�汾����
	WSADATA wsaData;		//socket����
	SOCKET sClient;			//�ͻ���socket
public:
	SocketClient(bool state = false, WORD version = MAKEWORD(2, 2));
	~SocketClient();

	SOCKET& getSocket() { return sClient; }

	bool isConnected() { return bIsConnected; }		//�õ�����״̬
	void initWinSock();								//��ʼ���׽���
	void createSocket();							//�����׽���
	int connect2server(string serverIp);			//���ӵ��ͻ���
	void disconnect();								//�Ͽ�����
	int getTime();									//��ȡ������ʱ��
	int getName();									//��ȡ������������
	int getClients();								//��ȡ�ͻ����б�
	int sendMsg(const int targetID,const string& msg);	//�������ͻ��˷�����Ϣ
};