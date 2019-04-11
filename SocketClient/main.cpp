#include <iostream>
#include "SocketClient.h"
#include <sstream>
#include <thread>
#include <mutex>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

void printClientList(const string& clientList);				//�ͻ����б��ӡ����������Э��ӽ��ܵ���Ϣ�����еõ��ͻ����б���ӡ
void procMsgList(deque<Msg>& msgList, mutex& mx);			//��Ϣ���д�����������ת����Ϣ֮�������������Ϣ���Ĵ���
void threadComMsg(deque<Msg>& msgList, mutex& mxList, mutex& mxPrint, const bool& exiSignal);	//ָʾ��Ϣ�����̺߳�����ʵʱ��ʾ������ת���������ͻ�����Ϣ
void threadListen(SocketClient& sc, deque<Msg>& msgList, mutex& mx, const bool& exitSignal);	//���ռ����̺߳�����ʵʱ�ӽ��ջ����л�ȡ�ַ�������Э�������Ϣ���ṹ��Msg��ӵ���Ϣ�����еȴ�����

int counter = 0;

int main()
{
	deque<Msg> deqMsgList;				//��Ϣ����
	mutex mxMsg;						//��Ϣ�����̻߳�����󣬷�ֹ�����߳�ͬʱ����Ϣ���н��в���
	mutex mxPrint;						//ָʾ��Ϣ��������󣬷�ֹ�˵���Ϣ��ָʾ��Ϣ����ͬʱ��ӡ����һ��
	bool exitSignal = false;

	SocketClient sClient;				//��������ͻ��˵���

	thread listenThread(threadListen, ref(sClient), ref(deqMsgList), ref(mxMsg), ref(exitSignal));		//���������̣߳������ӳɹ�ʱ�ӽ��ܴӷ�������������Ϣ
	thread comMsgThread(threadComMsg, ref(deqMsgList), ref(mxMsg), ref(mxPrint), ref(exitSignal));		//����ָʾ��Ϣ�����̣߳�ʵʱ��ʾ�������ͻ��˷�������Ϣ

	//���˵��������
	int choice;								
	int targetID;
	string ip;
	string msg;
	int ret;

	//��ʾ�˵�������ѡ�������Ӧ����
	//1.����
	//2.�Ͽ�
	//3.��ȡʱ��
	//4.��ȡ����
	//5.��ȡ�ͻ����б�
	//6.������Ϣ
	//7.�˳�
	while (1) {
		if (!sClient.isConnected()) {		//���ݿͻ�������״̬�ж�
			cout << "\n* * * * * * * * * * * *\n"
				<< "* �ͻ���״̬��δ����...\n"
				<< "* 1.����\n"
				<< "* 2.�˳�\n"
				<< "* * * * * * * * * * * *\n"
				<< "> ����ѡ������Խ���ѡ�񣨰��س���������: ";
			while (1) {
				cin >> choice;
				if (choice == 1 || choice == 2) {
					//����ѡ�����Ҫ�󣬽��д���
					switch (choice) {
					case 1:
						cout << "> ������Ŀ���������ip��ַ: ";
						cin >> ip;
						cout << endl;
						if (sClient.connect2server(ip) != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);		//�����ӳɹ��������ȴ���Ӧ��Ϣ����ͬ
						break;
					case 2:
						if (sClient.isConnected()) {
							sClient.disconnect();
							exitSignal = true;
							listenThread.join();		//�ȴ����������̷߳���
							comMsgThread.join();		//�ȴ�ָʾ��Ϣ�����̷߳���
							cout << "[ϵͳ��Ϣ] ���߳��˳��ɹ���" << endl;
						}
						exit(0);
						break;
					}
					break;
				}
				else {
					cout << "> ������������������ѡ��: ";
					cin.clear();				//�ָ�������״̬λ
					cin.ignore(1024, '\n');		//������뻺��������
				}
			}
		}
		else {	//�ͻ���������
			mxPrint.lock();				//��ӡ�˵�ʱ������������ֹ���߳�ͬʱ�����ɻ���
			cout <<"\n* * * * * * * * * * * *\n"
				<< "* �ͻ���״̬��������...\n"
				<< "* 1.����\n"
				<< "* 2.�Ͽ�����\n"
				<< "* 3.��ȡʱ��\n"
				<< "* 4.��ȡ����\n"
				<< "* 5.��ȡ�ͻ����б�\n"
				<< "* 6.������Ϣ\n"
				<< "* 7.�˳�\n"
				<< "* * * * * * * * * * * *\n"
				<< "> ����ѡ������Խ���ѡ�񣨰��س���������: ";
			mxPrint.unlock();
			while (1) {
				cin >> choice;
				if (choice >= 1 && choice <= 7) {
					//����ѡ�����Ҫ�󣬽��д���
					switch (choice) {
					case 1:
						cout << "> ������Ŀ���������ip��ַ: ";
						cin >> ip;
						cout << endl;
						if (sClient.connect2server(ip) != SOCKET_ERROR) procMsgList(deqMsgList, mxMsg);
						break;
					case 2:
						sClient.disconnect();
						cout << "[ϵͳ��Ϣ] �ѶϿ����ӣ�" << endl;
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
						cout << "> ����������Ҫ������Ϣ��Ŀ��ͻ������: ";
						cin >> targetID;
						cout << "> ����������Ҫ���͵���Ϣ����: ";
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
						cout << "[ϵͳ��Ϣ] ���߳��˳��ɹ���" << endl;
						exit(0);
						break;
					}
					break;
				}
				else {
					cout << "> ������������������ѡ��: ";
					cin.clear();
					cin.ignore(1024, '\n');
				}
			}
		}
	}
	return 0;
}

//��Ϣ�����̺߳���
void threadListen(SocketClient& sc,deque<Msg>& msgList, mutex& mx, const bool& exitSignal) {
	static enum RecState {
		rec_type,				//����ʶ��׶�
		rec_length,				//����ʶ��׶�
		rec_content				//����ʶ��׶�
	};

	while (sc.isConnected() == false);		//���������ѭ�������ͻ������ӳɹ�֮�������Ϣ����

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
		case rec_type:		//ʶ��״̬����rec_type������δ��ʼ������Ϣ�����������Ϣ������жϣ��޸�״̬Ϊrec_length����ʼ��ȡ�����ֶ�
			_type= MsgType(c - '0');
			if (_type >= 0 && _type <= 9) {
				msgTemp.msgType = _type;
				rs = rec_length;	//�޸�״̬Ϊrec_length����ʼ��ȡ�����ֶ�
				break;
			}
			else continue;
		case rec_length:		//ʶ��״̬����rec_length����ȡ�����ֶ���
			strLen[cnt++] = c;	//����λ�����ֶδ�ŵ�strLen������
			if (cnt == LENGTH_BITS) {		//�����ֶζ�ȡ��Ϻ�ʹ����ת��Ϊint�����浽iLen����
				cnt = 0;
				stringstream ss(strLen);
				ss >> iLen;
				rs = rec_content;	//�޸�״̬Ϊrec_content����ʼ��ȡ�����ֶ�
			}
			break;
		case rec_content:		//ʶ��״̬����rec_content����ȡ�����ֶ���
			strContent.push_back(c);
			cnt++;
			if (cnt == iLen){	//�����ֶζ�ȡ��Ϻ���������Ϣ���ṹ��ӵ���Ϣ����
				cnt = 0;
				msgTemp.content = strContent;
				if (msgTemp.msgType == RES_TIME) counter++;
				mx.lock();
				msgList.push_back(msgTemp);
				mx.unlock();
				strContent.clear();
				rs = rec_type;	//�޸�״̬Ϊrec_type�����¿�ʼ�ȴ���Ϣ��
			}
			break;
		}
	}
}

//ָʾ��Ϣ�����̺߳���
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
		if (!hasCommand) continue;	//��������û��ָʾ��Ϣʱ,�����д���
		mxList.lock();		//������Ϣ�б���д���ʱ,����������ֹ�����̶߳�deque���в��������ܵ��µ�����ʧЧ��
		auto iter = msgList.begin();
		while (iter != msgList.end()) {
			if (iter->msgType == COM_MSG) {
				msg = iter->content;
				index_l = msg.find("_", index_f);
				id = msg.substr(index_f, index_l - index_f);			//����õ�Դ�ͻ��˱��
				index_f = index_l + 1;
				index_l = msg.find("_", index_f);
				time = msg.substr(index_f, index_l - index_f);			//����õ�ʱ��
				index_f = index_l + 1;
				msg = msg.substr(index_f);								//����õ���Ϣ����
				mxPrint.lock();
				cout << "\n[ϵͳ��Ϣ] ���յ������ͻ�����Ϣ: " << endl;
				cout << "���: " + id << endl << "ʱ�䣨��-��-��-ʱ-��-�룩: " + time << endl << "����:" + msg << endl;
				mxPrint.unlock();
				iter = msgList.erase(iter);
			}
			else iter++;		//����Ϣ����ָʾ��Ϣ���鿴��һ��
		}
		mxList.unlock();
	}

}

//��Ϣ���д�����
void procMsgList(deque<Msg>& msgList, mutex& mx) {
	bool hasResponse=false;
	MsgType type;

	//�ȴ����յ���Ӧ��Ϣ
	while (!hasResponse) {
		mx.lock();			//������������ֹ�����̶߳�msgList���в��������µ�����ʧЧ
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

	mx.lock();			//��ʼ����ʱ������������ָֹʾ��Ϣ�����̶߳���Ϣ���н������/ɾ�����µĵ�����ʧЧ
	auto iter = msgList.begin();
	while (iter != msgList.end()) {		//������Ϣ�б�������Ϣ����������Ӧ����
		switch (iter->msgType) {
		case RES_TIME:
			cout << "[��������Ӧ] ��ǰ������ʱ��Ϊ����-��-��-ʱ-��-�룩: " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_NAME:
			cout << "[��������Ӧ] ������������Ϊ: " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_LIST:
			cout << "[��������Ӧ] �뱾�ͻ������ӵ���ͬ�������Ŀͻ����б����£�" << endl;
			printClientList(iter->content);
			cout << "[ϵͳ��Ϣ] ��ӡ��ϣ�" << endl;
			iter = msgList.erase(iter);
			break;
		case RES_MSG:
			cout << "[��������Ӧ] " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case RES_CON:
			cout << "[��������Ӧ] " + iter->content << endl;
			iter = msgList.erase(iter);
			break;
		case COM_MSG:
			iter++;			//����ָʾ��Ϣ������ʵʱ��Ӧ���߳̽��д���
			break;
		}
	}
	mx.unlock();
}

//��ӡ�ͻ����б���
void printClientList(const string& clientList) {
	string strClients = clientList;
	vector<string> vecClients;

	string::size_type index_f = 0;
	string::size_type index_l;
	
	//�����ַ����ָ�Ϊ��ʾһ�����ͻ�����Ϣ���ַ�������������
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
	ios_base::fmtflags fmt = cout.ios_base::setf(ios_base::left, ios_base::adjustfield);		//���������
	//��ӡ��ͷ
	cout.width(10);
	cout << "���";
	cout.width(20);
	cout << "IP��ַ";
	cout.width(20);
	cout << "�˿�" << endl;
	//�������ÿ���ͻ�����Ϣ
	for(auto iter=vecClients.begin();iter!=vecClients.end();iter++){
		//���ݡ�-���ָ��������ݲ��ֱ�������
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
	cout.ios_base::setf(fmt);			//�ָ������֮ǰ�ĸ�ʽ��״̬
}
