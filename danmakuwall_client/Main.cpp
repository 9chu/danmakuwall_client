#include "Common.h"
#include "AppFrame.h"

using namespace std;
using namespace DanmakuWall;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(248);
#endif

	HANDLE hGlobalMutex = CreateMutex(NULL, FALSE, L"DanmakuWallClient");
	DWORD dwRet = GetLastError();
	if (hGlobalMutex)
	{
		if (ERROR_ALREADY_EXISTS == dwRet)
		{
			MessageBox(0, L"程序已在运行。", L"弹幕墙", MB_ICONERROR | MB_OK);
			CloseHandle(hGlobalMutex);
			return -3;
		}
	}
	else
	{
		MessageBox(0, L"创建全局互斥锁失败。", L"弹幕墙", MB_ICONERROR | MB_OK);
		return -4;
	}

	std::unique_ptr<AppFrame> pFrame;

	try
	{
		pFrame = make_unique<AppFrame>();
	}
	catch (const fcyException& e)
	{
		MessageBoxA(0, (string("初始化失败。\n\n详细信息：\n") + e.GetSrc() + "\n" + e.GetDesc()).c_str(), "弹幕墙", MB_ICONERROR | MB_OK);
		CloseHandle(hGlobalMutex);
		return -1;
	}

	try
	{
		pFrame->Run();
	}
	catch (const fcyException& e)
	{
		MessageBoxA(0, (string("遭遇未处理的运行时错误，程序将退出。\n若该问题反复出现，请提交issue。\n\n详细信息：\n") +
			e.GetSrc() + "\n" + e.GetDesc()).c_str(), "弹幕墙", MB_ICONERROR | MB_OK);
		CloseHandle(hGlobalMutex);
		return -2;
	}

	CloseHandle(hGlobalMutex);
	return 0;
}
