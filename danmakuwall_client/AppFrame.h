#pragma once
#include "Common.h"
#include "Config.h"
#include "DanmakuPool.h"
#include "DanmakuReceiver.h"
#include "NotifyIcon.h"

namespace DanmakuWall
{
	/// \brief 应用程序框架
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
		/// \brief 调整窗口大小以适应屏幕大小
		void resizeWindow()ynothrow;
	public:
		/// \brief 启动程序框架
		void Run();
	protected:  // f2dEngineEventListener
		/// \brief 更新循环
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)ynothrow;
		/// \brief 渲染循环
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)ynothrow;
	protected:  // INotifyIconEventListener
		/// \brief 清空弹幕按钮按下
		void OnClearDanmakuClicked();
		/// \brief 退出按钮按下
		void OnExitClicked();
	public:
		AppFrame();
	};
}
