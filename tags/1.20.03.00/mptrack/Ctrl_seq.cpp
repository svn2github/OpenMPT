/*
 * ctrl_seq.cpp
 * ------------
 * Purpose: Order list for the pattern editor upper panel.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "moddoc.h"
#include "globals.h"
#include "ctrl_pat.h"
#include "view_pat.h"

//////////////////////////////////////////////////////////////
// CPatEdit

BOOL CPatEdit::PreTranslateMessage(MSG *pMsg)
//-------------------------------------------
{
	if (((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP)) && (pMsg->wParam == VK_TAB))
	{
		if ((pMsg->message == WM_KEYUP) && (m_pParent))
		{
			m_pParent->SwitchToView();
		}
		return TRUE;
	}
	return CEdit::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////
// COrderList

BEGIN_MESSAGE_MAP(COrderList, CWnd)
	//{{AFX_MSG_MAP(COrderList)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_COMMAND(ID_CONTROLTAB,					OnSwitchToView)

	ON_COMMAND(ID_ORDERLIST_INSERT,				OnInsertOrder)
	ON_COMMAND(ID_ORDERLIST_INSERT_SEPARATOR,	OnInsertSeparatorPattern)
	ON_COMMAND(ID_ORDERLIST_DELETE,				OnDeleteOrder)
	ON_COMMAND(ID_ORDERLIST_RENDER,				OnRenderOrder)
	ON_COMMAND(ID_ORDERLIST_EDIT_COPY,			OnEditCopy)
	ON_COMMAND(ID_ORDERLIST_EDIT_CUT,			OnEditCut)
	ON_COMMAND(ID_ORDERLIST_EDIT_PASTE,			OnEditPaste)

	ON_COMMAND(ID_PATTERN_PROPERTIES,			OnPatternProperties)
	ON_COMMAND(ID_PLAYER_PLAY,					OnPlayerPlay)
	ON_COMMAND(ID_PLAYER_PAUSE,					OnPlayerPause)
	ON_COMMAND(ID_PLAYER_PLAYFROMSTART,			OnPlayerPlayFromStart)
	ON_COMMAND(IDC_PATTERN_PLAYFROMSTART,		OnPatternPlayFromStart)
	//ON_COMMAND(ID_PATTERN_RESTART,			OnPatternPlayFromStart)
	ON_COMMAND(ID_ORDERLIST_NEW,				OnCreateNewPattern)
	ON_COMMAND(ID_ORDERLIST_COPY,				OnDuplicatePattern)
	ON_COMMAND(ID_PATTERNCOPY,					OnPatternCopy)
	ON_COMMAND(ID_PATTERNPASTE,					OnPatternPaste)
	ON_COMMAND(ID_ORDERLIST_LOCKPLAYBACK,		OnLockPlayback)
	ON_COMMAND(ID_ORDERLIST_UNLOCKPLAYBACK,		OnUnlockPlayback)
	ON_COMMAND_RANGE(ID_SEQUENCE_ITEM, ID_SEQUENCE_ITEM + MAX_SEQUENCES + 2, OnSelectSequence)
	ON_MESSAGE(WM_MOD_DRAGONDROPPING,			OnDragonDropping)
	ON_MESSAGE(WM_HELPHITTEST,					OnHelpHitTest)
	ON_MESSAGE(WM_MOD_KEYCOMMAND,				OnCustomKeyMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


bool COrderList::IsOrderInMargins(int order, int startOrder)
//----------------------------------------------------------
{
	const ORDERINDEX nMargins = GetMargins();
	return ((startOrder != 0 && order - startOrder < nMargins) || 
		order - startOrder >= GetLength() - nMargins);
}


void COrderList::EnsureVisible(ORDERINDEX order)
//----------------------------------------------
{
	// nothing needs to be done
	if(!IsOrderInMargins(order, m_nXScroll) || order == ORDERINDEX_INVALID) return;

	if(order < m_nXScroll)
	{
		if(order < GetMargins())
			m_nXScroll = 0;
		else
			m_nXScroll = order - GetMargins();
	} else
	{
		m_nXScroll = order + 2 * GetMargins() - 1;
		if(m_nXScroll < GetLength())
			m_nXScroll = 0;
		else
			m_nXScroll -= GetLength();
	}
}


COrderList::COrderList()
//----------------------
{
	m_hFont = nullptr;
	m_pParent = nullptr;
	m_cxFont = m_cyFont = 0;
	m_pModDoc = nullptr;
	m_nScrollPos = m_nXScroll = 0;
	m_nScrollPos2nd = ORDERINDEX_INVALID;
	m_nOrderlistMargins = CMainFrame::GetSettings().orderlistMargins;
	m_bScrolling = false;
	m_bDragging = false;
}


ORDERINDEX COrderList::GetOrderFromPoint(const CRect& rect, const CPoint& pt) const
//---------------------------------------------------------------------------------
{
	return static_cast<ORDERINDEX>(m_nXScroll + (pt.x - rect.left) / m_cxFont);
}


BOOL COrderList::Init(const CRect &rect, CCtrlPatterns *pParent, CModDoc *pModDoc, HFONT hFont)
//---------------------------------------------------------------------------------------------
{
	CreateEx(WS_EX_STATICEDGE, NULL, "", WS_CHILD|WS_VISIBLE, rect, pParent, IDC_ORDERLIST);
	m_pParent = pParent;
	m_pModDoc = pModDoc;
	m_hFont = hFont;
	colorText = GetSysColor(COLOR_WINDOWTEXT);
	colorInvalid = GetSysColor(COLOR_GRAYTEXT);
	colorTextSel = GetSysColor(COLOR_HIGHLIGHTTEXT);
	SendMessage(WM_SETFONT, (WPARAM)m_hFont);
	SetScrollPos(SB_HORZ, 0);
	EnableScrollBarCtrl(SB_HORZ, TRUE);
	return TRUE;
}


BOOL COrderList::UpdateScrollInfo()
//---------------------------------
{
	CRect rcClient;

	GetClientRect(&rcClient);
	if ((m_pModDoc) && (m_cxFont > 0) && (rcClient.right > 0))
	{
		CRect rect;
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		SCROLLINFO info;
		UINT nPage;

		int nMax = 0;
		if(pSndFile->GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM))
		{   // With MOD / XM, cut shown sequence to first '---' item...
			nMax = pSndFile->Order.GetLengthFirstEmpty();
		}
		else
		{   // ...for S3M/IT/MPT, show sequence until the last used item.
			nMax = pSndFile->Order.GetLengthTailTrimmed();
		}

		GetScrollInfo(SB_HORZ, &info, SIF_PAGE|SIF_RANGE);
		info.fMask = SIF_PAGE|SIF_RANGE;
		info.nMin = 0;
		nPage = rcClient.right / m_cxFont;
		if (nMax <= (int)nPage) nMax = nPage + 1;
		if ((nMax != info.nMax) || (nPage != info.nPage))
		{
			info.nPage = nPage;
			info.nMax = nMax;
			SetScrollInfo(SB_HORZ, &info, TRUE);
		}
	}
	return FALSE;
}


int COrderList::GetFontWidth()
//----------------------------
{
	if ((m_cxFont <= 0) && (m_hWnd) && (m_hFont))
	{
		CClientDC dc(this);
		HGDIOBJ oldfont = ::SelectObject(dc.m_hDC, m_hFont);
		CSize sz = dc.GetTextExtent("000+", 4);
		if (oldfont) ::SelectObject(dc.m_hDC, oldfont);
		return sz.cx;
	}
	return m_cxFont;
}


void COrderList::InvalidateSelection() const
//------------------------------------------
{
	ORDERINDEX nOrdLo = m_nScrollPos, nCount = 1;
	static ORDERINDEX m_nScrollPos2Old = m_nScrollPos2nd;
	if(m_nScrollPos2Old != ORDERINDEX_INVALID)
	{
		// there were multiple orders selected - remove them all
		ORDERINDEX nOrdHi = m_nScrollPos;
		if(m_nScrollPos2Old < m_nScrollPos)
		{
			nOrdLo = m_nScrollPos2Old;
		} else
		{
			nOrdHi = m_nScrollPos2Old;
		}
		nCount = nOrdHi - nOrdLo + 1;
	}
	m_nScrollPos2Old = m_nScrollPos2nd;
	CRect rcClient, rect;
	GetClientRect(&rcClient);
	rect.left = rcClient.left + (nOrdLo - m_nXScroll) * m_cxFont;
	rect.top = rcClient.top;
	rect.right = rect.left + m_cxFont * nCount;
	rect.bottom = rcClient.bottom;
	if (rect.right > rcClient.right) rect.right = rcClient.right;
	if (rect.left < rcClient.left) rect.left = rcClient.left;
	if (rect.right > rect.left) ::InvalidateRect(m_hWnd, &rect, FALSE);
}


ORDERINDEX COrderList::GetLength()
//--------------------------------
{
	CRect rcClient;
	GetClientRect(&rcClient);
	if(m_cxFont > 0)
		return static_cast<ORDERINDEX>(rcClient.right / m_cxFont);
	else
	{
		const int nFontWidth = GetFontWidth();
		return (nFontWidth > 0) ? static_cast<ORDERINDEX>(rcClient.right / nFontWidth) : 0;
	}
}


OrdSelection COrderList::GetCurSel(bool bIgnoreSelection) const
//-------------------------------------------------------------
{
	// returns the currently selected order(s)
	OrdSelection result;
	result.firstOrd = result.lastOrd = m_nScrollPos;
	// bIgnoreSelection: true if only first selection marker is important.
	if(!bIgnoreSelection && m_nScrollPos2nd != ORDERINDEX_INVALID)
	{
		if(m_nScrollPos2nd < m_nScrollPos) // ord2 < ord1
			result.firstOrd = m_nScrollPos2nd;
		else
			result.lastOrd = m_nScrollPos2nd;
	}
	LimitMax(result.firstOrd, m_pModDoc->GetSoundFile()->Order.GetLastIndex());
	LimitMax(result.lastOrd, m_pModDoc->GetSoundFile()->Order.GetLastIndex());
	return result;
}


bool COrderList::SetCurSel(ORDERINDEX sel, bool bEdit, bool bShiftClick, bool bIgnoreCurSel)
//------------------------------------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	ORDERINDEX &nOrder = bShiftClick ? m_nScrollPos2nd : m_nScrollPos;

	if ((sel < 0) || (sel >= pSndFile->Order.GetLength()) || (!m_pParent) || (!pMainFrm)) return false;
	if (!bIgnoreCurSel && sel == nOrder) return true;
	const ORDERINDEX nShownLength = GetLength();
	InvalidateSelection();
	nOrder = sel;

	if (!m_bScrolling)
	{
		const ORDERINDEX nMargins = GetMargins(GetMarginsMax(nShownLength));
		if(nOrder < m_nXScroll + nMargins)
		{
			// Must move first shown sequence item to left in order to show
			// the new active order.
			m_nXScroll = Util::Max(ORDERINDEX(0), static_cast<ORDERINDEX>(nOrder - nMargins));
			SetScrollPos(SB_HORZ, m_nXScroll);
			InvalidateRect(NULL, FALSE);
		} else
		{
			ORDERINDEX maxsel = nShownLength;
			if (maxsel) maxsel--;
			if (nOrder - m_nXScroll >= maxsel - nMargins)
			{
				// Must move first shown sequence item to right in order to show
				// the new active order.
				m_nXScroll = nOrder - (maxsel - nMargins);
				SetScrollPos(SB_HORZ, m_nXScroll);
				InvalidateRect(NULL, FALSE);
			}
		}
	}
	InvalidateSelection();
	if ((m_pParent) && (m_pModDoc) && (bEdit))
	{
		PATTERNINDEX n = pSndFile->Order[m_nScrollPos];
		if ((n < pSndFile->Patterns.Size()) && (pSndFile->Patterns[n]) && !bShiftClick)
		{
			CriticalSection cs;

			bool isPlaying = (pMainFrm->GetModPlaying() == m_pModDoc);
			bool changedPos = false;

			if(isPlaying && pSndFile->m_SongFlags[SONG_PATTERNLOOP])
			{
				// update channel parameters and play time
				m_pModDoc->SetElapsedTime(m_nScrollPos, 0);

				pSndFile->m_nPattern = n;
				pSndFile->m_nCurrentOrder = pSndFile->m_nNextOrder = m_nScrollPos;
				pMainFrm->ResetNotificationBuffer();
				pSndFile->m_nNextRow = 0;

				changedPos = true;
			} else if(m_pParent->GetFollowSong())
			{
				SongFlags pausedFlags = pSndFile->m_SongFlags & (SONG_PAUSED | SONG_STEP | SONG_PATTERNLOOP);

				// update channel parameters and play time
				m_pModDoc->SetElapsedTime(m_nScrollPos, 0);

				pSndFile->m_nCurrentOrder = m_nScrollPos;
				pSndFile->SetCurrentOrder(m_nScrollPos);
				pSndFile->m_SongFlags.set(pausedFlags);

				if(isPlaying) pMainFrm->ResetNotificationBuffer();
				changedPos = true;
			}

			if(changedPos && pSndFile->Order.IsPositionLocked(m_nScrollPos))
			{
				// Users wants to go somewhere else, so let him do that.
				OnUnlockPlayback();
			}

			m_pParent->SetCurrentPattern(n);
		}
	}
	UpdateInfoText();
	if(m_nScrollPos == m_nScrollPos2nd) m_nScrollPos2nd = ORDERINDEX_INVALID;
	return true;
}


UINT COrderList::GetCurrentPattern() const
//----------------------------------------
{
	CSoundFile* pSndFile = m_pModDoc ? m_pModDoc->GetSoundFile() : NULL;
	if ((pSndFile) && (m_nScrollPos < pSndFile->Order.GetLength()))
	{
		return pSndFile->Order[m_nScrollPos];
	}
	return 0;
}


BOOL COrderList::PreTranslateMessage(MSG *pMsg)
//---------------------------------------------
{
	//rewbs.customKeys: 
	//handle Patterns View context keys that we want to take effect in the orderlist.
	if ((pMsg->message == WM_SYSKEYUP)   || (pMsg->message == WM_KEYUP) || 
		(pMsg->message == WM_SYSKEYDOWN) || (pMsg->message == WM_KEYDOWN))
	{
		CInputHandler* ih = (CMainFrame::GetMainFrame())->GetInputHandler();

		//Translate message manually
		UINT nChar = pMsg->wParam;
		UINT nRepCnt = LOWORD(pMsg->lParam);
		UINT nFlags = HIWORD(pMsg->lParam);
		KeyEventType kT = ih->GetKeyEventType(nFlags);

		InputTargetContext ctx = (InputTargetContext)(kCtxCtrlOrderlist);
		if (ih->KeyEvent(ctx, nChar, nRepCnt, nFlags, kT) != kcNull)
			return true; // Mapped to a command, no need to pass message on.

		//HACK: masquerade as kCtxViewPatternsNote context until we implement appropriate
		//      command propagation to kCtxCtrlOrderlist context.

		ctx = (InputTargetContext)(kCtxViewPatternsNote);
		if (ih->KeyEvent(ctx, nChar, nRepCnt, nFlags, kT) != kcNull)
			return true; // Mapped to a command, no need to pass message on.

	}
	//end rewbs.customKeys 

	return CWnd::PreTranslateMessage(pMsg);
}


LRESULT COrderList::OnCustomKeyMsg(WPARAM wParam, LPARAM)
//-------------------------------------------------------
{
	if (wParam == kcNull)
		return 0;

	switch(wParam)
	{
	case kcEditCopy:
		OnEditCopy(); return wParam;
	case kcEditCut:
		OnEditCut(); return wParam;
	case kcEditPaste:
		OnEditPaste(); return wParam;

	// Orderlist navigation
	case kcOrderlistNavigateLeftSelect:
	case kcOrderlistNavigateLeft:
		SetCurSelTo2ndSel(wParam == kcOrderlistNavigateLeftSelect); SetCurSel(m_nScrollPos - 1); return wParam;
	case kcOrderlistNavigateRightSelect:
	case kcOrderlistNavigateRight:
		SetCurSelTo2ndSel(wParam == kcOrderlistNavigateRightSelect); SetCurSel(m_nScrollPos + 1); return wParam;
	case kcOrderlistNavigateFirstSelect:
	case kcOrderlistNavigateFirst:
		SetCurSelTo2ndSel(wParam == kcOrderlistNavigateFirstSelect); SetCurSel(0); return wParam;
	case kcEditSelectAll:
		SetCurSel(0);
		// fallthroughs intended.
	case kcOrderlistNavigateLastSelect:
	case kcOrderlistNavigateLast:
		if((m_pModDoc != nullptr) && (m_pModDoc->GetSoundFile() != nullptr))
		{
			SetCurSelTo2ndSel(wParam == kcOrderlistNavigateLastSelect || wParam == kcEditSelectAll);
			ORDERINDEX nLast = m_pModDoc->GetSoundFile()->Order.GetLengthTailTrimmed();
			if(nLast > 0) nLast--;
			SetCurSel(nLast);
		}
		return wParam;

	// Orderlist edit
	case kcOrderlistEditDelete:
		OnDeleteOrder(); return wParam;
	case kcOrderlistEditInsert:
		OnInsertOrder(); return wParam;
	case kcOrderlistSwitchToPatternView:
		OnSwitchToView(); return wParam;
	case kcOrderlistEditPattern:
		OnLButtonDblClk(0, CPoint(0,0)); OnSwitchToView(); return wParam;

	// Enter pattern number
	case kcOrderlistPat0:
	case kcOrderlistPat1:
	case kcOrderlistPat2:
	case kcOrderlistPat3:
	case kcOrderlistPat4:
	case kcOrderlistPat5:
	case kcOrderlistPat6:
	case kcOrderlistPat7:
	case kcOrderlistPat8:
	case kcOrderlistPat9:
		EnterPatternNum(wParam - kcOrderlistPat0); return wParam;
	case kcOrderlistPatMinus:
		EnterPatternNum(10); return wParam;
	case kcOrderlistPatPlus:
		EnterPatternNum(11); return wParam;
	case kcOrderlistPatIgnore:
		EnterPatternNum(12); return wParam;
	case kcOrderlistPatInvalid:
		EnterPatternNum(13); return wParam;

	// kCtxViewPatternsNote messages
	case kcSwitchToOrderList:
		OnSwitchToView();
		return wParam;
	case kcChangeLoopStatus:
		m_pParent->OnModCtrlMsg(CTRLMSG_PAT_LOOP, -1); return wParam;
	case kcToggleFollowSong:
		m_pParent->OnModCtrlMsg(CTRLMSG_PAT_FOLLOWSONG, 1); return wParam;

	case kcChannelUnmuteAll:
	case kcUnmuteAllChnOnPatTransition:
		::PostMessage(m_pParent->GetViewWnd(), WM_MOD_KEYCOMMAND, wParam, 0); return wParam;

	case kcOrderlistLockPlayback:
		OnLockPlayback(); return wParam;
	case kcOrderlistUnlockPlayback:
		OnUnlockPlayback(); return wParam;

	case kcDuplicatePattern:
		OnDuplicatePattern(); return wParam;
	case kcNewPattern:
		OnCreateNewPattern(); return wParam;
	}

	return 0;
}


// Helper function to enter pattern index into the orderlist.
// Call with param 0...9 (enter digit), 10 (decrease) or 11 (increase).
void COrderList::EnterPatternNum(int enterNum)
//--------------------------------------------
{
	if (m_pModDoc == nullptr) return;
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	if(pSndFile == nullptr) return;

	PATTERNINDEX nCurNdx = (m_nScrollPos < pSndFile->Order.GetLength()) ? pSndFile->Order[m_nScrollPos] : pSndFile->Order.GetInvalidPatIndex();
	PATTERNINDEX nMaxNdx = 0;
	for(PATTERNINDEX nPat = 0; nPat < pSndFile->Patterns.Size(); nPat++)
		if (pSndFile->Patterns.IsValidPat(nPat)) nMaxNdx = nPat;

	if (enterNum >= 0 && enterNum <= 9) // enter 0...9
	{
		if (nCurNdx >= pSndFile->Patterns.Size()) nCurNdx = 0;

		nCurNdx = nCurNdx * 10 + static_cast<PATTERNINDEX>(enterNum);
		STATIC_ASSERT(MAX_PATTERNS < 10000);
		if ((nCurNdx >= 1000) && (nCurNdx > nMaxNdx)) nCurNdx %= 1000;
		if ((nCurNdx >= 100) && (nCurNdx > nMaxNdx)) nCurNdx %= 100;
		if ((nCurNdx >= 10) && (nCurNdx > nMaxNdx)) nCurNdx %= 10;
	} else if (enterNum == 10) // decrease pattern index
	{
		const PATTERNINDEX nFirstInvalid = pSndFile->GetModSpecifications().hasIgnoreIndex ? pSndFile->Order.GetIgnoreIndex() : pSndFile->Order.GetInvalidPatIndex();
		if (nCurNdx == 0)
			nCurNdx = pSndFile->Order.GetInvalidPatIndex();
		else
		{
			nCurNdx--;
			if ((nCurNdx > nMaxNdx) && (nCurNdx < nFirstInvalid)) nCurNdx = nMaxNdx;
		}
	} else if (enterNum == 11) // increase pattern index
	{
		if(nCurNdx >= pSndFile->Order.GetInvalidPatIndex())
		{
			nCurNdx = 0;
		}
		else
		{
			nCurNdx++;
			const PATTERNINDEX nFirstInvalid = pSndFile->GetModSpecifications().hasIgnoreIndex ? pSndFile->Order.GetIgnoreIndex() : pSndFile->Order.GetInvalidPatIndex();
			if(nCurNdx > nMaxNdx && nCurNdx < nFirstInvalid)
				nCurNdx = nFirstInvalid;
		}
	} else if (enterNum == 12) // ignore index (+++)
	{
		if (pSndFile->GetModSpecifications().hasIgnoreIndex)
		{
			nCurNdx = pSndFile->Order.GetIgnoreIndex();
		}
	} else if (enterNum == 13) // invalid index (---)
	{
		nCurNdx = pSndFile->Order.GetInvalidPatIndex();
	}
	// apply
	if (nCurNdx != pSndFile->Order[m_nScrollPos])
	{
		pSndFile->Order[m_nScrollPos] = nCurNdx;
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);
		InvalidateSelection();
	}
}


static const char szClipboardOrdersHdr[] = "OpenMPT %3s\r\n";
static const char szClipboardOrdCountFieldHdr[]	= "OrdNum: %u\r\n";
static const char szClipboardOrdersFieldHdr[]	= "OrdLst: ";


void COrderList::OnEditCut()
//--------------------------
{
	OnEditCopy();
	OnDeleteOrder();
}


void COrderList::OnEditPaste()
//----------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CSoundFile* pSf = m_pModDoc->GetSoundFile();
	if (!pMainFrm)
		return;
	BeginWaitCursor();
	if (pMainFrm->OpenClipboard())
	{
		HGLOBAL hCpy = ::GetClipboardData(CF_TEXT);
		LPCSTR p;

		if ((hCpy) && ((p = (LPCSTR)GlobalLock(hCpy)) != NULL))
		{
			const DWORD dwMemSize = GlobalSize(hCpy);

			if (dwMemSize > sizeof(szClipboardOrdersHdr) &&
				memcmp(p, "OpenMPT ", 8) == 0 &&
				memcmp(p + 11, "\r\n", 2) == 0)
			{
				char buf[8];
				p += sizeof(szClipboardOrdersHdr) - 1;
				std::istrstream iStrm(p, dwMemSize - sizeof(szClipboardOrdersHdr) + 1);
				ORDERINDEX nCount = 0;
				std::vector<PATTERNINDEX> vecPat;
				while (iStrm.get(buf, sizeof(buf), '\n'))
				{
					if (memcmp(buf, "OrdNum:", 8) == 0) // Read expected order count.
						iStrm >> nCount;
					else if (memcmp(buf, "OrdLst:", 8) != 0)
					{	// Unrecognized data -> skip line.
						iStrm.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
						continue;
					}
					else // Read orders.
					{
						LimitMax(nCount, pSf->GetModSpecifications().ordersMax);
						vecPat.reserve(nCount);
						char bufItem[16];
						while (iStrm.peek() >= 32 && iStrm.getline(bufItem, sizeof(bufItem), ' '))
						{
							if (vecPat.size() >= pSf->GetModSpecifications().ordersMax)
								break;
							if (!(isdigit(bufItem[0]) || bufItem[0] == '+' || bufItem[0] == '-'))
								continue;
							PATTERNINDEX nPat = pSf->Order.GetInvalidPatIndex();
							if (bufItem[0] == '+')
							{
								nPat = pSf->Order.GetIgnoreIndex();
								if(!pSf->GetModSpecifications().hasIgnoreIndex) continue;
							}
							else if (isdigit(bufItem[0]))
							{
								nPat = ConvertStrTo<PATTERNINDEX>(bufItem);
								if (nPat >= pSf->GetModSpecifications().patternsMax)
									nPat = pSf->Order.GetInvalidPatIndex();
							}
							vecPat.push_back(nPat);
						}
						nCount = pSf->Order.Insert(m_nScrollPos, (ORDERINDEX)vecPat.size());
						for (ORDERINDEX nOrd = 0; nOrd < nCount; nOrd++)
							pSf->Order[m_nScrollPos + nOrd] = vecPat[nOrd];
					}
					m_pModDoc->SetModified();
					m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, NULL);
				}
			}
			GlobalUnlock(hCpy);
		}
		CloseClipboard();
	}
	EndWaitCursor();
}


void COrderList::OnEditCopy()
//---------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if ((!pMainFrm)) return;
	
	const OrdSelection ordsel = GetCurSel(false);

	DWORD dwMemSize;
	HGLOBAL hCpy;
	
	BeginWaitCursor();
	dwMemSize = sizeof(szClipboardOrdersHdr) + sizeof(szClipboardOrdersFieldHdr) + sizeof(szClipboardOrdCountFieldHdr);
	dwMemSize += ordsel.GetSelCount() * 6 + 8;
	if ((pMainFrm->OpenClipboard()) && ((hCpy = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwMemSize))!=NULL))
	{
		LPCSTR pszFormatName;
		EmptyClipboard();
		switch(m_pModDoc->GetSoundFile()->GetType())
		{
			case MOD_TYPE_S3M:	pszFormatName = "S3M"; break;
			case MOD_TYPE_XM:	pszFormatName = "XM"; break;
			case MOD_TYPE_IT:	pszFormatName = "IT"; break;
			case MOD_TYPE_MPT:	pszFormatName = "MPT"; break;
			default:			pszFormatName = "MOD"; break;
		}
		LPSTR p = (LPSTR)GlobalLock(hCpy);
		if (p)
		{
			const ModSequence& seq = m_pModDoc->GetSoundFile()->Order;
			wsprintf(p, szClipboardOrdersHdr, pszFormatName);
			p += strlen(p);
			wsprintf(p, szClipboardOrdCountFieldHdr, ordsel.GetSelCount());
			strcat(p, szClipboardOrdersFieldHdr);
			p += strlen(p);
			for(ORDERINDEX i = ordsel.firstOrd; i <= ordsel.lastOrd; i++)
			{
				std::string str;
				if (seq[i] == seq.GetInvalidPatIndex()) 
					str = "-";
				else if (seq[i] == seq.GetIgnoreIndex())
					str = "+";
				else
					str = Stringify(seq[i]);
				memcpy(p, str.c_str(), str.size());
				p += str.size();
				*p++ = ' ';
			}
			*p++ = '\r';
			*p++ = '\n';
			*p = 0;
		}
		GlobalUnlock(hCpy);
		SetClipboardData(CF_TEXT, (HANDLE) hCpy);
		CloseClipboard();
	}
	EndWaitCursor();
}


void COrderList::UpdateView(DWORD dwHintMask, CObject *pObj)
//----------------------------------------------------------
{
	if ((pObj != this) && (dwHintMask & HINT_MODSEQUENCE))
	{
		InvalidateRect(NULL, FALSE);
		UpdateInfoText();
	}
}


void COrderList::OnSwitchToView()
//-------------------------------
{
	if (m_pParent) m_pParent->PostViewMessage(VIEWMSG_SETFOCUS);
}


void COrderList::UpdateInfoText()
//-------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CSoundFile *pSndFile;
	if ((pMainFrm != nullptr) && (m_pModDoc != nullptr) && (pSndFile = m_pModDoc->GetSoundFile()) != nullptr && (::GetFocus() == m_hWnd))
	{
		CHAR s[128];
		strcpy(s, "");

		// MOD orderlist always ends after first empty pattern
		const ORDERINDEX nLength = (pSndFile->GetType() & MOD_TYPE_MOD) ? pSndFile->Order.GetLengthFirstEmpty() : pSndFile->Order.GetLengthTailTrimmed();

		if(CMainFrame::GetSettings().m_dwPatternSetup & PATTERN_HEXDISPLAY)
		{
			wsprintf(s, "Position %02Xh of %02Xh", m_nScrollPos, nLength);
		}
		else
		{
			wsprintf(s, "Position %d of %d (%02Xh of %02Xh)", m_nScrollPos, nLength, m_nScrollPos, nLength);
		}
		
		if (m_nScrollPos < pSndFile->Order.GetLength())
		{
			PATTERNINDEX nPat = pSndFile->Order[m_nScrollPos];
			if (nPat < pSndFile->Patterns.Size())
			{
				char szpat[MAX_PATTERNNAME] = "";
				if (pSndFile->Patterns[nPat].GetName(szpat) && strcmp(szpat, ""))
				{
					wsprintf(s + strlen(s), ": %s", szpat);
				}
			}
		}
		pMainFrm->SetInfoText(s);
	}
}


/////////////////////////////////////////////////////////////////
// COrderList messages

void COrderList::OnPaint()
//------------------------
{
	CHAR s[64];
	CPaintDC dc(this);
	HGDIOBJ oldfont = ::SelectObject(dc.m_hDC, m_hFont);
	HGDIOBJ oldpen = ::SelectObject(dc.m_hDC, CMainFrame::penSeparator);
	// First time ?
	if ((m_cxFont <= 0) || (m_cyFont <= 0))
	{
		CSize sz = dc.GetTextExtent("000+", 4);
		m_cxFont = sz.cx;
		m_cyFont = sz.cy;
	}

	if ((m_cxFont > 0) && (m_cyFont > 0) && (m_pModDoc))
	{
		CRect rcClient, rect;

		UpdateScrollInfo();
		BOOL bFocus = (::GetFocus() == m_hWnd);
		dc.SetBkMode(TRANSPARENT);
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		GetClientRect(&rcClient);
		rect = rcClient;
		ORDERINDEX nIndex = m_nXScroll;
		OrdSelection selection = GetCurSel(false);

		ORDERINDEX maxEntries = pSndFile->GetModSpecifications().ordersMax;
		if(pSndFile->Order.GetLength() > maxEntries)
		{
			// Only computed if potentially needed.
			maxEntries = Util::Max(maxEntries, pSndFile->Order.GetLengthTailTrimmed());
		}

		// Scrolling the shown orders(the showns rectangles)?
		while (rect.left < rcClient.right)
		{
			dc.SetTextColor(colorText);
			bool bHighLight = ((bFocus) && (nIndex >= selection.firstOrd && nIndex <= selection.lastOrd));
			const PATTERNINDEX nPat = (nIndex < pSndFile->Order.GetLength()) ? pSndFile->Order[nIndex] : PATTERNINDEX_INVALID;
			if ((rect.right = rect.left + m_cxFont) > rcClient.right) rect.right = rcClient.right;
			rect.right--;

			if(bHighLight)
			{
				// Currently selected order item
				FillRect(dc.m_hDC, &rect, CMainFrame::brushHighLight);
			} else if(pSndFile->Order.IsPositionLocked(nIndex))
			{
				// "Playback lock" indicator - grey out all order items which aren't played.
				FillRect(dc.m_hDC, &rect, CMainFrame::brushGray);
			} else
			{
				// Normal, unselected item.
				FillRect(dc.m_hDC, &rect, CMainFrame::brushWindow);
			}

			// Drawing the shown pattern-indicator or drag position.
			if (nIndex == ((m_bDragging) ? m_nDropPos : m_nScrollPos))
			{
				rect.InflateRect(-1, -1);
				dc.DrawFocusRect(&rect);
				rect.InflateRect(1, 1);
			}
			MoveToEx(dc.m_hDC, rect.right, rect.top, NULL);
			LineTo(dc.m_hDC, rect.right, rect.bottom);

			// Drawing the 'ctrl-transition' indicator
			if(nIndex == pSndFile->m_nSeqOverride)
			{
				MoveToEx(dc.m_hDC, rect.left + 4, rect.bottom - 4, NULL);
				LineTo(dc.m_hDC, rect.right - 4, rect.bottom - 4);
			} 

			// Drawing 'playing'-indicator.
			if(nIndex == pSndFile->GetCurrentOrder() && CMainFrame::GetMainFrame()->IsPlaying())
			{
				MoveToEx(dc.m_hDC, rect.left + 4, rect.top + 2, NULL);
				LineTo(dc.m_hDC, rect.right - 4, rect.top + 2);
			}

			s[0] = '\0';
			if(nIndex < maxEntries && (rect.left + m_cxFont - 4) <= rcClient.right)
			{
				if (nPat == pSndFile->Order.GetInvalidPatIndex()) strcpy(s, "---");
				else if (nPat == pSndFile->Order.GetIgnoreIndex()) strcpy(s, "+++");
				else if (nPat < pSndFile->Patterns.Size()) wsprintf(s, "%u", nPat);
				else strcpy(s, "???");
			}

			const COLORREF &textCol =
				(bHighLight
					? colorTextSel			// Highlighted pattern
					: (pSndFile->Patterns.IsValidPat(nPat)
						? colorText			// Normal pattern
						: colorInvalid));	// Non-existent pattern
			dc.SetTextColor(textCol);
			dc.DrawText(s, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			rect.left += m_cxFont;
			nIndex++;
		}
	}
	if (oldpen) ::SelectObject(dc.m_hDC, oldpen);
	if (oldfont) ::SelectObject(dc.m_hDC, oldfont);
}


void COrderList::OnSetFocus(CWnd *pWnd)
//-------------------------------------
{
	CWnd::OnSetFocus(pWnd);
	InvalidateSelection();
	UpdateInfoText();
	CMainFrame::GetMainFrame()->m_pOrderlistHasFocus = this;
}


void COrderList::OnKillFocus(CWnd *pWnd)
//--------------------------------------
{
	CWnd::OnKillFocus(pWnd);
	InvalidateSelection();
	CMainFrame::GetMainFrame()->m_pOrderlistHasFocus = nullptr;
}


void COrderList::OnLButtonDown(UINT nFlags, CPoint pt)
//----------------------------------------------------
{
	CRect rect;
	GetClientRect(&rect);
	if (pt.y < rect.bottom)
	{
		SetFocus();

		if(IsCtrlKeyPressed())
		{
			// Queue pattern
			QueuePattern(pt);
		} else
		{
			// mark pattern (+skip to)
			const int oldXScroll = m_nXScroll;

			ORDERINDEX nOrder = GetOrderFromPoint(rect, pt);
			OrdSelection selection = GetCurSel(false);

			// check if cursor is in selection - if it is, only react on MouseUp as the user might want to drag those orders
			if(m_nScrollPos2nd == ORDERINDEX_INVALID || nOrder < selection.firstOrd || nOrder > selection.lastOrd)
			{
				m_nScrollPos2nd = ORDERINDEX_INVALID;
				SetCurSel(nOrder, true, IsSelectionKeyPressed());
			}
			m_bDragging = !IsOrderInMargins(m_nScrollPos, oldXScroll);

			if(m_bDragging == true)
			{
				m_nDragOrder = GetCurSel(true).firstOrd;
				m_nDropPos = m_nDragOrder;
				SetCapture();
			}
		}
	} else
	{
		CWnd::OnLButtonDown(nFlags, pt);
	}
}


void COrderList::OnLButtonUp(UINT nFlags, CPoint pt)
//--------------------------------------------------
{
	CRect rect;
	GetClientRect(&rect);

	// Copy or move orders?
	const bool copyOrders = IsSelectionKeyPressed();

	if (m_bDragging)
	{
		m_bDragging = false;
		ReleaseCapture();
		if (rect.PtInRect(pt))
		{
			ORDERINDEX n = GetOrderFromPoint(rect, pt);
			if ((n != ORDERINDEX_INVALID) && (n == m_nDropPos) && (m_pModDoc))
			{
				// drag multiple orders (not quite as easy...)
				OrdSelection selection = GetCurSel(false);
				// move how many orders from where?
				ORDERINDEX moveCount = (selection.lastOrd - selection.firstOrd), nMovePos = selection.firstOrd;
				// drop before or after the selection
				bool moveBack = !(m_nDragOrder < m_nDropPos);
				// don't do anything if drop position is inside the selection
				if((m_nDropPos >= selection.firstOrd && m_nDropPos <= selection.lastOrd) || m_nDragOrder == m_nDropPos) return;
				// drag one order or multiple orders?
				bool multiSelection = (selection.firstOrd != selection.lastOrd);

				for(int i = 0; i <= moveCount; i++)
				{
					if(!m_pModDoc->MoveOrder(nMovePos, m_nDropPos, true, copyOrders)) return;
					if((moveBack ^ copyOrders) == true && multiSelection)
					{
						nMovePos++;
						m_nDropPos++;
					}
					if(moveBack && copyOrders && multiSelection)
					{
						nMovePos += 2;
						m_nDropPos++;
					}
				}

				if(multiSelection)
				{
					// adjust selection
					m_nScrollPos2nd = m_nDropPos - 1;
					m_nDropPos -= moveCount + (moveBack ? 0 : 1);
					SetCurSel((moveBack && !copyOrders) ? m_nDropPos - 1 : m_nDropPos);
				} else
				{
					SetCurSel((m_nDragOrder < m_nDropPos && !copyOrders) ? m_nDropPos - 1 : m_nDropPos);
				}
				m_pModDoc->SetModified();
			} else
			{
				ORDERINDEX nOrder = GetOrderFromPoint(rect, pt);
				OrdSelection selection = GetCurSel(false);

				// this should actually have equal signs but that breaks multiselect: nOrder >= selection.nOrdLo && nOrder <= section.nOrdHi
				if(pt.y < rect.bottom && m_nScrollPos2nd != ORDERINDEX_INVALID && nOrder > selection.firstOrd && nOrder < selection.lastOrd)
				{
					// Remove selection if we didn't drag anything but multiselect was active
					m_nScrollPos2nd = ORDERINDEX_INVALID;
					SetFocus();
					SetCurSel(GetOrderFromPoint(rect, pt));
				}
			}
		}
		InvalidateRect(NULL, FALSE);
	} else
	{
		CWnd::OnLButtonUp(nFlags, pt);
	}
}


void COrderList::OnMouseMove(UINT nFlags, CPoint pt)
//--------------------------------------------------
{
	if ((m_bDragging) && (m_cxFont))
	{
		CRect rect;

		GetClientRect(&rect);
		ORDERINDEX n = ORDERINDEX_INVALID;
		if (rect.PtInRect(pt))
		{
			CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
			n = GetOrderFromPoint(rect, pt);
			if (n >= pSndFile->Order.GetLength() || n >= pSndFile->GetModSpecifications().ordersMax) n = ORDERINDEX_INVALID;
		}
		if (n != m_nDropPos)
		{
			if (n != ORDERINDEX_INVALID)
			{
				m_nDropPos = n;
				InvalidateRect(NULL, FALSE);
				SetCursor(CMainFrame::curDragging);
			} else
			{
				m_nDropPos = ORDERINDEX_INVALID;
				SetCursor(CMainFrame::curNoDrop);
			}
		}
	} else
	{
		CWnd::OnMouseMove(nFlags, pt);
	}
}


void COrderList::OnSelectSequence(UINT nid)
//-----------------------------------------
{
	SelectSequence(static_cast<SEQUENCEINDEX>(nid - ID_SEQUENCE_ITEM));
}


void COrderList::OnRButtonDown(UINT nFlags, CPoint pt)
//----------------------------------------------------
{
	CRect rect;
	GetClientRect(&rect);
	if (m_bDragging)
	{
		m_nDropPos = ORDERINDEX_INVALID;
		OnLButtonUp(nFlags, pt);
	}
	if (pt.y >= rect.bottom) return;

	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();

	bool multiSelection = (m_nScrollPos2nd != ORDERINDEX_INVALID);

	if(!multiSelection) SetCurSel(GetOrderFromPoint(rect, pt));
	SetFocus();
	HMENU hMenu = ::CreatePopupMenu();
	if(!hMenu) return;
	
	// check if at least one pattern in the current selection exists
	bool bPatternExists = false;
	OrdSelection selection = GetCurSel(false);
	for(ORDERINDEX nOrd = selection.firstOrd; nOrd <= selection.lastOrd; nOrd++)
	{
		bPatternExists = ((pSndFile->Order[nOrd] < pSndFile->Patterns.Size())
			&& (pSndFile->Patterns[pSndFile->Order[nOrd]] != nullptr));
		if(bPatternExists) break;
	}

	const DWORD greyed = bPatternExists ? 0 : MF_GRAYED;

	CInputHandler* ih = (CMainFrame::GetMainFrame())->GetInputHandler();

	if(multiSelection)
	{
		// several patterns are selected.
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_INSERT, "&Insert Patterns\t" + ih->GetKeyTextFromCommand(kcOrderlistEditInsert));
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_DELETE, "&Remove Patterns\t" + ih->GetKeyTextFromCommand(kcOrderlistEditDelete));
		AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_EDIT_COPY, "&Copy Orders\t" + ih->GetKeyTextFromCommand(kcEditCopy));
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_EDIT_CUT, "&C&ut Orders\t" + ih->GetKeyTextFromCommand(kcEditCut));
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_EDIT_PASTE, "&Paste Orders\t" + ih->GetKeyTextFromCommand(kcEditPaste));
		AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
		AppendMenu(hMenu, MF_STRING | greyed, ID_ORDERLIST_COPY, "&Duplicate Patterns\t" + ih->GetKeyTextFromCommand(kcDuplicatePattern));
	}
	else
	{
		// only one pattern is selected
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_INSERT, "&Insert Pattern\t" + ih->GetKeyTextFromCommand(kcOrderlistEditInsert));
		if(pSndFile->GetModSpecifications().hasIgnoreIndex)
		{
			AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_INSERT_SEPARATOR, "&Insert Separator\t" + ih->GetKeyTextFromCommand(kcOrderlistPatIgnore));
		}
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_DELETE, "&Remove Pattern\t" + ih->GetKeyTextFromCommand(kcOrderlistEditDelete));
		AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_NEW, "Create &New Pattern\t" + ih->GetKeyTextFromCommand(kcNewPattern));
		AppendMenu(hMenu, MF_STRING | greyed, ID_ORDERLIST_COPY, "&Duplicate Pattern\t" + ih->GetKeyTextFromCommand(kcDuplicatePattern));
		AppendMenu(hMenu, MF_STRING | greyed, ID_PATTERNCOPY, "&Copy Pattern");
		AppendMenu(hMenu, MF_STRING | greyed, ID_PATTERNPASTE, "P&aste Pattern");
		AppendMenu(hMenu, MF_STRING, ID_ORDERLIST_EDIT_PASTE, "&Paste Orders\t" + ih->GetKeyTextFromCommand(kcEditPaste));
		if (pSndFile->TypeIsIT_MPT_XM())
		{
			AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
			AppendMenu(hMenu, MF_STRING | greyed, ID_PATTERN_PROPERTIES, "&Pattern properties...");
		}
		if (pSndFile->GetType() == MOD_TYPE_MPT)
		{
			AppendMenu(hMenu, MF_SEPARATOR, NULL, "");

			HMENU menuSequence = ::CreatePopupMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)menuSequence, TEXT("Sequences"));
			
			const SEQUENCEINDEX numSequences = pSndFile->Order.GetNumSequences();
			for(SEQUENCEINDEX i = 0; i < numSequences; i++)
			{
				CString str;
				if(pSndFile->Order.GetSequence(i).m_sName.IsEmpty())
					str.Format(TEXT("Sequence %u"), i);
				else
					str.Format(TEXT("%u: %s"), i, (LPCTSTR)pSndFile->Order.GetSequence(i).m_sName);
				const UINT flags = (pSndFile->Order.GetCurrentSequenceIndex() == i) ? MF_STRING|MF_CHECKED : MF_STRING;
				AppendMenu(menuSequence, flags, ID_SEQUENCE_ITEM + i, str);
			}
			if (pSndFile->Order.GetNumSequences() < MAX_SEQUENCES)
			{
				AppendMenu(menuSequence, MF_STRING, ID_SEQUENCE_ITEM + MAX_SEQUENCES, TEXT("&Duplicate current sequence"));
				AppendMenu(menuSequence, MF_STRING, ID_SEQUENCE_ITEM + MAX_SEQUENCES + 1, TEXT("&Create empty sequence"));
			}
			if (pSndFile->Order.GetNumSequences() > 1)
				AppendMenu(menuSequence, MF_STRING, ID_SEQUENCE_ITEM + MAX_SEQUENCES + 2, TEXT("D&elete current sequence"));
		}
	}
	AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
	AppendMenu(hMenu, ((selection.firstOrd == pSndFile->m_lockOrderStart && selection.lastOrd == pSndFile->m_lockOrderEnd) ? (MF_STRING | MF_CHECKED) : MF_STRING), ID_ORDERLIST_LOCKPLAYBACK, "&Lock Playback to Selection\t" + ih->GetKeyTextFromCommand(kcOrderlistLockPlayback));
	AppendMenu(hMenu, (pSndFile->m_lockOrderStart == ORDERINDEX_INVALID ? (MF_STRING | MF_GRAYED) : MF_STRING), ID_ORDERLIST_UNLOCKPLAYBACK, "&Unlock Playback\t" + ih->GetKeyTextFromCommand(kcOrderlistUnlockPlayback));

	AppendMenu(hMenu, MF_SEPARATOR, NULL, "");
	AppendMenu(hMenu, MF_STRING | greyed, ID_ORDERLIST_RENDER, "Render to &Wave");

	ClientToScreen(&pt);
	::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
	::DestroyMenu(hMenu);
}


void COrderList::OnLButtonDblClk(UINT, CPoint)
//--------------------------------------------
{
	if ((m_pModDoc) && (m_pParent))
	{
		m_nScrollPos2nd = ORDERINDEX_INVALID;
		SetFocus();
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
		m_pParent->SetCurrentPattern(pSndFile->Order[m_nScrollPos]);
	}
}


void COrderList::OnMButtonDown(UINT nFlags, CPoint pt)
//----------------------------------------------------
{
	UNREFERENCED_PARAMETER(nFlags);
	QueuePattern(pt);
}


void COrderList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar*)
//--------------------------------------------------------------
{
	UINT nNewPos = m_nXScroll;
	UINT smin, smax;
	
	GetScrollRange(SB_HORZ, (LPINT)&smin, (LPINT)&smax);
	m_bScrolling = true;
	switch(nSBCode)
	{
	case SB_LEFT:			nNewPos = 0; break;
	case SB_LINELEFT:		if (nNewPos) nNewPos--; break;
	case SB_LINERIGHT:		if (nNewPos < smax) nNewPos++; break;
	case SB_PAGELEFT:		if (nNewPos > 4) nNewPos -= 4; else nNewPos = 0; break;
	case SB_PAGERIGHT:		if (nNewPos+4 < smax) nNewPos += 4; else nNewPos = smax; break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:	nNewPos = nPos; if (nNewPos & 0xFFFF8000) nNewPos = smin; break;
	case SB_ENDSCROLL:		m_bScrolling = false; break;
	}
	if (nNewPos > smax) nNewPos = smax;
	if (nNewPos != m_nXScroll)
	{
		m_nXScroll = static_cast<ORDERINDEX>(nNewPos);
		SetScrollPos(SB_HORZ, m_nXScroll);
		InvalidateRect(NULL, FALSE);
	}
}


void COrderList::OnSize(UINT nType, int cx, int cy)
//-------------------------------------------------
{
	int nPos;
	int smin, smax;

	CWnd::OnSize(nType, cx, cy);
	UpdateScrollInfo();
	GetScrollRange(SB_HORZ, &smin, &smax);
	nPos = GetScrollPos(SB_HORZ);
	if (nPos > smax) nPos = smax;
	if (m_nXScroll != nPos)
	{
		m_nXScroll = static_cast<ORDERINDEX>(nPos);
		SetScrollPos(SB_HORZ, m_nXScroll);
		InvalidateRect(NULL, FALSE);
	}
}


void COrderList::OnInsertOrder()
//------------------------------
{
	// insert the same order(s) after the currently selected order(s)
	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();

		const OrdSelection selection = GetCurSel(false);	
		const ORDERINDEX nInsertCount = selection.lastOrd - selection.firstOrd, nInsertEnd = selection.lastOrd;

		for(ORDERINDEX i = 0; i <= nInsertCount; i++)
		{
			// Checking whether there is some pattern at the end of orderlist.
			if (pSndFile->Order.GetLength() < 1 || pSndFile->Order.Last() < pSndFile->Patterns.Size())
			{
				if(pSndFile->Order.GetLength() < pSndFile->GetModSpecifications().ordersMax)
					pSndFile->Order.Append();
			}
			for(int j = pSndFile->Order.GetLastIndex(); j > nInsertEnd; j--)
				pSndFile->Order[j] = pSndFile->Order[j - 1];
		}
		// now that there is enough space in the order list, overwrite the orders
		for(ORDERINDEX i = 0; i <= nInsertCount; i++)
		{
			if(nInsertEnd + i + 1 < pSndFile->GetModSpecifications().ordersMax
			   && 
			   nInsertEnd + i + 1 < pSndFile->Order.GetLength())
				pSndFile->Order[nInsertEnd + i + 1] = pSndFile->Order[nInsertEnd - nInsertCount + i];
		}
		m_nScrollPos = min(nInsertEnd + 1, pSndFile->Order.GetLastIndex());
		if(nInsertCount > 0)
			m_nScrollPos2nd = min(m_nScrollPos + nInsertCount, pSndFile->Order.GetLastIndex());
		else
			m_nScrollPos2nd = ORDERINDEX_INVALID;

		InvalidateSelection();
		EnsureVisible(m_nScrollPos2nd);
		// first inserted order has higher priority than the last one
		EnsureVisible(m_nScrollPos);

		InvalidateRect(NULL, FALSE);
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);
	}
}


void COrderList::OnInsertSeparatorPattern()
//-----------------------------------------
{
	// Insert a separator pattern after the current pattern, don't move order list cursor
	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();

		const OrdSelection selection = GetCurSel(true);
		ORDERINDEX insertPos = selection.firstOrd;
		
		if(pSndFile->Order[selection.firstOrd] != pSndFile->Order.GetInvalidPatIndex())
		{
			// If we're not inserting on a stop (---) index, we move on by one position.
			insertPos++;
			// Checking whether there is some pattern at the end of orderlist.
			if (pSndFile->Order.GetLength() < 1 || pSndFile->Order.Last() < pSndFile->Patterns.Size())
			{
				if(pSndFile->Order.GetLength() < pSndFile->GetModSpecifications().ordersMax)
					pSndFile->Order.Append();
			}
			for(int j = pSndFile->Order.GetLastIndex(); j > selection.firstOrd; j--)
				pSndFile->Order[j] = pSndFile->Order[j - 1];

		}

		pSndFile->Order[insertPos] = pSndFile->Order.GetIgnoreIndex();

		InvalidateRect(NULL, FALSE);
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);
	}
}


void COrderList::OnRenderOrder()
//------------------------------
{
	OrdSelection selection = GetCurSel(false);
	m_pModDoc->OnFileWaveConvert(selection.firstOrd, selection.lastOrd);
}


void COrderList::OnDeleteOrder()
//------------------------------
{
	if (m_pModDoc)
	{
		CSoundFile *pSndFile = m_pModDoc->GetSoundFile();

		OrdSelection selection = GetCurSel(false);
		// remove selection
		m_nScrollPos2nd = ORDERINDEX_INVALID;

		pSndFile->Order.Remove(selection.firstOrd, selection.lastOrd);

		m_pModDoc->SetModified();
		InvalidateRect(NULL, FALSE);
		m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);

		SetCurSel(selection.firstOrd);
		PATTERNINDEX nNewPat = pSndFile->Order[selection.firstOrd];
		if ((nNewPat < pSndFile->Patterns.Size()) && (pSndFile->Patterns[nNewPat] != nullptr) && (m_pParent))
		{
			m_pParent->SetCurrentPattern(nNewPat);
		}
	}
}


void COrderList::OnPatternProperties()
//------------------------------------
{
	if (m_pParent) m_pParent->PostViewMessage(VIEWMSG_PATTERNPROPERTIES);
}


void COrderList::OnPlayerPlay()
//-----------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_PLAYER_PLAY);
}


void COrderList::OnPlayerPause()
//------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_PLAYER_PAUSE);
}


void COrderList::OnPlayerPlayFromStart()
//--------------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_PLAYER_PLAYFROMSTART);
}


void COrderList::OnPatternPlayFromStart()
//---------------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, IDC_PATTERN_PLAYFROMSTART);
}


void COrderList::OnCreateNewPattern()
//-----------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_ORDERLIST_NEW);
}


void COrderList::OnDuplicatePattern()
//-----------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_ORDERLIST_COPY);
}


void COrderList::OnPatternCopy()
//------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_PATTERNCOPY);
}


void COrderList::OnPatternPaste()
//-------------------------------
{
	if (m_pParent) m_pParent->PostMessage(WM_COMMAND, ID_PATTERNPASTE);
}


LRESULT COrderList::OnHelpHitTest(WPARAM, LPARAM)
//-----------------------------------------------
{
	return HID_BASE_COMMAND + IDC_ORDERLIST;
}


LRESULT COrderList::OnDragonDropping(WPARAM doDrop, LPARAM lParam)
//-----------------------------------------------------------------
{
	LPDRAGONDROP pDropInfo = (LPDRAGONDROP)lParam;
	ORDERINDEX posdest;
	BOOL canDrop;
	CSoundFile *pSndFile;
	CPoint pt;

	if ((!pDropInfo) || (!m_pModDoc) || (m_pModDoc != pDropInfo->pModDoc) || (!m_cxFont)) return FALSE;
	pSndFile = m_pModDoc->GetSoundFile();
	canDrop = FALSE;
	switch(pDropInfo->dwDropType)
	{
	case DRAGONDROP_ORDER:
		if (pDropInfo->dwDropItem >= pSndFile->Order.size()) break;
	case DRAGONDROP_PATTERN:
		canDrop = TRUE;
		break;
	}
	if(!canDrop || !doDrop) return canDrop;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	if (pt.x < 0) pt.x = 0;
	posdest = static_cast<ORDERINDEX>(m_nXScroll + (pt.x / m_cxFont));
	if (posdest >= pSndFile->Order.GetLength()) return FALSE;
	switch(pDropInfo->dwDropType)
	{
	case DRAGONDROP_PATTERN:
		pSndFile->Order[posdest] = static_cast<PATTERNINDEX>(pDropInfo->dwDropItem);
		break;

	case DRAGONDROP_ORDER:
		pSndFile->Order[posdest] = pSndFile->Order[pDropInfo->dwDropItem];
		break;
	}
	if (canDrop)
	{
		InvalidateRect(NULL, FALSE);
		m_pModDoc->SetModified();
		m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);
		SetCurSel(posdest, true);
	}
	return canDrop;
}


ORDERINDEX COrderList::SetMargins(int i)
//--------------------------------------
{
	m_nOrderlistMargins = i;
	return GetMargins();
}

void COrderList::SelectSequence(const SEQUENCEINDEX nSeq)
//-------------------------------------------------------
{
	CriticalSection cs;

	CMainFrame::GetMainFrame()->ResetNotificationBuffer();
	CSoundFile& rSf = *m_pModDoc->GetSoundFile();
	if(nSeq == MAX_SEQUENCES + 2)
	{
		CString strParam; strParam.Format(TEXT("%u: %s"), rSf.Order.GetCurrentSequenceIndex(), (LPCTSTR)rSf.Order.m_sName);
		CString str;
		AfxFormatString1(str, IDS_CONFIRM_SEQUENCE_DELETE, strParam);
		if (Reporting::Confirm(str) == cnfYes)
			rSf.Order.RemoveSequence();
		else
		{
			return;
		}
	}
	else if(nSeq == MAX_SEQUENCES || nSeq == MAX_SEQUENCES + 1)
		rSf.Order.AddSequence((nSeq == MAX_SEQUENCES));
	else if(nSeq < rSf.Order.GetNumSequences())
		rSf.Order.SetSequence(nSeq);
	ORDERINDEX nPosCandidate = rSf.Order.GetLengthTailTrimmed() - 1;
	SetCurSel(min(m_nScrollPos, nPosCandidate), true, false, true);
	if (m_pParent)
		m_pParent->SetCurrentPattern(rSf.Order[m_nScrollPos]);

	UpdateScrollInfo();
	// This won't make sense anymore in the new sequence.
	OnUnlockPlayback();

	cs.Leave();

	UpdateView(HINT_MODSEQUENCE);
	m_pModDoc->SetModified();
	m_pModDoc->UpdateAllViews(NULL, HINT_MODSEQUENCE, this);
}


void COrderList::QueuePattern(CPoint pt)
//--------------------------------------
{
	CRect rect;
	GetClientRect(&rect);

	if(!rect.PtInRect(pt)) return;
	if(m_pModDoc == nullptr) return;
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	if(pSndFile == nullptr) return;

	const PATTERNINDEX ignoreIndex = pSndFile->Order.GetIgnoreIndex();
	const PATTERNINDEX stopIndex = pSndFile->Order.GetInvalidPatIndex();
	const ORDERINDEX length = pSndFile->Order.GetLength();
	ORDERINDEX order = GetOrderFromPoint(rect, pt);

	// If this is not a playable order item, find the next valid item.
	while(order < length && (pSndFile->Order[order] == ignoreIndex || pSndFile->Order[order] == stopIndex))
	{
		order++;
	}

	if(order < length)
	{
		if(pSndFile->m_nSeqOverride == order)
		{
			// This item is already queued: Dequeue it.
			pSndFile->m_nSeqOverride = ORDERINDEX_INVALID;
		} else
		{
			if(pSndFile->Order.IsPositionLocked(order))
			{
				// Users wants to go somewhere else, so let him do that.
				OnUnlockPlayback();
			}

			pSndFile->m_nSeqOverride = order;
		}
		InvalidateRect(NULL, FALSE);
	}
}


void COrderList::OnLockPlayback()
//-------------------------------
{
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();

	OrdSelection selection = GetCurSel(false);
	if(selection.firstOrd == pSndFile->m_lockOrderStart && selection.lastOrd == pSndFile->m_lockOrderEnd)
	{
		OnUnlockPlayback();
	} else
	{
		pSndFile->m_lockOrderStart = selection.firstOrd;
		pSndFile->m_lockOrderEnd = selection.lastOrd;
		InvalidateRect(NULL, FALSE);
	}
}


void COrderList::OnUnlockPlayback()
//----------------------------------
{
	CSoundFile *pSndFile = m_pModDoc->GetSoundFile();
	pSndFile->m_lockOrderStart = pSndFile->m_lockOrderEnd = ORDERINDEX_INVALID;
	InvalidateRect(NULL, FALSE);
}
