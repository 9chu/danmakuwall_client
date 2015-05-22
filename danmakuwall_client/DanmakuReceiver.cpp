#include "DanmakuReceiver.h"

using namespace std;
using namespace DanmakuWall;

DanmakuReceiver::DanmakuReceiver(Config* pConfig)
	: m_pConfig(pConfig)
{
	// �ֽ�����
	static wregex s_UrlPattern(L"^http://(.+?)(:(\\d+?)){0,1}(/.+)$");
	wsmatch tMatchResult;
	if (!std::regex_search(pConfig->GetFetchApiUrl(), tMatchResult, s_UrlPattern))
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "Invalid danmaku fetch url.");
	m_strHost = tMatchResult[1].str();
	if (tMatchResult[3].length() != 0)
		m_iPort = _wtoi(tMatchResult[3].str().c_str());
	m_strUri = tMatchResult[4].str();
	m_strRequestString = fcyStringHelper::WideCharToMultiByte(L"key=" + pConfig->GetFetchAuthKey(), CP_UTF8);

	// ���WinHTTP֧��
	if (FALSE == WinHttpCheckPlatform())
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpCheckPlatform failed, unsupported platform.");

	// ��ʼ��WINHTTP
	m_hWinHttp = WinHttpOpen(
		L"Danmakuwall Client/0.1",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC);
	if (!m_hWinHttp)
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpOpen failed.");

	// �����û�ָ��
	DWORD_PTR pThis = reinterpret_cast<DWORD_PTR>(this);
	if (FALSE == WinHttpSetOption(m_hWinHttp, WINHTTP_OPTION_CONTEXT_VALUE, &pThis, sizeof(pThis)))
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpSetOption failed.");

	// ���ó�ʱ
	if (FALSE == WinHttpSetTimeouts(m_hWinHttp, 10000, 10000, 30000, (int)(pConfig->GetFetchTimeout() * 1000)))
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpSetTimeouts failed.");

	// ��������
	m_hConnect = WinHttpConnect(
		m_hWinHttp,
		m_strHost.c_str(),
		m_iPort == 0 ? INTERNET_DEFAULT_PORT : m_iPort,
		0);
	if (!m_hConnect)
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpConnect failed.");

	// ��������
	m_hRequest = WinHttpOpenRequest(
		m_hConnect,
		L"POST",
		L"/api/fetch_comment",
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_ESCAPE_PERCENT | WINHTTP_FLAG_REFRESH);
	if (!m_hRequest)
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpOpenRequest failed.");

	// ���ûص�
	if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(
		m_hRequest,
		winhttpCallback,
		WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
		NULL))
	{
		throw fcyException("DanmakuReceiver::DanmakuReceiver", "WinHttpSetStatusCallback failed.");
	}

	m_CloseEvent.Reset();
}

DanmakuReceiver::~DanmakuReceiver()
{
	m_hRequest.Clear();
	m_hConnect.Clear();
	m_hWinHttp.Clear();

	m_CloseEvent.Wait();
}

void DanmakuReceiver::callbackHandler(
	DWORD dwInternetStatus, 
	LPVOID lpvStatusInformation, 
	DWORD dwStatusInformationLength)
{
	switch (dwInternetStatus)
	{
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
		m_LockSec.Lock();
		m_iState = DanmakuReceiverState::Error;
		m_LockSec.UnLock();
		break;
	case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
		m_CloseEvent.Set();
		break;
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		// ��������ϣ���������
		if (FALSE == WinHttpWriteData(
			m_hRequest,
			m_strRequestString.data(),
			m_strRequestString.size(),
			NULL))
		{
			m_LockSec.Lock();
			m_iState = DanmakuReceiverState::Error;
			m_LockSec.UnLock();
		}
		break;
	case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
		// ���ķ�����ϣ��ȴ���Ӧ
		if (FALSE == WinHttpReceiveResponse(m_hRequest, 0))
		{
			m_LockSec.Lock();
			m_iState = DanmakuReceiverState::Error;
			m_LockSec.UnLock();
		}
		break;
	case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
		// ��ͷ��Ӧ
		{
			DWORD statusCode = 0;
			DWORD statusCodeSize = sizeof(DWORD);

			if (FALSE == WinHttpQueryHeaders(
				m_hRequest,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&statusCode,
				&statusCodeSize,
				WINHTTP_NO_HEADER_INDEX))  // ���HTTP״̬��
			{
				m_LockSec.Lock();
				m_iState = DanmakuReceiverState::Error;
				m_LockSec.UnLock();
			}
			else if (HTTP_STATUS_OK != statusCode)
			{
				m_LockSec.Lock();
				m_iState = DanmakuReceiverState::Error;
				m_LockSec.UnLock();
			}
			else if (FALSE == WinHttpReadData(
				m_hRequest,
				m_dataBuffer.data(),
				m_dataBuffer.size(),
				0))  // ��ȡ����
			{
				m_LockSec.Lock();
				m_iState = DanmakuReceiverState::Error;
				m_LockSec.UnLock();
			}
		}
		break;
	case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		// ��������
		{
			if (0 < dwStatusInformationLength)
			{
				handleData(dwStatusInformationLength);

				if (FALSE == WinHttpReadData(
					m_hRequest,
					m_dataBuffer.data(),
					m_dataBuffer.size(),
					0))
				{
					m_LockSec.Lock();
					m_iState = DanmakuReceiverState::Error;
					m_LockSec.UnLock();
				}
			}
			else
				dataReceived();
		}
		break;
	}
}

void DanmakuReceiver::handleData(size_t len)
{
	size_t tOrgSize = m_fullDataBuffer.size();
	m_fullDataBuffer.resize(m_fullDataBuffer.size() + len);
	memcpy((void*)(m_fullDataBuffer.data() + tOrgSize), m_dataBuffer.data(), len);
}

void DanmakuReceiver::dataReceived()
{
	try
	{
		// ��������
		fcyJson tJson(fcyStringHelper::MultiByteToWideChar(m_fullDataBuffer, CP_UTF8));
		fcyJsonList* pRootList = tJson.GetRoot()->ToList();

		if (pRootList)
		{
			for (size_t i = 0; i < pRootList->GetCount(); ++i)
			{
				fcyJsonDict* pDanmaku = pRootList->GetValue(i)->ToDict();
				if (pDanmaku)
				{
					fcyJsonValue* pIp = pDanmaku->GetValue(L"ip");
					fcyJsonValue* pComment = pDanmaku->GetValue(L"comment");
					fcyJsonValue* pColor = pDanmaku->GetValue(L"color");
					fcyJsonValue* pTime = pDanmaku->GetValue(L"time");
					fcyJsonValue* pSize = pDanmaku->GetValue(L"size");
					fcyJsonValue* pType = pDanmaku->GetValue(L"type");

					if (!pIp || !pComment || !pColor || !pTime || !pSize || !pType ||
						pComment->GetType() != FCYJSONVALUETYPE_STRING ||
						pSize->GetType() != FCYJSONVALUETYPE_NUMBER ||
						pType->GetType() != FCYJSONVALUETYPE_NUMBER ||
						pColor->GetType() != FCYJSONVALUETYPE_LIST ||
						pColor->ToList()->GetCount() != 3)
						continue;

					m_LockSec.Lock();
					try
					{
						fcyJsonList* pColorList = pColor->ToList();

						m_DanmakuQueue.emplace(
							pComment->ToString()->GetStr(),
							fcyColor(
								(int)pColorList->GetValue(0)->ToNumber(),
								(int)pColorList->GetValue(1)->ToNumber(),
								(int)pColorList->GetValue(2)->ToNumber()
							),
							static_cast<DanmakuStyle>((int)pType->ToNumber()),
							static_cast<DanmakuFontSize>((int)pSize->ToNumber())
							);
					}
					catch (...)
					{
						// �����쳣
					}
					m_LockSec.UnLock();
				}
			}
		}
	}
	catch (...)
	{
		// ���Դ���
	}
	
	// ������������
	m_fullDataBuffer.clear();
	if (FALSE == WinHttpSendRequest(
		m_hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		m_strRequestString.size(),
		0))
	{
		m_LockSec.Lock();
		m_iState = DanmakuReceiverState::Error;
		m_LockSec.UnLock();
	}
}

void DanmakuReceiver::DoFetch()
{
	m_LockSec.Lock();

	if (m_iState == DanmakuReceiverState::Fetching)
	{
		m_LockSec.UnLock();
		return;
	}

	// ��������
	m_fullDataBuffer.clear();
	if (FALSE == WinHttpSendRequest(
		m_hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		m_strRequestString.size(),
		0))
	{
		m_iState = DanmakuReceiverState::Error;
		m_LockSec.UnLock();
		return;
	}
	else
	{
		m_iState = DanmakuReceiverState::Fetching;
		m_LockSec.UnLock();
		return;
	}
}
