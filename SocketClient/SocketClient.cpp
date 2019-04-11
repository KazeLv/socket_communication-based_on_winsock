#include "SocketClient.h"

SocketClient::SocketClient(bool state,WORD version) {
	bIsConnected = state;
	wVersionReq = version;
	initWinSock();
	createSocket();
}

SocketClient::~SocketClient() {
	SocketClient::disconnect();
	WSACleanup();
}

void SocketClient::initWinSock() {
	int ret = WSAStartup(wVersionReq,&wsaData);
	if (ret != 0) cout << "[ϵͳ��Ϣ] WSAStartup() failed!"<<endl;
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		cout << "[ϵͳ��Ϣ] Invalid Winsock version!" << endl;
	}
}

void SocketClient::createSocket() {
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET) {
		WSACleanup();
		cout << "[ϵͳ��Ϣ] socket() failed!" << endl;
	}
}

int SocketClient::connect2server(string serverIp) {
	initWinSock();
	createSocket();
	//���÷�������ַ��Ϣ
	struct sockaddr_in saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(SERVER_PORT);
	saServer.sin_addr.S_un.S_addr = inet_addr(serverIp.c_str());

	//���ӷ�����
	int ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if ( ret == SOCKET_ERROR) {
		cout << "[ϵͳ��Ϣ] ����ʧ��!" << endl;
		bIsConnected = false;
		closesocket(sClient);
		WSACleanup();
	}
	else {
		cout << "[ϵͳ��Ϣ] ���ӳɹ�!" << endl;
		bIsConnected = true;
	}
	return ret;
}

void SocketClient::disconnect() {
	char c;
	shutdown(sClient, SD_BOTH);
	closesocket(sClient);
	bIsConnected = false;
}

int SocketClient::getTime() {
	int ret;
	string msg2send = std::to_string(REQ_TIME);
	ret = send(sClient, msg2send.c_str(), msg2send.length() + 1, 0);
	if (ret == SOCKET_ERROR) cout << "[ϵͳ��Ϣ] ʱ��������ʧ�ܣ�" << endl;
	return ret;
}

int SocketClient::getName() {
	string msg2send = std::to_string(REQ_NAME);
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[ϵͳ��Ϣ] ����������ʧ�ܣ�" << endl;
	return ret;
}

int SocketClient::getClients() {
	string msg2send = std::to_string(REQ_LIST);
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[ϵͳ��Ϣ] �ͻ����б�������ʧ�ܣ�" << endl;
	return ret;
}

int SocketClient::sendMsg(const int id,const string& msg) {
	char strID[3];
	sprintf_s(strID, 3,"%02d", id);					//��ʽ����ţ�2λ
	char strLen[4];	
	sprintf_s(strLen, 4, "%03d", msg.length());		//��ʽ����Ϣ���ȣ�3λ
	//���ݹ�����װ�õ�Ҫ���͵��ַ���
	string msg2send = to_string(REQ_MSG) + string(strLen) + string(strID) + msg;	
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[ϵͳ��Ϣ] ��Ϣ������ʧ��" << endl;
	return ret;
}
