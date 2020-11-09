#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib,"Ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <vector>
#include <mutex>
using namespace std;

queue<string> MessagesToConcumers;
vector<SOCKET> Consumers;
mutex globalMutex;

// Принятие от клиента сообщений.
void chat(SOCKET servSock)
{
	while (true)
	{
		globalMutex.lock();
		cout << "Thread ¹" << this_thread::get_id() << " listen." << endl;
		globalMutex.unlock();
		int res = listen(servSock, 4);
		if (res == SOCKET_ERROR)
		{
			cout << "Unable to listen on thread " << this_thread::get_id() << endl;
			return;
		}

		SOCKET clientSock;
		SOCKADDR_IN from;
		int fromlen = sizeof(from);
		clientSock = accept(servSock, (struct sockaddr*)&from, &fromlen);
		if (clientSock == INVALID_SOCKET)
		{
			cout << "Unable to accepton thread " << this_thread::get_id() << endl;
			return;
		}

		globalMutex.lock();
		Consumers.push_back(clientSock);
		cout << "Welcome to the club, " << inet_ntoa(from.sin_addr) << ", port " << htons(from.sin_port) << endl;
		char name[1024];
		// Принимаем имя от клиента.
		res = recv(clientSock, name, 1024, 0);
		if (res == SOCKET_ERROR)
		{
			cout << "Unable to recv on thread " << this_thread::get_id() << endl;
			return;
		}
		string toStr(name);
		globalMutex.unlock();

		while (true)
		{
			char szReq[1024];
			// Принимаем сообщение от клиента.
			res = recv(clientSock, szReq, 1024, 0);
			if (res == SOCKET_ERROR)
			{
				cout << "Unable to recv on thread " << this_thread::get_id() << endl;
				return;
			}
			
			// Добавляем имя к сообщению, чтобы отослать другим клиентам.
			string mes = toStr + ": " + string(szReq);
			if (mes[0] == '\\' && mes[1] == 's')
			{
				cout << inet_ntoa(from.sin_addr) << " disconnected." << endl;
				break;
			}
			cout << mes << endl;

			globalMutex.lock();
			// Помещаем сообщение в очередь для рассылки.
			MessagesToConcumers.push(mes);
			globalMutex.unlock();
		}

		globalMutex.lock();
		// Убираем клиента из списка пользователей.
		Consumers.erase(remove(Consumers.begin(), Consumers.end(), clientSock));
		globalMutex.unlock();

		closesocket(clientSock);
	}
}

// Отправляем сообщения пользователям.
void sendToConsumers(SOCKET servSock)
{
	while (true)
	{
		globalMutex.lock();
		// Если очередь сообщений не пуста, то рассылаем сообщения.
		while (!MessagesToConcumers.empty())
		{
			string buff = MessagesToConcumers.front();
			MessagesToConcumers.pop();
			// Отправляем сообщение каждому пользователю.
			for (auto& c : Consumers)
			{
				int res = send(c, buff.c_str(), 1024, 0);
				if (res == SOCKET_ERROR)
				{
					cout << "Unable to send a message" << endl;
					globalMutex.unlock();
					return;
				}
				if (buff[0] == '\\' && buff[1] == 's')
				{
					cout << "Server shutdown." << endl;
					globalMutex.unlock();
					return;
				}
			}
		}
		globalMutex.unlock();
	}
}

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN sin;
	sin.sin_family = PF_INET;
	sin.sin_port = htons(2006);
	sin.sin_addr.s_addr = INADDR_ANY;

	int res = bind(serverSock, (LPSOCKADDR)&sin, sizeof(sin));
	if (res == SOCKET_ERROR)
	{
		cout << "Unable to bind" << endl;
		WSACleanup();
		system("pause");
		closesocket(serverSock);
		return SOCKET_ERROR;
	}

	cout << "Server listen." << endl;
	// Запускаем три потока для пользователей и один поток для рассылки сообщений.
	thread firstChatThread(chat, serverSock);
	thread secondChatThread(chat, serverSock);
	thread thirdChatThread(chat, serverSock);
	thread sender(sendToConsumers, serverSock);
	firstChatThread.join();
	secondChatThread.join();
	thirdCharThread.join();
	sender.join();

	closesocket(serverSock);
}
