#include "AppFrame.h"

using namespace std;
using namespace DanmakuWall;

AppFrame::AppFrame()
{
	// ��ʼ������
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			// ��DLL֡ջ�������׳��쳣��ע��������ABI��������
			throw fcyException(Src, Desc);
		}
	} tErrListener;

	// ��ʼ������
	CreateF2DEngineAndInit(
		F2DVERSION,
		fcyRect(0.f, 0.f, 50.f, 50.f),
		L"��Ļǽ ��Ļ����",
		true,
		true,
		F2DAALEVEL_NONE,
		this,
		&m_pEngine,
		&tErrListener
		);

	// ������
	m_pWindow = m_pEngine->GetMainWindow();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pRenderDev = m_pRenderer->GetDevice();

	// ��ʼ����Ⱦ��
	if (FCYFAILED(m_pRenderDev->CreateGraphics2D(0, 0, &m_pGraph2d)))
		throw fcyException("AppFrame::AppFrame", "CreateGraphics2D failed.");

	// ���ô�����ʽ
	SetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	SetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE, 
		GetWindowLong((HWND)m_pWindow->GetHandle(), GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);  // ���ò������+��괩͸
	m_pWindow->SetBorderType(F2DWINBORDERTYPE_NONE);
	m_pWindow->SetTopMost(true);

	// װ�������ļ�
	m_pConfig = make_unique<Config>(L"config.json");

	// ��ʼ����Ļ��
	m_pDanmakuPool = make_unique<DanmakuPool>(m_pRenderer, m_pConfig.get());

	// ��ʼ����Ļ������
	m_pDanmakuReceiver = make_unique<DanmakuReceiver>(m_pConfig.get());

	// ��������ͼ��
	m_pNotifyIcon = make_unique<NotifyIcon>(m_pConfig.get(), this);

	// �����С������������Ļ
	resizeWindow();
}

void AppFrame::resizeWindow()ynothrow
{
	// ͨ��API��ô��ڴ�С
	fuInt tScreenW = GetSystemMetrics(SM_CXSCREEN);
	fuInt tScreenH = GetSystemMetrics(SM_CYSCREEN);

	// �����С
	m_pRenderDev->SetBufferSize(tScreenW, tScreenH, true, true, F2DAALEVEL_NONE);
	m_pGraph2d->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(
		0.f,
		(float)tScreenW,
		(float)tScreenH,
		0.f, 0.f, 100.f
		));
	m_pDanmakuPool->Resize(fcyVec2((float)tScreenW, (float)tScreenH));

	// ����λ��
	m_pWindow->SetRect(fcyRect(0.f, 0.f, (float)tScreenW, (float)tScreenH));
}

void AppFrame::Run()
{
	// ����ʼ���ɹ���ִ��ѭ��
	if (m_pEngine)
	{
		m_pWindow->SetVisiable(true);
		m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, 30);  // ����30FPS�Ա���CPU����
	}

	m_pWindow->SetVisiable(false);

	// ����߳��ϵ��쳣�������׳�
	if (m_bExceptionThrown)
	{
		m_bExceptionThrown = false;
		throw m_threadException;
	}
}

fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)ynothrow
{
	// ÿ3�������ö�һ��
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

		// ��ȡ��Ļ
		DanmakuData tData;
		while (m_pDanmakuReceiver->ReceiveDanmaku(tData))
			m_pDanmakuPool->SendDanmaku(tData);

		// ���µ�Ļ��
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
		// ��Ⱦ��Ļ��
		m_pDanmakuPool->Render(m_pGraph2d);
	}
	catch (const fcyException& e)
	{
		m_bExceptionThrown = true;
		m_threadException = e;
		m_pEngine->Abort();
	}

	m_pGraph2d->End();

	// ֱ�Ӹ��µ�����
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
