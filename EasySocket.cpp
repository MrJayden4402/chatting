#include "EasySocket.h"
#include <winsock2.h>
SOCKET clientSocket;
int InitSocket(void)
{
	// 初始化 Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Failed to initialize winsock" << std::endl;
		return 1;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "Failed to create client socket" << std::endl;
		WSACleanup();
		return 2;
	}
	return 0;
}
int ConnectToServer(const char *IPv4, int Port)
{
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Port);			  // 设置服务器端口号
	serverAddr.sin_addr.s_addr = inet_addr(IPv4); // 设置服务器IP地址

	// 连接到服务器
	if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Failed to connect to the server(ConnectToServer)" << std::endl;
		MessageBox(NULL, "Failed to connect to the server", "Error", MB_OK | MB_ICONERROR);
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}
void CloseSocket(void)
{
	// 关闭 socket 和清理 Winsock
	closesocket(clientSocket);
	WSACleanup();
}
bool SendToServer(string sendMess)
{
	sendMess = to_string(sendMess.size()) + ":" + sendMess;
	return send(clientSocket, sendMess.c_str(), strlen(sendMess.c_str()), 0) == SOCKET_ERROR;
}
vector<string> ServerMessList;
string GetMessFromServer(int MaxSize)
{
	if (!ServerMessList.empty())
	{
		string s = ServerMessList[0];
		ServerMessList.erase(ServerMessList.begin());
		return s;
	}

	char buffer[MaxSize];
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
		return string("SOCKET_ERROR");

	string s(buffer);
	int ssize = 0;
	for (int i = 0; i < s.size(); i++)
		if (s[i] == ':')
		{
			ServerMessList.push_back(s.substr(i + 1, ssize));
			i += ssize;
			ssize = 0;
		}
		else
		{
			ssize = ssize * 10 + s[i] - 48;
		}
	string ss = ServerMessList[0];
	ServerMessList.erase(ServerMessList.begin());
	return ss;
}
vector<SOCKET> clientSockets;
DWORD WINAPI ClientHandler(LPVOID lpParameter)
{
	SOCKET clientSocket = reinterpret_cast<SOCKET>(lpParameter);
	char buffer[4096];
	int bytesRead;

	while (true)
	{
		memset(buffer, 0, sizeof(buffer));

		// 接收客户端消息
		bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesRead <= 0)
		{
			break;
		}
		// 将消息转发给其他客户端
		for (const auto &socket : clientSockets)
		{
			if (socket != clientSocket)
			{
				send(socket, buffer, bytesRead, 0);
			}
		}
	}

	// 关闭客户端socket
	closesocket(clientSocket);

	return 0;
}
int RunServer(int Port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed." << std::endl;
		return -1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Failed to create socket." << std::endl;
		WSACleanup();
		return -1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(Port);

	if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Failed to bind." << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Failed to listen." << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}

	std::cout << "Server listening on port " + to_string(Port) << std::endl;

	while (true)
	{
		// 接受新的客户端连接
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "Failed to accept client." << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return -1;
		}

		std::cout << "\nNew client connected." << std::endl;

		// 将新的客户端连接添加到列表中
		clientSockets.push_back(clientSocket);

		// 创建新线程来处理每个客户端的消息
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ClientHandler, reinterpret_cast<void *>(clientSocket), 0, nullptr);
	}

	// 关闭所有客户端连接
	for (const auto &clientSocket : clientSockets)
	{
		closesocket(clientSocket);
	}

	// 关闭服务器socket
	closesocket(serverSocket);

	// 清理Winsock库
	WSACleanup();

	return 0;
}
