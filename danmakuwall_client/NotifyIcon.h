#pragma once
#include "Common.h"
#include "Config.h"

namespace DanmakuWall
{
	/// \brief ����ͼ��ص�
	struct INotifyIconEventListener
	{
		virtual void OnClearDanmakuClicked() = 0;
		virtual void OnExitClicked() = 0;
	};

	/// \brief ����ͼ��
	/// \note  ʵ���ϣ�������ͼ��󶨵�һ�������Ĵ��������ڽ�����Ϣ
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
