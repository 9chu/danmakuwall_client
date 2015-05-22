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
			MessageBox(0, L"�����������С�", L"��Ļǽ", MB_ICONERROR | MB_OK);
			CloseHandle(hGlobalMutex);
			return -3;
		}
	}
	else
	{
		MessageBox(0, L"����ȫ�ֻ�����ʧ�ܡ�", L"��Ļǽ", MB_ICONERROR | MB_OK);
		return -4;
	}

	std::unique_ptr<AppFrame> pFrame;

	try
	{
		pFrame = make_unique<AppFrame>();
	}
	catch (const fcyException& e)
	{
		MessageBoxA(0, (string("��ʼ��ʧ�ܡ�\n\n��ϸ��Ϣ��\n") + e.GetSrc() + "\n" + e.GetDesc()).c_str(), "��Ļǽ", MB_ICONERROR | MB_OK);
		CloseHandle(hGlobalMutex);
		return -1;
	}

	try
	{
		pFrame->Run();
	}
	catch (const fcyException& e)
	{
		MessageBoxA(0, (string("����δ���������ʱ���󣬳����˳���\n�������ⷴ�����֣����ύissue��\n\n��ϸ��Ϣ��\n") +
			e.GetSrc() + "\n" + e.GetDesc()).c_str(), "��Ļǽ", MB_ICONERROR | MB_OK);
		CloseHandle(hGlobalMutex);
		return -2;
	}

	CloseHandle(hGlobalMutex);
	return 0;
}
