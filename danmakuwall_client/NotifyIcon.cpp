#include "NotifyIcon.h"

#include "resource.h"

#define APP_NOTIFYICON         (WM_USER + 1)
#define APP_IDM_CLEARDANMAKU   1000
#define APP_IDM_EXIT           1001

using namespace std;
using namespace DanmakuWall;

static std::unordered_map<HWND, NotifyIcon*> s_boundInstances;
static fcStrW s_strWindowClassName = L"NotifyIcon Host";

class NotifyIconWindowClass
{
public:
	NotifyIconWindowClass(WNDPROC Proc)
	{
		WNDCLASSEX tWindowClass;
		ZeroMemory(&tWindowClass, sizeof(WNDCLASSEX));
		tWindowClass.style = CS_HREDRAW | CS_VREDRAW;
		tWindowClass.cbSize = sizeof(WNDCLASSEX);
		tWindowClass.lpszClassName = s_strWindowClassName;  // 窗口类类名
		tWindowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);  // 小图标
		tWindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // 图标
		tWindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // 背景色
		tWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);  // 鼠标指针
		tWindowClass.hInstance = GetModuleHandle(NULL);  // 程序实例
		tWindowClass.lpfnWndProc = Proc;  // 窗口过程	
		if (!RegisterClassEx(&tWindowClass))
			throw fcyException("NotifyIcon::NotifyIcon", "RegisterClassEx failed.");
	}
	~NotifyIconWindowClass()
	{
		UnregisterClass(s_strWindowClassName, GetModuleHandle(NULL));
	}
};

LRESULT CALLBACK NotifyIcon::globalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto i = s_boundInstances.find(hWnd);
	if (i != s_boundInstances.end())
	{
		NotifyIcon* pForm = i->second;
		if (uMsg == WM_DESTROY)
			s_boundInstances.erase(i);
		return pForm->wndProc(hWnd, uMsg, wParam, lParam);
	}
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

NotifyIcon::NotifyIcon(Config* pConfig, INotifyIconEventListener* pListener)
	: m_pListener(pListener)
{
	static NotifyIconWindowClass s_WindowClass(NotifyIcon::globalWndProc);

	// 创建窗口
	m_hGhostWindow = CreateWindowEx(
		WS_EX_APPWINDOW,
		s_strWindowClassName,
		L"DanmakuWall GhostWindow",
		WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION,
		0,  // left
		0,  // top
		10,  // width
		10,  // height
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
		);
	if (!m_hGhostWindow)
		throw fcyException("NotifyIcon::NotifyIcon", "CreateWindowEx failed.");

	// 创建托盘图标
	memset(&m_NotifyData, 0, sizeof(m_NotifyData));
	m_NotifyData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyData.hWnd = m_hGhostWindow;
	m_NotifyData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_NotifyData.uCallbackMessage = APP_NOTIFYICON;
	m_NotifyData.uID = IDI_APP;
	m_NotifyData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP));
	wcsncpy_s(m_NotifyData.szTip, pConfig->GetTrayIconTitle().c_str(), min(127, pConfig->GetTrayIconTitle().length()));
	if (FALSE == Shell_NotifyIcon(NIM_ADD, &m_NotifyData))
	{
		DestroyWindow(m_hGhostWindow);
		throw fcyException("NotifyIcon::NotifyIcon", "Shell_NotifyIcon failed.");
	}

	// 创建菜单
	m_hPopupMenu = CreatePopupMenu();
	if (!m_hPopupMenu)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);
		DestroyWindow(m_hGhostWindow);
		throw fcyException("NotifyIcon::NotifyIcon", "CreatePopupMenu failed.");
	}
	AppendMenu(m_hPopupMenu, MF_STRING, APP_IDM_CLEARDANMAKU, pConfig->GetTrayMenuStripClear().c_str());
	AppendMenu(m_hPopupMenu, MF_STRING, APP_IDM_EXIT, pConfig->GetTrayMenuStripExit().c_str());

	// 注册回调
	s_boundInstances[m_hGhostWindow] = this;
}

NotifyIcon::~NotifyIcon()
{
	DestroyWindow(m_hGhostWindow);
}

LRESULT NotifyIcon::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);
		break;
	case APP_NOTIFYICON:
		switch (lParam)
		{
		case WM_LBUTTONDOWN:
			break;
		case WM_RBUTTONDOWN:
			{
				POINT tCursorPos;
				GetCursorPos(&tCursorPos);
				TrackPopupMenu(m_hPopupMenu, TPM_RIGHTBUTTON, tCursorPos.x, tCursorPos.y, NULL, hWnd, NULL);
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case APP_IDM_CLEARDANMAKU:
			if (m_pListener)
				m_pListener->OnClearDanmakuClicked();
			break;
		case APP_IDM_EXIT:
			if (m_pListener)
				m_pListener->OnExitClicked();
			break;
		default:
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
