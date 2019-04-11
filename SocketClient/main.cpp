#include <iostream>
#include "SocketClient.h"
#include <sstream>
#include <thread>
#include <mutex>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

void printClientList(const string& clientList);				//客户端列表打印函数，根据协议从接受的消息内容中得到客户端列表并打印
void procMsgList(deque<Msg>& msgList, mutex& mx);			//消息队列处理函数，除了转发消息之外的其他类型消息包的处理
void threadComMsg(deque<Msg>& msgList, mutex& mxList, mutex& mxPrint, const bool& exiSignal);	//指示消息处理线程函数，实时显示服务器转发的其他客户端消息
void threadListen(SocketClient& sc, deque<Msg>& msgList, mutex& mx, const bool& exitSignal);	//接收监听线程函数，实时从接收缓冲中获取字符并根据协议组成消息包结构体Msg添加到消息队列中等待处理

int counter = 0;

int main()
{
	deque<Msg> deqMsgList;				//消息队列
	mutex mxMsg;						//消息监听线程互斥对象，防止两个线程同时对消息队列进行操作
	mutex mxPrint;						//指示消息处理互斥对象，防止菜单信息和指示消息内容同时打印混在一起
	bool exitSignal = false;

	SocketClient sClient;				//创建管理客户端的类

	thread listenThread(threadListen, ref(sClient), ref(deqMsgList), ref(mxMsg), ref(exitSignal));		//开启监听线程，当连接成功时从接受从服务器发来的消息
	thread comMsgThread(threadComMsg, ref(deqMsgList), ref(mxMsg), ref(mxPrint), ref(exitSignal));		//开启指示消息监听线程，实时显示从其他客户端发来的消息

	//主菜单处理变量
	int choice;								
	int targetID;
	string ip;
	string msg;
	int ret;

	//显示菜单并根据选择进行相应操作
	//1.连接
	//2.断开
	//3.获取时间
	//4.获取名字
	//5.获取客户端列表
	//6.发送消息
	//7.退出
	while (1) {
		if (!sClient.isConnected()) {		//根据客户端连接状态判断
			cout << "\n* * * * * * * * * * * *\n"
				<< "* 客户端状态：未连接...\n"
				<< "* 1.连接\n"
				<< "* 2.退出\n"
				<< "* * * * * * * * * * * *\n"
				<< "> 输入选项序号以进行选择（按回车键结束）: ";
			while (1) {
				cin >> choice;
				if (choice == 1 || choice == 2) {
					//输入选项符合要求，进行处理
					switch (choice) {
					case 1:
						cout << "> 请输入目标服务器的ip地址: ";
						cin >> ip;
						cout << endl;
						if (sClient.connect2server(ip) != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);		//若连接成功则阻塞等待响应消息，下同
						break;
					case 2:
						if (sClient.isConnected()) {
							sClient.disconnect();
							exitSignal = true;
							listenThread.join();		//等待监听接受线程返回
							comMsgThread.join();		//等待指示消息处理线程返回
							cout << "[系统消息] 子线程退出成功！" << endl;
						}
						exit(0);
						break;
					}
					break;
				}
				else {
					cout << "> 输入有误，请重新输入选择: ";
					cin.clear();				//恢复输入流状态位
					cin.ignore(1024, '\n');		//清空输入缓冲区内容
				}
			}
		}
		else {	//客户端已连接
			mxPrint.lock();				//打印菜单时进行锁定，防止子线程同时输出造成混乱
			cout <<"\n* * * * * * * * * * * *\n"
				<< "* 客户端状态：已连接...\n"
				<< "* 1.连接\n"
				<< "* 2.断开连接\n"
				<< "* 3.获取时间\n"
				<< "* 4.获取名字\n"
				<< "* 5.获取客户端列表\n"
				<< "* 6.发送消息\n"
				<< "* 7.退出\n"
				<< "* * * * * * * * * * * *\n"
				<< "> 输入选项序号以进行选择（按回车键结束）: ";
			mxPrint.unlock();
			while (1) {
				cin >> choice;
				if (choice >= 1 && choice <= 7) {
					//输入选项符合要求，进行处理
					switch (choice) {
					case 1:
						cout << "> 请输入目标服务器的ip地址: ";
						cin >> ip;
						cout << endl;
						if (sClient.connect2server(ip) != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 2:
						sClient.disconnect();
						cout << "[系统消息] 已断开连接！" << endl;
						break;
					case 3:
						cout << endl;
						if (sClient.getTime() != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 4:
						cout << endl;
						if (sClient.getName() != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 5:
						cout << endl;
						if (sClient.getClients() != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 6:
						cout << "> 请输入你想要发送消息的目标客户端序号: ";
						cin >> targetID;
						cout << "> 请输入你想要发送的消息内容: ";
						cin.ignore();
						getline(cin, msg);
						if (sClient.sendMsg(targetID, msg) != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 7:
						if (sClient.isConnected()) {
							sClient.disconnect();
						}
						exitSignal = true;
						listenThread.join();
						comMsgThread.join();
						cout << "[系统消息] 子线程退出成功！" << endl;
						exit(0);
						break;
					}
					break;
				}
				else {
					cout << "> 输入有误，请重新输入选择: ";
					cin.clear();
					cin.ignore(1024, '\n');
				}
			}
		}
	}
	return 0;
}

//消息监听线程函数
void threadListen(SocketClient& sc,deque<Msg>& msgList, mutex& mx, const bool& exitSignal) {
	static enum RecState {
		rec_type,				//类型识别阶段
		rec_length,				//长度识别阶段
		rec_content				//内容识别阶段
	};

	while (sc.isConnected() == false);		//开启后进入循环，当客户端连接成功之后进行消息监听

	Msg msgTemp;
	char c;
	char strLen[3];
	int iLen; 
	MsgType _type;
	string strContent;
	RecState rs = rec_type;
	SOCKET sock = sc.getSocket();
	int cnt = 0;
	while (1) {
		if (exitSignal) return;
		recv(sock, &c, 1, 0);
		switch (rs) {
		case rec_type:		//识别状态处于rec_type，表明未开始接受消息包，则进行消息包类别判断，修改状态为rec_length，开始读取长度字段
			_type= MsgType(c - '0');
			if (_type >= 0 && _type <= 9) {
				msgTemp.msgType = _type;
				rs = rec_length;	//修改状态为rec_length，开始读取长度字段
				break;
			}
			else continue;
		case rec_length:		//识别状态处于rec_length，读取长度字段中
			strLen[cnt++] = c;	//将三位长度字段存放到strLen数组内
			if (cnt == LENGTH_BITS) {		//长度字段读取完毕后使用流转换为int并保存到iLen变量
				cnt = 0;
				stringstream ss(strLen);
				ss >> iLen;
				rs = rec_content;	//修改状态为rec_content，开始读取内容字段
			}
			break;
		case rec_content:		//识别状态处于rec_content，读取内容字段中
			strContent.push_back(c);
			cnt++;
			if (cnt == iLen){	//内容字段读取完毕后将完整的消息包结构添加到消息队列
				cnt = 0;
				msgTemp.content = strContent;
				if (msgTemp.msgType == RES_TIME) counter++;
				mx.lock();
				msgList.push_back(msgTemp);
				mx.unlock();
				strContent.clear();
				rs = rec_type;	//修改状态为rec_type，重新开始等待消息包
			}
			break;
		}
	}
}

//指示消息处理线程函数
void threadComMsg(deque<Msg>& msgList, mutex& mxList, mutex& mxPrint, const bool& exitSignal) {
	string id;
	string time;
	string msg;
	int index_f = 0;
	int index_l;
	bool hasCommand;

	while (!exitSignal) {
		hasCommand = false;
		mxList.lock();
		for (auto i = msgList.cbegin(); i != msgList.cend(); i++) {	
			if (i->msgType == COM_MSG) {
				hasCommand = true;
				break;
			}
		}
		mxList.unlock();
		if (!hasCommand) continue;	//当队列中没有指示消息时,不进行处理
		mxList.lock();		//当对消息列表进行处理时,进行锁定防止其他线程对deque进行操作（可能导致迭代器失效）
		auto iter = msgList.begin();
		while (iter != msgList.end()) {
			if (iter->msgType == COM_MSG) {
				msg = iter->content;
				index_l = msg.find("_", index_f);
				id = msg.substr(index_f, index_l - index_f);			//分离得到源客户端编号
				index_f = index_l + 1;
				index_l = msg.find("_", index_f);
				time = msg.substr(index_f, index_l - index_f);			//分离得到时间
				index_f = index_l + 1;
				msg = msg.substr(index_f);								//分离得到消息内容
				mxPrint.lock();
				cout << "\n[系统消息] 接收到其他客户端消息: " << endl;
				cout << "编号: " + id << endl << "时间（年-月-日-时-分-秒）: " + time << endl << "内容:" + msg << endl;
				mxPrint.unlock();
				iter = msgList.erase(iter);
			}
			else iter++;		//该消息不是指示消息，查看下一个
		}
		mxList.unlock();
	}

}

//消息队列处理函数
void procMsgList(deque<Msg>& msgList, mutex& mx) {
	bool hasResponse=false;
	MsgType type;

	//等待接收到响应消息
	while (!hasResponse) {
		mx.lock();			//锁定互斥锁防止其他线程对msgList进行操作而导致迭代器失效
		auto i = msgList.cbegin();
		while (i != msgList.cend()) {
			type = i->msgType;
			if (type == RES_TIME || type == RES_NAME || type == RES_LIST || type == RES_MSG || type == RES_CON) {
				hasResponse = true;
				break;
			}
			i++;
		}
		mx.unlock();
	}	

	string id;
	string ip;
	string port;
	int index_f = 0;
	int index_l;

	mx.lock();			//开始处理时进行锁定，防止指示消息处理线程对消息队列进行添加/删除导致的迭代器失效
	auto iter = msgList.begin();
	while (iter != msgList.end()) {		//遍历消息列表并根据消息类型做出相应处理
		switch (iter->msgType) {
		case RES_TIME:
			cout << "[服务器响应] 当前服务器时间为（年-月-日-时-分-秒）: " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_NAME:
			cout << "[服务器响应] 服务器主机名为: " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_LIST:
			cout << "[服务器响应] 与本客户端连接到相同服务器的客户端列表如下：" << endl;
			printClientList(iter->content);
			cout << "[系统消息] 打印完毕！" << endl;
			iter = msgList.erase(iter);
			break;
		case RES_MSG:
			cout << "[服务器响应] " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_CON:
			cout << "[服务器响应] " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case COM_MSG:
			iter++;			//忽略指示消息，留给实时响应的线程进行处理
			break;
		}
	}
	mx.unlock();
}

//打印客户端列表函数
void printClientList(const string& clientList) {
	string strClients = clientList;
	vector<string> vecClients;

	string::size_type index_f = 0;
	string::size_type index_l;
	
	//将长字符串分割为表示一个个客户端信息的字符串并存入容器
	string::size_type pos = strClients.find('_');
	while (pos != string::npos) {
		index_l = pos;
		vecClients.push_back(strClients.substr(index_f, index_l - index_f));
		index_f = pos + 1;
		pos = strClients.find('_', index_f);
	}
	vecClients.push_back(strClients.substr(index_f));

	string id;
	string ip;
	string port;
	ios_base::fmtflags fmt = cout.ios_base::setf(ios_base::left, ios_base::adjustfield);		//设置左对齐
	//打印表头
	cout.width(10);
	cout << "编号";
	cout.width(20);
	cout << "IP地址";
	cout.width(20);
	cout << "端口" << endl;
	//遍历输出每条客户端信息
	for(auto iter=vecClients.begin();iter!=vecClients.end();iter++){
		//根据‘-’分割三项内容并分别进行输出
		index_f = 0;
		pos = iter->find('-');
		index_l = pos;
		cout.width(10);
		cout << iter->substr(index_f, index_l - index_f);

		index_f = pos + 1;
		pos = iter->find('-', index_f);
		index_l = pos;
		cout.width(20);
		cout << iter->substr(index_f, index_l - index_f);

		index_f = pos + 1;
		pos = iter->find('-', index_f);
		index_l = pos;
		cout.width(20);
		cout << iter->substr(index_f, index_l - index_f) << endl;
	}
	cout.ios_base::setf(fmt);			//恢复输出流之前的格式化状态
}
