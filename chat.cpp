#include "EasySocket.cpp"
#include <fstream>

bool gameover = false;
bool StartToTalk = false;
string IPv4, UserName;
int Port;
HWND window;
HWND hStatic, Input, Sure, CallKey;
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1 : 0)
bool startProcess(const char *name, const char *path)
{
	STARTUPINFOA sInfo = {0};
	PROCESS_INFORMATION pInfo = {0};
	// 启动新进程
	if (!CreateProcessA((LPCSTR)name, (LPSTR)path, NULL, NULL, FALSE,
						NULL, NULL, NULL, &sInfo, &pInfo))
	{
		return false;
	}
	// 关闭主线程句柄，因为我们不会使用它
	CloseHandle(pInfo.hThread);
	// 关闭进程句柄，我们只需要保持主线程句柄
	CloseHandle(pInfo.hProcess);

	return true;
}
bool isCalling = false;
vector<string> MessList;
void CallAnswer(void)
{
	isCalling = true;
	MessageBox(0, "你的朋友提醒你尽快回复消息", "来自聊天室的提醒", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	isCalling = false;
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char buffer[64];
	memset(buffer, 0, 64);
	GetWindowText(hwnd, buffer, 64);
	string name(buffer);
	if (name == "来自聊天室的提醒")
	{
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}
vector<string> InFileString;
void GetMessage__(void)
{
	char buffer[4096];
	while (true)
	{
		// 从服务器接收响应
		memset(buffer, 0, sizeof(buffer));
		string s = GetMessFromServer(4096);
		if (s == "SOCKET_ERROR")
		{
			MessList.push_back("Failed to receive data from the server(GetMessage_)");
			MessList.push_back("Error code: " + to_string(WSAGetLastError()));
			break;
		}
		istringstream istr(s);
		string type;
		istr >> type;
		if (s == "CallAnswer")
		{
			if (!isCalling)
				new thread(CallAnswer);
			// EnumWindows(EnumWindowsProc,0);
		}
		// else if (type == "Infile")
		// {
		// 	string k = istr.str();
		// 	string s = k.substr(7, k.size() - 7);
		// 	// MessageBox(0, s.c_str(), "InFile", 0);
		// 	InFileString.push_back(s);
		// }
		// else if (type == "Startfile")
		// {
		// 	string s;
		// 	istr >> s;
		// 	// MessageBox(0, s.c_str(), "StartFile", 0);
		// 	InFileString.push_back("Infile " + s);
		// }
		// else if (type == "EndInFile")
		// {
		// 	// MessageBox(0, "EndFile", "EndFile", 0);
		// 	ofstream fout(InFileString[0].c_str(), ofstream::binary | ofstream::out);
		// 	for (int i = 1; i < InFileString.size(); i++)
		// 		for (int j = 0; j < InFileString[i].size(); j++)
		// 			fout.write(&(InFileString[i])[j], 1);
		// 	fout.clear();
		// 	fout.close();
		// 	InFileString.clear();
		// }
		else
		{
			MessList.push_back(s);
		}
		Sleep(10);
	}
}
void SendFile(string filename)
{
	vector<char> file;
	vector<string> thstr;
	ifstream fin(filename.c_str(), ifstream::binary | ifstream::in);
	char ch;
	while (fin.read(&ch, 1))
		file.push_back(ch);
	fin.clear();
	fin.close();

	string s;
	for (int i = 0; i < file.size(); i++)
		s += file[i];
	// MessList.push_back(s);
	// return;

	int thn = file.size() / 1000;
	int othern = file.size() % 1000;
	for (int i = 0; i < thn; i++)
	{
		thstr.push_back("");
		for (int j = 0; j < 1000; j++)
			thstr[thstr.size() - 1].push_back(file[i * 1000 + j]);
	}
	string ostr;
	for (int i = 0; i < othern; i++)
		ostr.push_back(file[thn * 1000 + i]);
	string sendMess = "Startfile " + filename;
	SendToServer(sendMess);
	for (int i = 0; i < thstr.size(); i++)
	{
		sendMess = "Infile " + thstr[i];
		SendToServer(sendMess);
		Sleep(100);
	}
	sendMess = "Infile " + ostr;
	SendToServer(sendMess);
	sendMess = "EndInFile";
	SendToServer(sendMess);
}
int CallAnswerColdTime = 0;
LRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
		int result = MessageBox(window, "确定关闭？", "聊天室", MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);
		if (result == IDYES)
			exit(0);
		return 1;
		break;
	}
	case WM_DESTROY:
		gameover = true;
		exit(0);
		break;
	case WM_COMMAND:
		if ((HWND)lParam == Sure)
		{
			if (!StartToTalk)
			{
				StartToTalk = true;
				char buffer[128];
				memset(buffer, 0, 128);
				GetWindowText(Input, buffer, 128);
				string s(buffer);
				istringstream istr(s);
				string t;
				istr >> t >> IPv4 >> Port >> UserName;
				if (t == "y")
				{
					MessList.push_back("启动中...");
					startProcess("EasySocketServer.exe", string("socket封装服务器.exe_" + to_string(Port)).c_str());
					Sleep(5000);
					MessList.push_back("已启动,可以发送消息");
				}

				ConnectToServer(IPv4.c_str(), Port);
				new thread(GetMessage__);
				if (t == "n")
				{
					MessList.push_back("已连接,可以发送消息");
					string sendMess = UserName + "加入了服务器";
					if (SendToServer(sendMess))
					{
						MessList.push_back("Failed to send data to the server");
						break;
					}
				}
			}
			else
			{
				char buffer[128];
				memset(buffer, 0, 128);
				GetWindowText(Input, buffer, 128);
				string s = string(buffer);
				if (s.empty())
					break;
				istringstream istr(s);
				string ss;
				istr >> ss;
				if (ss == "SendFile")
				{
					istr >> ss;
					new thread(SendFile, ss);
					break;
				}
				string sendMess = UserName + ":" + string(buffer);
				if (SendToServer(sendMess))
				{
					MessList.push_back("Failed to send data to the server");
					break;
				}
				MessList.push_back(UserName + ":" + string(buffer));
			}
			SetWindowText(Input, "");
		}
		if ((HWND)lParam == CallKey)
		{
			if (GetTickCount() - CallAnswerColdTime < 30000)
			{
				MessList.push_back("不能过于频繁地提醒发送，冷却剩余" + to_string((30000 - (GetTickCount() - CallAnswerColdTime)) / 1000) + "秒");
				break;
			}
			CallAnswerColdTime = GetTickCount();
			string sendMess = "CallAnswer";
			if (SendToServer(sendMess))
			{
				MessList.push_back("Failed to send data to the server");
				break;
			}
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
void SendMessage__(void)
{
	MessList.push_back("请输入起始信息（是否开启服务器y/n + IPv4 + 端口号 + 你的名字）");
}
void KeyEnterToSendMessage(void)
{
	int t = GetTickCount();
	while (true)
	{
		if (KEY_DOWN(13) && GetForegroundWindow() == window)
		{
			if (GetTickCount() - t < 300)
			{
				SendMessage(window, WM_COMMAND, 0, (LPARAM)Sure);
			}
			t = GetTickCount();
			while (KEY_DOWN(13))
				Sleep(1);
		}
		Sleep(50);
	}
}
void Game_Run(void)
{
	new thread(KeyEnterToSendMessage);
	while (true)
	{
		while (MessList.size() > 25)
			MessList.erase(MessList.begin());
		string s;
		for (int i = 0; i < MessList.size(); i++)
			s = s + MessList[i] + "\n";
		SetWindowText(hStatic, s.c_str());
		Sleep(600);
	}
}
// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MainWindowClass";
	wc.hIconSm = NULL;
	RegisterClassEx(&wc);
	// create a new window
	window = CreateWindow("MainWindowClass", "聊天室",
						  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_EX_TOPMOST, CW_USEDEFAULT, CW_USEDEFAULT,
						  700, 500, NULL, NULL, hInstance, NULL);
	if (window == 0)
		return 0;
	ShowWindow(window, SW_SHOW);
	UpdateWindow(window);
	// SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	hStatic = CreateWindow(TEXT("STATIC"), TEXT("Test String"), WS_CHILD | WS_VISIBLE, 0, 0, 700, 400, window, 0, NULL, NULL);
	SetWindowText(hStatic, "<聊天室>");
	Input = CreateWindow("EDIT", 0, WS_BORDER | WS_CHILD | WS_VISIBLE, 30, 420, 400, 20, window, 0, NULL, NULL);
	Sure = CreateWindow(TEXT("BUTTON"), TEXT("确定"), WS_CHILD | WS_VISIBLE, 450, 420, 40, 20, window, 0, NULL, NULL);
	CallKey = CreateWindow(TEXT("BUTTON"), TEXT("提醒查看消息"), WS_CHILD | WS_VISIBLE, 510, 420, 130, 20, window, 0, NULL, NULL);

	HFONT hThinFont = CreateFontA(
		-MulDiv(9, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72),
		0, 0, 0,
		FW_LIGHT,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"Segoe UI");

	SendMessage(hStatic, WM_SETFONT, (WPARAM)hThinFont, TRUE);
	SendMessage(Input, WM_SETFONT, (WPARAM)hThinFont, TRUE);
	SendMessage(Sure, WM_SETFONT, (WPARAM)hThinFont, TRUE);
	SendMessage(CallKey, WM_SETFONT, (WPARAM)hThinFont, TRUE);

	int res = InitSocket();
	if (res != 0)
	{
		MessageBox(0, to_string(res).c_str(), "Error", 0);
	}

	new thread(SendMessage__);
	new thread(Game_Run);

	// main message loop
	MSG message;
	while (!gameover)
	{
		if (GetMessage(&message, NULL, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return message.wParam;
}
