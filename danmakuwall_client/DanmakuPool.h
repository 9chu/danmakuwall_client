#pragma once
#include "Common.h"
#include "Config.h"

namespace DanmakuWall
{
	class DanmakuPool;

	/// \brief ��Ļ��ʽ
	enum class DanmakuStyle
	{
		Classical = 0,
		TopFloat = 1,
		BottomFloat = 2
	};

	/// \brief ������ʽ
	enum class DanmakuFontSize
	{
		Tiny = 0,
		Small = 1,
		Normal = 2,
		Large = 3,
		Huge = 4
	};

	/// \brief ��Ļԭʼ����
	class DanmakuData
	{
	private:
		std::wstring m_strComment;  // �����ı�
		fcyColor m_BlendColor;  // �����ɫ
		DanmakuStyle m_iType;  // ��ʽ
		DanmakuFontSize m_iSize;  // �����С
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

	/// \brief ��Ļ
	class Danmaku
	{
	private:
		fcyVec2 m_vPosition;  // ���Ͻ�����
		fcyVec2 m_vSize;  // ��߿��С

		fcyColor m_cBlendColor;  // �����ɫ
		DanmakuStyle m_iStyle;  // ��ʽ
		DanmakuFontSize m_iFontSize;  // �����С

		fcyRefPointer<f2dFontProvider> m_pFont;  // �󶨵�����
		std::vector<std::wstring> m_strLines;  // ÿ���ı�
		std::vector<fcyVec2> m_vDrawPositions;  // ��ͼλ�ã���������Ͻǣ�

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
		/// \brief ���µ�Ļ��
		bool Update(float ElapsedTime)ynothrow;
		/// \brief ��Ⱦ��Ļ��
		void Render(f2dGraphics2D* pGraph2d, f2dFontRenderer* pRenderer)ynothrow;
	private:
		Danmaku(const Danmaku&);
		Danmaku& operator=(const Danmaku&);
	public:
		/// \brief ���쵯Ļ
		/// \param orgData ��Ļԭʼ����
		/// \param pConfigInfo ������Ϣ�����ڲ���
		/// \param pLinkHead �����׽ڵ�
		/// \param pLinkTail ����β�ڵ�
		Danmaku(DanmakuPool* pParent, 
			const DanmakuData& orgData,
			Danmaku** pLinkHead = nullptr,
			Danmaku** pLinkTail = nullptr);
		Danmaku(Danmaku&& org);
		~Danmaku();
	};

	/// \brief ��Ļ��
	class DanmakuPool
	{
	private:
		f2dRenderer* m_pRenderer;
		Config* m_pConfig;

		// ��Ļ��С
		fcyVec2 m_ScreenSize;
		
		// �������ִ�С�ĵ�Ļ����
		fcyRefPointer<f2dFontProvider> m_pFonts[5];

		// ������Ⱦ��
		fcyRefPointer<f2dFontRenderer> m_pFontRenderer;

		// ��Ļ��
		bool m_bClearInvoke = false;
		bool m_bIdle = true;
		Danmaku* m_pTopDanmakuLinkHead = nullptr;
		Danmaku* m_pTopDanmakuLinkTail = nullptr;
		Danmaku* m_pBottomDanmakuLinkHead = nullptr;
		Danmaku* m_pBottomDanmakuLinkTail = nullptr;
		std::list<Danmaku> m_DanmakuPool;

		// ���䵯Ļ����״̬
		float m_fLaunchOffsetY = 0.f;
		float m_fLaunchResetTimer = 0.f;
	public:
		const Config* GetConfig()const ynothrow { return m_pConfig; }
		fcyRefPointer<f2dFontProvider>* GetFontList()ynothrow { return m_pFonts; }
		fcyRefPointer<f2dFontRenderer> GetFontRenderer()ynothrow { return m_pFontRenderer; }

		/// \brief ��鵯Ļ���Ƿ����
		bool IsIdle()ynothrow;
		/// \brief ���µ�Ļ��
		void Update(float ElapsedTime)ynothrow;
		/// \brief ��Ⱦ��Ļ��
		void Render(f2dGraphics2D* pGraph2d)ynothrow;

		/// \brief ���͵�Ļ
		void SendDanmaku(const DanmakuData& data);
		/// \brief ��յ�Ļ��
		void ClearDanmaku();

		/// \brief ���ô�С
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
