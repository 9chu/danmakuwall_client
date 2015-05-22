#include "Config.h"

using namespace std;
using namespace DanmakuWall;

class ConfigReadHelper
{
private:
	fcyJsonValue* m_pObj;
public:
	ConfigReadHelper operator[](fuInt iIndex)
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_LIST)
			throw fcyException("ConfigReadHelper::operator[]", "Invalid file format.");
		return m_pObj->ToList()->GetValue(iIndex);
	}
	ConfigReadHelper operator[](fcStrW iKey)
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_DICT)
			throw fcyException("ConfigReadHelper::operator[]", "Invalid file format.");
		return m_pObj->ToDict()->GetValue(iKey);
	}
	fDouble ToNumber()
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_NUMBER)
			throw fcyException("ConfigReadHelper::ToNumber", "Invalid file format.");
		return m_pObj->ToNumber();
	}
	fBool ToBoolean()
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_BOOL)
			throw fcyException("ConfigReadHelper::ToBoolean", "Invalid file format.");
		return m_pObj->ToBool();
	}
	fcStrW ToString()
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_STRING)
			throw fcyException("ConfigReadHelper::ToString", "Invalid file format.");
		return m_pObj->ToString()->GetStr();
	}
	fcyVec2 ToVec2()
	{
		if (m_pObj->GetType() != FCYJSONVALUETYPE_LIST)
			throw fcyException("ConfigReadHelper::ToVec2", "Invalid file format.");
		fcyJsonList* pVecList = m_pObj->ToList();
		if (pVecList->GetCount() != 2 || pVecList->GetValue(0)->GetType() != FCYJSONVALUETYPE_NUMBER ||
			pVecList->GetValue(1)->GetType() != FCYJSONVALUETYPE_NUMBER)
			throw fcyException("ConfigReadHelper::ToVec2", "Invalid file format.");
		return fcyVec2((float)pVecList->GetValue(0)->ToNumber(), (float)pVecList->GetValue(1)->ToNumber());
	}
public:
	ConfigReadHelper(fcyJsonValue* pObj)
		: m_pObj(pObj)
	{
		if (!m_pObj)
			throw fcyException("ConfigReadHelper::ConfigReadHelper", "Invalid file format.");
	}	
};

Config::Config(const std::wstring& Filename)
	: m_Filename(Filename)
{
	m_fFontSize[0] = 20.f;
	m_fFontSize[1] = 26.f;
	m_fFontSize[2] = 32.f;
	m_fFontSize[3] = 38.f;
	m_fFontSize[4] = 44.f;

	Load();
}

void Config::Load()
{
	fcyRefPointer<fcyFileStream> pFile;
	pFile.DirectSet(new fcyFileStream(m_Filename.c_str(), false));
	fcyJson tJsonFile(pFile);
	ConfigReadHelper tReadHelper(tJsonFile.GetRoot());

	// 获取配置信息
	std::wstring tStrFetchApiUrl = tReadHelper[L"fetchApiUrl"].ToString();
	std::wstring tStrFetchAuthKey = tReadHelper[L"fetchAuthKey"].ToString();
	float tFetchTimeout = (float)tReadHelper[L"fetchTimeout"].ToNumber();
	float tReconnectTime = (float)tReadHelper[L"reconnectTime"].ToNumber();
	std::wstring tReconnectInfo = tReadHelper[L"reconnectInfo"].ToString();
	float tAutoTopMostTime = (float)tReadHelper[L"autoTopMostTime"].ToNumber();
	float tDanmakuAlpha = (float)tReadHelper[L"danmakuAlpha"].ToNumber();
	std::wstring tFont = tReadHelper[L"styles"][L"font"].ToString();
	fcyVec2 tMargin = tReadHelper[L"styles"][L"margin"].ToVec2();
	float tInnerMargin = (float)tReadHelper[L"styles"][L"innerMargin"].ToNumber();
	fcyVec2 tScreenPadding = tReadHelper[L"styles"][L"screenPadding"].ToVec2();
	float tFontSize[5] = {
		(float)tReadHelper[L"styles"][L"fontSize"][L"tiny"].ToNumber(),
		(float)tReadHelper[L"styles"][L"fontSize"][L"small"].ToNumber(),
		(float)tReadHelper[L"styles"][L"fontSize"][L"normal"].ToNumber(),
		(float)tReadHelper[L"styles"][L"fontSize"][L"large"].ToNumber(),
		(float)tReadHelper[L"styles"][L"fontSize"][L"huge"].ToNumber()
	};
	float tTopDanmakuLifetime = (float)tReadHelper[L"danmaku"][L"top"][L"lifetime"].ToNumber();
	float tBottomDanmakuLifetime = (float)tReadHelper[L"danmaku"][L"bottom"][L"lifetime"].ToNumber();
	float tClassicalDanmakuVelocityFactor = (float)tReadHelper[L"danmaku"][L"classical"][L"velocityFactor"].ToNumber();
	float tClassicalDanmakuBaseVelocity = (float)tReadHelper[L"danmaku"][L"classical"][L"baseVelocity"].ToNumber();
	float tClassicalDanmakuPositionResetTime = (float)tReadHelper[L"danmaku"][L"classical"][L"positionResetTime"].ToNumber();
	std::wstring tTrayIconTitle = tReadHelper[L"trayIcon"][L"title"].ToString();
	std::wstring tMenuStripClear = tReadHelper[L"trayIcon"][L"menuStrip"][L"clear"].ToString();
	std::wstring tMenuStripExit = tReadHelper[L"trayIcon"][L"menuStrip"][L"exit"].ToString();

	// 拷贝并更新
	m_strFetchApiUrl = tStrFetchApiUrl;
	m_strFetchAuthKey = tStrFetchAuthKey;
	m_fFetchTimeout = tFetchTimeout;
	m_fReconnectTime = tReconnectTime;
	m_strReconnectInfo = tReconnectInfo;
	m_fAutoTopMostTime = tAutoTopMostTime;
	m_fDanmakuAlpha = tDanmakuAlpha;
	m_Font = tFont;
	m_vMargin = tMargin;
	m_fInnerMargin = tInnerMargin;
	m_vScreenPadding = tScreenPadding;
	memcpy(m_fFontSize, tFontSize, sizeof(tFontSize));
	m_fTopDanmakuLifetime = tTopDanmakuLifetime;
	m_fBottomDanmakuLifetime = tBottomDanmakuLifetime;
	m_fClassicalDanmakuVelocityFactor = tClassicalDanmakuVelocityFactor;
	m_fClassicalDanmakuBaseVelocity = tClassicalDanmakuBaseVelocity;
	m_fClassicalDanmakuPositionResetTime = tClassicalDanmakuPositionResetTime;
	m_strTrayIconTitle = tTrayIconTitle;
	m_strMenuStripClear = tMenuStripClear;
	m_strMenuStripExit = tMenuStripExit;

	if (m_fDanmakuAlpha < 50.f)
		m_fDanmakuAlpha = 50.f;
	else if (m_fDanmakuAlpha > 255.f)
		m_fDanmakuAlpha = 255.f;
}
