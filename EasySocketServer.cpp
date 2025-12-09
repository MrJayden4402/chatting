#include "EasySocket.cpp"
using namespace std;
string MainName;
bool isProcessRunning(const char *processName)
{
	// 创建一个进程快照
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// 遍历进程快照
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	while (Process32Next(hProcessSnap, &pe32))
	{
		// 检查进程名是否匹配
		if (strcmp(pe32.szExeFile, processName) == 0)
		{
			CloseHandle(hProcessSnap);
			return true;
		}
	}

	CloseHandle(hProcessSnap);
	return false;
}
void CloseWithUser(void)
{
	while (true)
	{
		if (!isProcessRunning("chat.exe"))
		{
			CloseSocket();
			exit(0);
		}
		Sleep(1000);
	}
}
int main(int argc, char *argv[])
{
	string cmdstring(*argv);
	// MessageBox(0, cmdstring.c_str(), "cmdstring", 0);
	istringstream istr(cmdstring);
	int Port;
	string input;
	getline(istr, MainName, '_');
	getline(istr, input, '_');
	istringstream getport(input);
	getport >> Port;
	cout << cmdstring << endl;
	// new thread(CloseWithUser);
	// MessageBox(0, to_string(Port).c_str(), 0, 0);
	RunServer(Port);
	return 0;
}
