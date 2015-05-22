#pragma once
#include "Common.h"

namespace DanmakuWall
{
	/// \brief 配置文件
	class Config
	{
	private:
		std::wstring m_Filename;  // 文件名

		// 配置信息
		std::wstring m_strFetchApiUrl = L"http://127.0.0.1:8159/api/fetch_comment";
		std::wstring m_strFetchAuthKey = L"password";
		float m_fFetchTimeout = 120.f;
		float m_fReconnectTime = 30.f;
		std::wstring m_strReconnectInfo = L"弹幕墙连接失败，稍后重试...";
		float m_fAutoTopMostTime = 3.f;
		float m_fDanmakuAlpha = 200.f;
		std::wstring m_Font = L"Microsoft YaHei Bold";
		fcyVec2 m_vMargin = fcyVec2(3.f, 3.f);
		float m_fInnerMargin = 2.f;
		fcyVec2 m_vScreenPadding = fcyVec2(5.f, 5.f);
		float m_fFontSize[5];
		float m_fTopDanmakuLifetime = 4.f;
		float m_fBottomDanmakuLifetime = 4.f;
		float m_fClassicalDanmakuVelocityFactor = 0.2f;
		float m_fClassicalDanmakuBaseVelocity = 100.f;
		float m_fClassicalDanmakuPositionResetTime = 4.f;
		std::wstring m_strTrayIconTitle = L"弹幕墙";
		std::wstring m_strMenuStripClear = L"清空弹幕池";
		std::wstring m_strMenuStripExit = L"退出";
	public:
		const std::wstring& GetFetchApiUrl()const ynothrow { return m_strFetchApiUrl; }
		const std::wstring& GetFetchAuthKey()const ynothrow { return m_strFetchAuthKey; }
		float GetFetchTimeout()const ynothrow { return m_fFetchTimeout; }
		float GetReconnectTime()const ynothrow { return m_fReconnectTime; }
		const std::wstring& GetReconnectInfo()const ynothrow { return m_strReconnectInfo; }
		float GetAutoTopMostTime()const ynothrow { return m_fAutoTopMostTime; }
		float GetDanmakuAlpha()const ynothrow { return m_fDanmakuAlpha; }
		const std::wstring& GetFont()const ynothrow { return m_Font; }
		fcyVec2 GetMargin()const ynothrow { return m_vMargin; }
		float GetInnerMargin()const ynothrow { return m_fInnerMargin; }
		fcyVec2 GetScreenPadding()const ynothrow { return m_vScreenPadding; }
		const float* GetFontSize()const ynothrow { return m_fFontSize; }
		float GetTopDanmakuLifetime()const ynothrow { return m_fTopDanmakuLifetime; }
		float GetBottomDanmakuLifetime()const ynothrow { return m_fBottomDanmakuLifetime; }
		float GetClassicalDanmakuVelocityFactor()const ynothrow { return m_fClassicalDanmakuVelocityFactor; }
		float GetClassicalDanmakuBaseVelocity()const ynothrow { return m_fClassicalDanmakuBaseVelocity; }
		float GetClassicalDanmakuPositionResetTime()const ynothrow { return m_fClassicalDanmakuPositionResetTime; }
		const std::wstring& GetTrayIconTitle()const ynothrow { return m_strTrayIconTitle; }
		const std::wstring& GetTrayMenuStripClear()const ynothrow { return m_strMenuStripClear; }
		const std::wstring& GetTrayMenuStripExit()const ynothrow { return m_strMenuStripExit; }
	public:
		void Load();
	public:
		Config(const std::wstring& Filename);
	};
}
