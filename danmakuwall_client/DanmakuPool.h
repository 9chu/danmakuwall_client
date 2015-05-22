#pragma once
#include "Common.h"
#include "Config.h"

namespace DanmakuWall
{
	class DanmakuPool;

	/// \brief 弹幕样式
	enum class DanmakuStyle
	{
		Classical = 0,
		TopFloat = 1,
		BottomFloat = 2
	};

	/// \brief 字体样式
	enum class DanmakuFontSize
	{
		Tiny = 0,
		Small = 1,
		Normal = 2,
		Large = 3,
		Huge = 4
	};

	/// \brief 弹幕原始数据
	class DanmakuData
	{
	private:
		std::wstring m_strComment;  // 评论文本
		fcyColor m_BlendColor;  // 混合颜色
		DanmakuStyle m_iType;  // 样式
		DanmakuFontSize m_iSize;  // 字体大小
	public:
		DanmakuData& operator=(DanmakuData&& right)
		{
			m_strComment = std::move(right.m_strComment);
			m_BlendColor = right.m_BlendColor;
			m_iType = right.m_iType;
			m_iSize = right.m_iSize;
			return *this;
		}
	public:
		const std::wstring& GetComment()const ynothrow { return m_strComment; }
		fcyColor GetBlendColor()const ynothrow { return m_BlendColor; }
		DanmakuStyle GetStyle()const ynothrow { return m_iType; }
		DanmakuFontSize GetFontSize()const ynothrow { return m_iSize; }
	private:
		DanmakuData(const DanmakuData&);
		DanmakuData& operator=(const DanmakuData&);
	public:
		DanmakuData()
			: m_iType(DanmakuStyle::Classical), m_iSize(DanmakuFontSize::Normal) {}
		DanmakuData(DanmakuData&& right)
			: m_strComment(std::move(right.m_strComment)), m_BlendColor(right.m_BlendColor), 
			m_iType(right.m_iType), m_iSize(right.m_iSize) {}
		DanmakuData(std::wstring Comment, fcyColor Color, DanmakuStyle Style, DanmakuFontSize Size)
			: m_strComment(Comment), m_BlendColor(Color), m_iType(Style), m_iSize(Size) {}
	};

	/// \brief 弹幕
	class Danmaku
	{
	private:
		fcyVec2 m_vPosition;  // 左上角坐标
		fcyVec2 m_vSize;  // 外边框大小

		fcyColor m_cBlendColor;  // 混合颜色
		DanmakuStyle m_iStyle;  // 样式
		DanmakuFontSize m_iFontSize;  // 字体大小

		fcyRefPointer<f2dFontProvider> m_pFont;  // 绑定的字体
		std::vector<std::wstring> m_strLines;  // 每行文本
		std::vector<fcyVec2> m_vDrawPositions;  // 绘图位置（相对于左上角）

		bool m_bDead = false;
		float m_fLifetime = 0.f;
		float m_fVelocity = 0.f;

		Danmaku** m_pHead = nullptr;
		Danmaku** m_pTail = nullptr;
		Danmaku* m_pPrev = nullptr;
		Danmaku* m_pNext = nullptr;
	private:
		void linkToList()ynothrow;
		void removeLink()ynothrow;
		void updatePosition()ynothrow;
	public:
		fcyVec2 GetPosition()const ynothrow { return m_vPosition; }
		void SetPosition(fcyVec2 pos)ynothrow
		{
			m_vPosition = pos;
			updatePosition();
		}
		fcyVec2 GetSize()const ynothrow { return m_vSize; }
		DanmakuStyle GetStyle()const ynothrow { return m_iStyle; }
		Danmaku*& GetLinkPrev()ynothrow { return m_pPrev; }
		Danmaku*& GetLinkNext()ynothrow { return m_pNext; }
		/// \brief 更新弹幕池
		bool Update(float ElapsedTime)ynothrow;
		/// \brief 渲染弹幕池
		void Render(f2dGraphics2D* pGraph2d, f2dFontRenderer* pRenderer)ynothrow;
	private:
		Danmaku(const Danmaku&);
		Danmaku& operator=(const Danmaku&);
	public:
		/// \brief 构造弹幕
		/// \param orgData 弹幕原始数据
		/// \param pConfigInfo 配置信息，用于布局
		/// \param pLinkHead 链表首节点
		/// \param pLinkTail 链表尾节点
		Danmaku(DanmakuPool* pParent, 
			const DanmakuData& orgData,
			Danmaku** pLinkHead = nullptr,
			Danmaku** pLinkTail = nullptr);
		Danmaku(Danmaku&& org);
		~Danmaku();
	};

	/// \brief 弹幕池
	class DanmakuPool
	{
	private:
		f2dRenderer* m_pRenderer;
		Config* m_pConfig;

		// 屏幕大小
		fcyVec2 m_ScreenSize;
		
		// 保存五种大小的弹幕字体
		fcyRefPointer<f2dFontProvider> m_pFonts[5];

		// 字体渲染器
		fcyRefPointer<f2dFontRenderer> m_pFontRenderer;

		// 弹幕池
		bool m_bClearInvoke = false;
		bool m_bIdle = true;
		Danmaku* m_pTopDanmakuLinkHead = nullptr;
		Danmaku* m_pTopDanmakuLinkTail = nullptr;
		Danmaku* m_pBottomDanmakuLinkHead = nullptr;
		Danmaku* m_pBottomDanmakuLinkTail = nullptr;
		std::list<Danmaku> m_DanmakuPool;

		// 经典弹幕发射状态
		float m_fLaunchOffsetY = 0.f;
		float m_fLaunchResetTimer = 0.f;
	public:
		const Config* GetConfig()const ynothrow { return m_pConfig; }
		fcyRefPointer<f2dFontProvider>* GetFontList()ynothrow { return m_pFonts; }
		fcyRefPointer<f2dFontRenderer> GetFontRenderer()ynothrow { return m_pFontRenderer; }

		/// \brief 检查弹幕池是否空闲
		bool IsIdle()ynothrow;
		/// \brief 更新弹幕池
		void Update(float ElapsedTime)ynothrow;
		/// \brief 渲染弹幕池
		void Render(f2dGraphics2D* pGraph2d)ynothrow;

		/// \brief 发送弹幕
		void SendDanmaku(const DanmakuData& data);
		/// \brief 清空弹幕池
		void ClearDanmaku();

		/// \brief 设置大小
		void Resize(const fcyVec2& size)ynothrow
		{
			m_ScreenSize = size;
			m_fLaunchOffsetY = 0.f;
			m_fLaunchResetTimer = 0.f;
		}
	public:
		DanmakuPool(f2dRenderer* pRenderer, Config* pConfig);
	};
}
