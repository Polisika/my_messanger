#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib,"Ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <mutex>

using namespace std;

char message[1024];

void showMessages(SOCKET clientSock)
{
    char r[1024] = "\0";
    int size = sizeof(r);
    int ret = INFINITY;
    while (true)
    {
        ret = recv(clientSock, r, size, 0);
        if (ret == SOCKET_ERROR)
        {
            cout << "Disconnected. Print \\s to quit." << endl;
            return;
        }
        else if (strcmp(message, r) != 0)
        {
            if (message[0] == '\\' && message[1] == 's')
                break;
            cout << r << endl;
        }
    }
}
void sendMessages(SOCKET clientSock)
{
    while (true)
    {
        gets_s(message, sizeof(message));
        send(clientSock, message, sizeof(message), 0);

        if (message[0] == '\\' && message[1] == 's')
            break;
    }
}
int main()
{
#pragma region Ñonnect
    WORD ver = MAKEWORD(2, 2);
    WSADATA wsaData;
    int retVal = 0;
    WSAStartup(ver, (LPWSADATA)&wsaData);
    LPHOSTENT hostEnt;
    hostEnt = gethostbyname("localhost");
    if (!hostEnt)
    {
        printf("Unable to collect gethostbyname\n");
        WSACleanup();
        system("pause");
        return 1;
    }

    SOCKET clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == SOCKET_ERROR)
    {
        printf("Unable to create socket\n");
        WSACleanup();
        system("pause");
        return 1;
    }
    string ip;
    cout << "ip>";
    cin >> ip;
    cin.ignore();

    SOCKADDR_IN serverInfo;
    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    serverInfo.sin_port = htons(2006);

    retVal = connect(clientSock, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));
    if (retVal == SOCKET_ERROR)
    {
        printf("Unable to connect\n");
        closesocket(clientSock);
        WSACleanup();
        system("pause");
        return 1;
    }
    printf("Connection made sucessfully\n");
#pragma endregion
#pragma region Client Logiñ
    cout << "Print message to all." << endl;
    thread mess(showMessages, ref(clientSock));
    thread sendmess(sendMessages, ref(clientSock));
    mess.join();
    sendmess.join();
#pragma endregion
#pragma region Cleanup
    closesocket(clientSock);
    WSACleanup();
#pragma endregion
    system("pause");
    return 0;
}