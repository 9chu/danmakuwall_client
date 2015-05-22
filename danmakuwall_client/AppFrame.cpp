#include "AppFrame.h"

using namespace std;
using namespace DanmakuWall;

AppFrame::AppFrame()
{
	// 初始化引擎
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			// 在DLL帧栈中重新抛出异常，注意隐含的ABI兼容问题
			throw fcyException(Src, Desc);
		}
	} tErrListener;

	// 初始化引擎
	CreateF2DEngineAndInit(
		F2DVERSION,
		fcyRect(0.f, 0.f, 50.f, 50.f),
		L"弹幕墙 弹幕窗口",
		true,
		true,
		F2DAALEVEL_NONE,
		this,
		&m_pEngine,
		&tErrListener
		);

	// 获得组件
	m_pWindow = m_pEngine->GetMainWindow();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pRenderDev = m_pRenderer->GetDevice();

	// 初始化渲染器
	if (FCYFAILED(m_pRenderDev->CreateGraphics2D(0, 0, &m_pGraph2d)))
		throw fcyException("AppFrame::AppFrame", "CreateGraphics2D failed.");

	// 设置窗口样式
	SetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	SetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE, 
		GetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);  // 设置层叠属性+鼠标穿透
	m_pWindow->SetBorderType(F2DWINBORDERTYPE_NONE);
	m_pWindow->SetTopMost(true);

	// 装载配置文件
	m_pConfig = make_unique<Config>(L"config.json");

	// 初始化弹幕池
	m_pDanmakuPool = make_unique<DanmakuPool>(m_pRenderer, m_pConfig.get());

	// 初始化弹幕接收器
	m_pDanmakuReceiver = make_unique<DanmakuReceiver>(m_pConfig.get());

	// 创建托盘图标
	m_pNotifyIcon = make_unique<NotifyIcon>(m_pConfig.get(), this);

	// 重设大小并铺满整个屏幕
	resizeWindow();
}

void AppFrame::resizeWindow()ynothrow
{
	// 通过API获得窗口大小
	fuInt tScreenW = GetSystemMetrics(SM_CXSCREEN);
	fuInt tScreenH = GetSystemMetrics(SM_CYSCREEN);

	// 重设大小
	m_pRenderDev->SetBufferSize(tScreenW, tScreenH, true, true, F2DAALEVEL_NONE);
	m_pGraph2d->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(
		0.f,
		(float)tScreenW,
		(float)tScreenH,
		0.f, 0.f, 100.f
		));
	m_pDanmakuPool->Resize(fcyVec2((float)tScreenW, (float)tScreenH));

	// 重设位置
	m_pWindow->SetRect(fcyRect(0.f, 0.f, (float)tScreenW, (float)tScreenH));
}

void AppFrame::Run()
{
	// 若初始化成功，执行循环
	if (m_pEngine)
	{
		m_pWindow->SetVisiable(true);
		m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, 30);  // 限制30FPS以避免CPU消耗
	}

	m_pWindow->SetVisiable(false);

	// 检查线程上的异常并重新抛出
	if (m_bExceptionThrown)
	{
		m_bExceptionThrown = false;
		throw m_threadException;
	}
}

fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)ynothrow
{
	// 每3秒重新置顶一次
	m_fTopMostTimer -= (float)ElapsedTime;
	if (m_fTopMostTimer <= 0.f)
	{
		m_fTopMostTimer = max(1.f, m_pConfig->GetAutoTopMostTime());
		SetWindowPos((HWND)m_pWindow->GetHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	try
	{
		switch (m_pDanmakuReceiver->GetState())
		{
		case DanmakuReceiverState::Idle:
			m_pDanmakuReceiver->DoFetch();
			break;
		case DanmakuReceiverState::Error:
			if (m_fReconnectTimer <= 0.f)
			{
				m_fReconnectTimer = m_pConfig->GetReconnectTime();
				m_pDanmakuPool->SendDanmaku(
					DanmakuData(m_pConfig->GetReconnectInfo(), 0xFFFF0000, DanmakuStyle::TopFloat, DanmakuFontSize::Small));
			}
			else
			{
				m_fReconnectTimer -= (float)ElapsedTime;
				if (m_fReconnectTimer <= 0.f)
					m_pDanmakuReceiver->DoFetch();
			}
			break;
		default:
			break;
		}

		// 获取弹幕
		DanmakuData tData;
		while (m_pDanmakuReceiver->ReceiveDanmaku(tData))
			m_pDanmakuPool->SendDanmaku(tData);

		// 更新弹幕池
		m_pDanmakuPool->Update((float)ElapsedTime);
	}
	catch (const fcyException& e)
	{
		m_bExceptionThrown = true;
		m_threadException = e;
		return false;
	}

	return true;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)ynothrow
{
	if (m_pDanmakuPool->IsIdle())
		return false;

	m_pRenderDev->Clear();
	m_pGraph2d->Begin();

	try
	{
		// 渲染弹幕池
		m_pDanmakuPool->Render(m_pGraph2d);
	}
	catch (const fcyException& e)
	{
		m_bExceptionThrown = true;
		m_threadException = e;
		m_pEngine->Abort();
	}

	m_pGraph2d->End();

	// 直接更新到窗口
	m_pRenderDev->UpdateScreenToWindow(0, (fByte)m_pConfig->GetDanmakuAlpha());

	return false;
}

void AppFrame::OnClearDanmakuClicked()
{
	m_pDanmakuPool->ClearDanmaku();
}

void AppFrame::OnExitClicked()
{
	m_pEngine->Abort();
}
