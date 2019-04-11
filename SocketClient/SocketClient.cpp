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
	if (ret != 0) cout << "[系统消息] WSAStartup() failed!"<<endl;
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		cout << "[系统消息] Invalid Winsock version!" << endl;
	}
}

void SocketClient::createSocket() {
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET) {
		WSACleanup();
		cout << "[系统消息] socket() failed!" << endl;
	}
}

int SocketClient::connect2server(string serverIp) {
	initWinSock();
	createSocket();
	//配置服务器地址信息
	struct sockaddr_in saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(SERVER_PORT);
	saServer.sin_addr.S_un.S_addr = inet_addr(serverIp.c_str());

	//连接服务器
	int ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if ( ret == SOCKET_ERROR) {
		cout << "[系统消息] 连接失败!" << endl;
		bIsConnected = false;
		closesocket(sClient);
		WSACleanup();
	}
	else {
		cout << "[系统消息] 连接成功!" << endl;
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
	if (ret == SOCKET_ERROR) cout << "[系统消息] 时间请求发送失败！" << endl;
	return ret;
}

int SocketClient::getName() {
	string msg2send = std::to_string(REQ_NAME);
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[系统消息] 名字请求发送失败！" << endl;
	return ret;
}

int SocketClient::getClients() {
	string msg2send = std::to_string(REQ_LIST);
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[系统消息] 客户端列表请求发送失败！" << endl;
	return ret;
}

int SocketClient::sendMsg(const int id,const string& msg) {
	char strID[3];
	sprintf_s(strID, 3,"%02d", id);					//格式化编号，2位
	char strLen[4];	
	sprintf_s(strLen, 4, "%03d", msg.length());		//格式化消息长度，3位
	//根据规则组装得到要发送的字符串
	string msg2send = to_string(REQ_MSG) + string(strLen) + string(strID) + msg;	
	int ret = send(sClient, msg2send.c_str(), msg2send.length(), 0);
	if (ret == SOCKET_ERROR) cout << "[系统消息] 消息请求发送失败" << endl;
	return ret;
}
