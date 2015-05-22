#pragma once
#include "Common.h"
#include "DanmakuPool.h"

#include <winhttp.h>

namespace DanmakuWall
{
	/// \brief WinHTTP句柄
	class WinHttpHandle
	{
	private:
		HINTERNET m_hHandle;
	public:
		WinHttpHandle& operator=(WinHttpHandle&& org)
		{
			m_hHandle = org.m_hHandle;
			org.m_hHandle = nullptr;
			return *this;
		}
		operator HINTERNET()
		{
			return m_hHandle;
		}
		operator bool()const
		{
			return m_hHandle != nullptr;
		}
		void Clear()
		{
			if (m_hHandle)
				WinHttpCloseHandle(m_hHandle);
			m_hHandle = nullptr;
		}
	private:
		WinHttpHandle& operator=(const WinHttpHandle&);
		WinHttpHandle(const WinHttpHandle&);
	public:
		WinHttpHandle()
			: m_hHandle(nullptr) {}
		WinHttpHandle(HINTERNET hHandle)
			: m_hHandle(hHandle) {}
		WinHttpHandle(WinHttpHandle&& org)
			: m_hHandle(org.m_hHandle)
		{
			org.m_hHandle = nullptr;
		}
		~WinHttpHandle()
		{
			Clear();
		}
	};

	/// \brief 接收器状态
	enum class DanmakuReceiverState
	{
		Idle,
		Error,
		Fetching
	};

	/// \brief 弹幕接收器
	class DanmakuReceiver
	{
	private:
		Config* m_pConfig = nullptr;

		std::wstring m_strHost;
		fuShort m_iPort = 0u;
		std::wstring m_strUri;
		std::string m_strRequestString;

		WinHttpHandle m_hWinHttp;
		WinHttpHandle m_hConnect;
		WinHttpHandle m_hRequest;

		std::array<uint8_t, 8 * 1024> m_dataBuffer;
		std::string m_fullDataBuffer;

		fcyCriticalSection m_LockSec;
		DanmakuReceiverState m_iState = DanmakuReceiverState::Idle;
		std::queue<DanmakuData> m_DanmakuQueue;

		fcyEvent m_CloseEvent;
	private:
		static void CALLBACK winhttpCallback(
			HINTERNET hInternet,
			DWORD_PTR dwContext,
			DWORD dwInternetStatus,
			LPVOID lpvStatusInformation,
			DWORD dwStatusInformationLength)
		{
			DanmakuReceiver* pThis = reinterpret_cast<DanmakuReceiver*>(dwContext);
			if (pThis)
			{
				pThis->callbackHandler(
					dwInternetStatus,
					lpvStatusInformation,
					dwStatusInformationLength);
			}
		}
	private:
		void callbackHandler(
			DWORD dwInternetStatus,
			LPVOID lpvStatusInformation,
			DWORD dwStatusInformationLength);
		void handleData(size_t len);
		void dataReceived();
	public:
		/// \brief 获取状态
		DanmakuReceiverState GetState()
		{
			DanmakuReceiverState iState;
			m_LockSec.Lock();
			iState = m_iState;
			m_LockSec.UnLock();
			return iState;
		}

		/// \brief 接收一条弹幕
		bool ReceiveDanmaku(DanmakuData& out)
		{
			m_LockSec.Lock();
			if (m_DanmakuQueue.empty())
			{
				m_LockSec.UnLock();
				return false;
			}
			
			out = std::move(m_DanmakuQueue.front());
			m_DanmakuQueue.pop();
			m_LockSec.UnLock();
			return true;
		}

		/// \brief 通知获取弹幕
		/// \note  除非遭遇错误，否则将保持获取状态
		void DoFetch();
	public:
		DanmakuReceiver(Config* pConfig);
		~DanmakuReceiver();
	};
}
