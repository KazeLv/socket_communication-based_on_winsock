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
	//请求数据包编号
	REQ_TIME,		//时间请求，0
	REQ_NAME,		//名字请求，1
	REQ_LIST,		//列表请求，2
	REQ_MSG,		//消息请求，3
	//响应数据包编号
	RES_TIME,		//时间响应，4
	RES_NAME,		//名字响应，5
	RES_LIST,		//列表响应，6
	RES_MSG,		//消息响应，7
	//指示数据包编号
	COM_MSG,		//消息指示，8
	
	RES_CON			//连接响应，9
};

struct Msg {		//数据包结构
	MsgType msgType;		//数据包类型
	string content;			//数据包内容
};

class SocketClient{
private:
	bool bIsConnected;		//连接标志位
	WORD wVersionReq;		//版本需求
	WSADATA wsaData;		//socket数据
	SOCKET sClient;			//客户端socket
public:
	SocketClient(bool state = false, WORD version = MAKEWORD(2, 2));
	~SocketClient();

	SOCKET& getSocket() { return sClient; }

	bool isConnected() { return bIsConnected; }		//得到连接状态
	void initWinSock();								//初始化套接字
	void createSocket();							//创建套接字
	int connect2server(string serverIp);			//连接到客户端
	void disconnect();								//断开连接
	int getTime();									//获取服务器时间
	int getName();									//获取服务器主机名
	int getClients();								//获取客户端列表
	int sendMsg(const int targetID,const string& msg);	//向其他客户端发送消息
};