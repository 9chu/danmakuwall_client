#pragma once
#include "Common.h"
#include "Config.h"

namespace DanmakuWall
{
	/// \brief 托盘图标回调
	struct INotifyIconEventListener
	{
		virtual void OnClearDanmakuClicked() = 0;
		virtual void OnExitClicked() = 0;
	};

	/// \brief 托盘图标
	/// \note  实现上，将托盘图标绑定到一个单独的窗口上用于接收消息
	class NotifyIcon
	{
	private:
		HWND m_hGhostWindow = NULL;
		NOTIFYICONDATA m_NotifyData;
		HMENU m_hPopupMenu = NULL;

		INotifyIconEventListener* m_pListener = NULL;
	private:
		static LRESULT CALLBACK globalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	public:
		NotifyIcon(Config* pConfig, INotifyIconEventListener* pListener);
		~NotifyIcon();
	};
}
