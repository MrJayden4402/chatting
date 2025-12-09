#include <stdio.h>
#include <vector>
#include <windows.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <tlhelp32.h>
#include <stdlib.h>
using namespace std;
extern SOCKET clientSocket;
int InitSocket(void);
int ConnectToServer(const char* IPv4, int Port);
void CloseSocket(void);
bool SendToServer(string sendMess);
extern vector<string> ServerMessList;
string GetMessFromServer(int MaxSize = 1024);
extern vector<SOCKET> clientSockets;
int RunServer(int Port);
DWORD WINAPI ClientHandler(LPVOID lpParameter);
