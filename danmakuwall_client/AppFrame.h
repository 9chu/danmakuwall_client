#pragma once
#include "Common.h"
#include "Config.h"
#include "DanmakuPool.h"
#include "DanmakuReceiver.h"
#include "NotifyIcon.h"

namespace DanmakuWall
{
	/// \brief Ӧ�ó�����
	class AppFrame :
		public f2dEngineEventListener,
		public INotifyIconEventListener
	{
	private:
		fcyRefPointer<f2dEngine> m_pEngine;
		f2dWindow* m_pWindow = nullptr;
		f2dRenderer* m_pRenderer = nullptr;
		f2dRenderDevice* m_pRenderDev = nullptr;
		fcyRefPointer<f2dGraphics2D> m_pGraph2d;

		bool m_bExceptionThrown = false;
		fcyException m_threadException;

		std::unique_ptr<Config> m_pConfig;
		std::unique_ptr<DanmakuPool> m_pDanmakuPool;
		std::unique_ptr<DanmakuReceiver> m_pDanmakuReceiver;
		std::unique_ptr<NotifyIcon> m_pNotifyIcon;

		float m_fTopMostTimer = 0.f;
		float m_fReconnectTimer = 0.f;
	private:
		/// \brief �������ڴ�С����Ӧ��Ļ��С
		void resizeWindow()ynothrow;
	public:
		/// \brief ����������
		void Run();
	protected:  // f2dEngineEventListener
		/// \brief ����ѭ��
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)ynothrow;
		/// \brief ��Ⱦѭ��
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)ynothrow;
	protected:  // INotifyIconEventListener
		/// \brief ��յ�Ļ��ť����
		void OnClearDanmakuClicked();
		/// \brief �˳���ť����
		void OnExitClicked();
	public:
		AppFrame();
	};
}
