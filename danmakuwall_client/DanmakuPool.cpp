#include "DanmakuPool.h"

using namespace std;
using namespace DanmakuWall;

Danmaku::Danmaku(DanmakuPool* pParent, const DanmakuData& orgData, Danmaku** pLinkHead, Danmaku** pLinkTail)
	: m_pHead(pLinkHead), m_pTail(pLinkTail)
{
	m_cBlendColor = orgData.GetBlendColor();
	m_iStyle = orgData.GetStyle();
	m_iFontSize = orgData.GetFontSize();

	// 分解评论到多行
	fcyStringHelper::StringSplit(orgData.GetComment(), L"\n", true, m_strLines);

	// 剔除空白行
	auto i = m_strLines.begin();
	while (i != m_strLines.end())
	{
		*i = fcyStringHelper::Trim(*i);
		if (i->size() == 0)
			i = m_strLines.erase(i);
		else
			++i;
	}

	// 计算文字布局
	if (m_strLines.size() == 0)
		m_bDead = true;
	else
	{
		auto pFontRenderer = pParent->GetFontRenderer();

		// 获得字体
		m_pFont = pParent->GetFontList()[static_cast<int>(m_iFontSize)];
		pFontRenderer->SetFontProvider(m_pFont);

		// 计算绘图位置
		float tTop = pParent->GetConfig()->GetMargin().x;
		float tWidth = 0.f;
		for (auto i = m_strLines.begin(); i != m_strLines.end(); ++i)
		{
			fcyRect tBoundingBox = pFontRenderer->MeasureString(i->c_str());
			m_vDrawPositions.push_back(fcyVec2(-tBoundingBox.GetWidth() / 2 - tBoundingBox.a.x,
				tTop - tBoundingBox.a.y));
			tWidth = max(tWidth, tBoundingBox.GetWidth());
			tTop += tBoundingBox.GetHeight();
			if ((i + 1) == m_strLines.end())
				tTop += pParent->GetConfig()->GetMargin().y;
			else
				tTop += pParent->GetConfig()->GetInnerMargin();
		}
		m_vSize.Set(tWidth, tTop);

		// 修正坐标
		tWidth /= 2;
		for (auto i = m_vDrawPositions.begin(); i != m_vDrawPositions.end(); ++i)
			(*i).x += tWidth;
	}

	// 根据样式设置生命期或者速度
	switch (m_iStyle)
	{
	case DanmakuStyle::TopFloat:
		m_fLifetime = pParent->GetConfig()->GetTopDanmakuLifetime();
		break;
	case DanmakuStyle::BottomFloat:
		m_fLifetime = pParent->GetConfig()->GetBottomDanmakuLifetime();
		break;
	default:
		m_fVelocity = pParent->GetConfig()->GetClassicalDanmakuBaseVelocity() +
			pParent->GetConfig()->GetClassicalDanmakuVelocityFactor() * m_vSize.y;
		break;
	}

	linkToList();
}

Danmaku::Danmaku(Danmaku&& org)
	: m_vPosition(org.m_vPosition), m_vSize(org.m_vSize), m_cBlendColor(org.m_cBlendColor),
	m_iStyle(org.m_iStyle), m_iFontSize(org.m_iFontSize), m_pFont(org.m_pFont),
	m_strLines(std::move(org.m_strLines)), m_vDrawPositions(std::move(org.m_vDrawPositions)),
	m_bDead(org.m_bDead), m_fLifetime(org.m_fLifetime), m_fVelocity(org.m_fVelocity)
{
	org.m_pFont = nullptr;
	org.m_bDead = true;
}

Danmaku::~Danmaku()
{
	removeLink();
}

void Danmaku::linkToList()ynothrow
{
	if (m_pHead)  // 连接到链表
	{
		assert(m_pTail);

		Danmaku*& pHead = *m_pHead;
		Danmaku*& pTail = *m_pTail;
		if (!pHead)
		{
			assert(!pTail);

			pHead = pTail = this;
		}
		else
		{
			assert(pTail);

			pTail->GetLinkNext() = this;
			m_pPrev = pTail;
			pTail = this;
		}

		// updatePosition();
	}
}

void Danmaku::removeLink()ynothrow
{
	if (m_pHead)
	{
		assert(m_pTail);

		Danmaku*& pHead = *m_pHead;
		Danmaku*& pTail = *m_pTail;
		if (m_pPrev)
			m_pPrev->GetLinkNext() = m_pNext;
		if (m_pNext)
			m_pNext->GetLinkPrev() = m_pPrev;
		if (pHead == this)
			pHead = m_pNext;
		if (pTail == this)
			pTail = m_pPrev;
	}
}

void Danmaku::updatePosition()ynothrow
{
	if (m_pHead)
	{
		assert(m_pTail);

		Danmaku*& pHead = *m_pHead;
		Danmaku*& pTail = *m_pTail;

		// 将节点往前移动
		while (m_pPrev && m_pPrev->GetPosition().y > GetPosition().y)
		{
			Danmaku* pPrev = m_pPrev;
			if (pPrev->GetLinkPrev())
				pPrev->GetLinkPrev()->GetLinkNext() = this;
			if (GetLinkNext())
				GetLinkNext()->GetLinkPrev() = pPrev;
			pPrev->GetLinkNext() = m_pNext;
			m_pPrev = pPrev->GetLinkPrev();
			pPrev->GetLinkPrev() = this;
			m_pNext = pPrev;
			if (pHead == pPrev)
				pHead = this;
			if (pTail == this)
				pTail = pPrev;
		}

		// 将节点往后移动
		while (m_pNext && m_pNext->GetPosition().y < GetPosition().y)
		{
			Danmaku* pNext = m_pNext;
			if (pNext->GetLinkNext())
				pNext->GetLinkNext()->GetLinkPrev() = this;
			if (GetLinkPrev())
				GetLinkPrev()->GetLinkNext() = pNext;
			m_pNext->GetLinkPrev() = m_pPrev;
			m_pNext = pNext->GetLinkNext();
			pNext->GetLinkNext() = this;
			m_pPrev = pNext;
			if (pTail == pNext)
				pTail = this;
			if (pHead == this)
				pHead = pNext;
		}
	}
}

bool Danmaku::Update(float ElapsedTime)ynothrow
{
	if (!m_bDead)
	{
		switch (m_iStyle)
		{
		case DanmakuStyle::TopFloat:
		case DanmakuStyle::BottomFloat:
			m_fLifetime -= ElapsedTime;
			if (m_fLifetime < 0.f)
				m_bDead = true;
			break;
		default:
			m_vPosition.x -= m_fVelocity * ElapsedTime;
			if (m_vPosition.x + m_vSize.x < 0.f)
				m_bDead = true;
			break;
		}
	}

	return !m_bDead;
}

void Danmaku::Render(f2dGraphics2D* pGraph2d, f2dFontRenderer* pRenderer)ynothrow
{
	if (m_pFont)
	{
		pRenderer->SetFontProvider(m_pFont);
		for (size_t i = 0; i < m_vDrawPositions.size(); ++i)
		{
			static fcyVec2 s_ExpandDirections[8] = {
				fcyVec2(-2.f, 0.f),
				fcyVec2(2.f, 0.f),
				fcyVec2(0.f, -2.f),
				fcyVec2(0.f, 2.f),
				fcyVec2(1.f, 1.f),
				fcyVec2(-1.f, 1.f),
				fcyVec2(1.f, -1.f),
				fcyVec2(-1.f, -1.f)
			};

			fcyVec2 tDrawPos = m_vPosition + m_vDrawPositions[i];

			// 绘制底色
			pRenderer->SetColor(m_cBlendColor.argb == 0xFF000000u ? 0xFFFFFFFFu : 0xFF000000u);
			for (int j = 0; j < 8; ++j)
				pRenderer->DrawTextW(pGraph2d, m_strLines[i].c_str(), s_ExpandDirections[j] + tDrawPos);

			// 绘制文字
			pRenderer->SetColor(m_cBlendColor);
			pRenderer->DrawTextW(pGraph2d, m_strLines[i].c_str(), tDrawPos);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

DanmakuPool::DanmakuPool(f2dRenderer* pRenderer, Config* pConfig)
	: m_pRenderer(pRenderer), m_pConfig(pConfig)
{
	// 初始化字体
	for (int i = 0; i < 5; ++i)
	{
		if (FCYFAILED(m_pRenderer->CreateSystemFont(
			m_pConfig->GetFont().c_str(),
			0,
			fcyVec2(m_pConfig->GetFontSize()[i], m_pConfig->GetFontSize()[i]),
			F2DFONTFLAG_NONE,
			&m_pFonts[i])))
			throw fcyException("DanmakuPool::DanmakuPool", "Load system font failed.");
	}
	
	// 创建字体渲染器
	if (FCYFAILED(m_pRenderer->CreateFontRenderer(nullptr, &m_pFontRenderer)))
		throw fcyException("DanmakuPool::DanmakuPool", "Create font renderer failed.");

	m_ScreenSize.Set((float)m_pRenderer->GetDevice()->GetBufferWidth(), (float)m_pRenderer->GetDevice()->GetBufferHeight());
}

bool DanmakuPool::IsIdle()ynothrow
{
	return m_bIdle;
}

void DanmakuPool::Update(float ElapsedTime)ynothrow
{
	bool bIdle = true;

	if (m_bClearInvoke)
	{
		m_DanmakuPool.clear();
		bIdle = false;
		m_bClearInvoke = false;
		return;
	}

	auto i = m_DanmakuPool.begin();
	while (i != m_DanmakuPool.end())
	{
		if (!i->Update(ElapsedTime))
		{
			i = m_DanmakuPool.erase(i);
			bIdle = false;
		}
		else
		{
			if (i->GetStyle() == DanmakuStyle::Classical)
				bIdle = false;
			++i;
		}
	}

	if (m_fLaunchResetTimer > 0.f)
	{
		m_fLaunchResetTimer -= ElapsedTime;
		if (m_fLaunchResetTimer <= 0.f)
			m_fLaunchOffsetY = 0.f;
	}

	m_bIdle = bIdle & m_bIdle;
}

void DanmakuPool::Render(f2dGraphics2D* pGraph2d)ynothrow
{
	for (auto i = m_DanmakuPool.begin(); i != m_DanmakuPool.end(); ++i)
		i->Render(pGraph2d, m_pFontRenderer);

	m_bIdle = true;
}

void DanmakuPool::SendDanmaku(const DanmakuData& data)
{
	m_bIdle = false;

	switch (data.GetStyle())
	{
	case DanmakuStyle::TopFloat:
		// 顶部弹幕
		{
			m_DanmakuPool.emplace_back(this, data, &m_pTopDanmakuLinkHead, &m_pTopDanmakuLinkTail);
			Danmaku& obj = m_DanmakuPool.back();
			
			// 寻找显示位置
			float tOffsetY = m_pConfig->GetScreenPadding().x;
			Danmaku* p = m_pTopDanmakuLinkHead;
			while (p)
			{
				if (p == &obj)
				{
					p = p->GetLinkNext();
					continue;
				}

				if (tOffsetY + p->GetSize().y <= p->GetPosition().y)
					break;
				else
				{
					tOffsetY = p->GetPosition().y + p->GetSize().y;
					p = p->GetLinkNext();
					if (tOffsetY + obj.GetSize().y > m_ScreenSize.y - m_pConfig->GetScreenPadding().y)
						tOffsetY = m_pConfig->GetScreenPadding().x;
				}
			}
			obj.SetPosition(fcyVec2(m_ScreenSize.x / 2 - obj.GetSize().x / 2, tOffsetY));
		}
		break;
	case DanmakuStyle::BottomFloat:
		// 底部弹幕
		{
			m_DanmakuPool.emplace_back(this, data, &m_pTopDanmakuLinkHead, &m_pTopDanmakuLinkTail);
			Danmaku& obj = m_DanmakuPool.back();

			// 寻找显示位置
			float tOffsetY = m_ScreenSize.y - m_pConfig->GetScreenPadding().y - obj.GetSize().y;
			Danmaku* p = m_pTopDanmakuLinkTail;
			while (p)
			{
				if (p == &obj)
				{
					p = p->GetLinkPrev();
					continue;
				}

				if (tOffsetY >= p->GetPosition().y + p->GetSize().y)
					break;
				else
				{
					tOffsetY = p->GetPosition().y - obj.GetSize().y;
					p = p->GetLinkPrev();
					if (tOffsetY < m_pConfig->GetScreenPadding().x)
						tOffsetY = m_ScreenSize.y - m_pConfig->GetScreenPadding().y - obj.GetSize().y;
				}
			}
			obj.SetPosition(fcyVec2(m_ScreenSize.x / 2 - obj.GetSize().x / 2, tOffsetY));
		}
		break;
	default:
		// 在当前位置发射一个经典弹幕
		{
			m_DanmakuPool.emplace_back(this, data);
			Danmaku& obj = m_DanmakuPool.back();
			if (m_fLaunchOffsetY + m_pConfig->GetScreenPadding().x + obj.GetSize().y > m_ScreenSize.y - m_pConfig->GetScreenPadding().y)
			{
				// 发射点越界处理
				m_fLaunchOffsetY = 0.f;
				m_fLaunchResetTimer = 0.f;
			}
			obj.SetPosition(fcyVec2(m_ScreenSize.x, m_fLaunchOffsetY + m_pConfig->GetScreenPadding().x));
			m_fLaunchOffsetY += obj.GetSize().y;
			if (m_fLaunchResetTimer <= 0.f)
				m_fLaunchResetTimer = m_pConfig->GetClassicalDanmakuPositionResetTime();
		}
		break;
	}
}

void DanmakuPool::ClearDanmaku()
{
	m_bClearInvoke = true;
}
