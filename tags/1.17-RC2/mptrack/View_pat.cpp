#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "moddoc.h"
#include "dlg_misc.h"
#include "globals.h"
#include "view_pat.h"
#include "ctrl_pat.h"
#include "dlsbank.h"
#include "EffectVis.h"		//rewbs.fxvis
#include "PatternGotoDialog.h"		
#include "PatternRandomizer.h"
#include ".\arrayutils.h"
#include ".\view_pat.h"


#define MAX_SPACING		16
#define	PLUGNAME_HEIGHT		16	//rewbs.patPlugName

#pragma warning(disable:4244)

MODCOMMAND CViewPattern::m_cmdOld = { 0,0, 0,0, 0,0};
MODCOMMAND CViewPattern::m_cmdFind = { 0,0,0,0,0,0 };
MODCOMMAND CViewPattern::m_cmdReplace = { 0,0,0,0,0,0 };
DWORD CViewPattern::m_dwFindFlags = 0;
DWORD CViewPattern::m_dwReplaceFlags = 0;
UINT CViewPattern::m_nFindMinChn = 0;
UINT CViewPattern::m_nFindMaxChn = 0;

IMPLEMENT_SERIAL(CViewPattern, CModScrollView, 0)

BEGIN_MESSAGE_MAP(CViewPattern, CModScrollView)
	//{{AFX_MSG_MAP(CViewPattern)
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSKEYDOWN()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MOD_KEYCOMMAND,	OnCustomKeyMsg)		//rewbs.customKeys
	ON_MESSAGE(WM_MOD_MIDIMSG,		OnMidiMsg)
	ON_COMMAND(ID_EDIT_CUT,			OnEditCut)
	ON_COMMAND(ID_EDIT_COPY,		OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE,		OnEditPaste)
	ON_COMMAND(ID_EDIT_MIXPASTE,	OnEditMixPaste)		//rewbs.mixPaste
	ON_COMMAND(ID_EDIT_SELECT_ALL,	OnEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECTCOLUMN,OnEditSelectColumn)
	ON_COMMAND(ID_EDIT_SELECTCOLUMN2,OnSelectCurrentColumn)
	ON_COMMAND(ID_EDIT_FIND,		OnEditFind)
	ON_COMMAND(ID_EDIT_FIND,		OnEditGoto)
	ON_COMMAND(ID_EDIT_FINDNEXT,	OnEditFindNext)
	ON_COMMAND(ID_EDIT_RECSELECT,	OnRecordSelect)
// -> CODE#0012
// -> DESC="midi keyboard split"
	ON_COMMAND(ID_EDIT_SPLITRECSELECT,	OnSplitRecordSelect)
// -! NEW_FEATURE#0012
	ON_COMMAND(ID_EDIT_UNDO,		OnEditUndo)	
	ON_COMMAND(ID_PATTERN_MUTE,		OnMuteFromClick) //rewbs.customKeys
	ON_COMMAND(ID_PATTERN_SOLO,		OnSoloFromClick) //rewbs.customKeys
	ON_COMMAND(ID_PATTERN_TRANSITIONMUTE, OnTogglePendingMuteFromClick)
	ON_COMMAND(ID_PATTERN_UNMUTEALL,OnUnmuteAll)
	ON_COMMAND(ID_PATTERN_DELETEROW,OnDeleteRows)
	ON_COMMAND(ID_PATTERN_DELETEALLROW,OnDeleteRowsEx)
	ON_COMMAND(ID_PATTERN_INSERTROW,OnInsertRows)
	ON_COMMAND(ID_NEXTINSTRUMENT,	OnNextInstrument)
	ON_COMMAND(ID_PREVINSTRUMENT,	OnPrevInstrument)
	ON_COMMAND(ID_PATTERN_PLAYROW,	OnPatternStep)
	ON_COMMAND(ID_CONTROLENTER,		OnPatternStep)
	ON_COMMAND(ID_CONTROLTAB,		OnSwitchToOrderList)
	ON_COMMAND(ID_PREVORDER,		OnPrevOrder)
	ON_COMMAND(ID_NEXTORDER,		OnNextOrder)
	ON_COMMAND(IDC_PATTERN_RECORD,	OnPatternRecord)
	ON_COMMAND(ID_TRANSPOSE_UP,					OnTransposeUp)
	ON_COMMAND(ID_TRANSPOSE_DOWN,				OnTransposeDown)
	ON_COMMAND(ID_TRANSPOSE_OCTUP,				OnTransposeOctUp)
	ON_COMMAND(ID_TRANSPOSE_OCTDOWN,			OnTransposeOctDown)
	ON_COMMAND(ID_PATTERN_PROPERTIES,			OnPatternProperties)
	ON_COMMAND(ID_PATTERN_INTERPOLATE_VOLUME,	OnInterpolateVolume)
	ON_COMMAND(ID_PATTERN_INTERPOLATE_EFFECT,	OnInterpolateEffect)
	ON_COMMAND(ID_PATTERN_INTERPOLATE_NOTE,		OnInterpolateNote)
	ON_COMMAND(ID_PATTERN_VISUALIZE_EFFECT,		OnVisualizeEffect)		//rewbs.fxvis
	ON_COMMAND(ID_PATTERN_OPEN_RANDOMIZER,		OnOpenRandomizer)
	ON_COMMAND(ID_GROW_SELECTION,				OnGrowSelection)
	ON_COMMAND(ID_SHRINK_SELECTION,				OnShrinkSelection)
	ON_COMMAND(ID_PATTERN_SETINSTRUMENT,		OnSetSelInstrument)
	ON_COMMAND(ID_CURSORCOPY,					OnCursorCopy)
	ON_COMMAND(ID_CURSORPASTE,					OnCursorPaste)
	ON_COMMAND(ID_PATTERN_AMPLIFY,				OnPatternAmplify)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO,			OnUpdateUndo)
	ON_COMMAND_RANGE(ID_PLUGSELECT, ID_PLUGSELECT+MAX_MIXPLUGINS, OnSelectPlugin) //rewbs.patPlugName

	//}}AFX_MSG_MAP
	ON_WM_INITMENU()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


CViewPattern::CViewPattern()
//--------------------------
{
	m_pEffectVis = NULL; //rewbs.fxvis
	m_pRandomizer = NULL;
	m_bLastNoteEntryBlocked=false;

	m_nMenuOnChan = 0;
	m_nPattern = 0;
	m_nDetailLevel = 4;
	m_pEditWnd = NULL;
	m_pGotoWnd = NULL;
	m_Dib.Init(CMainFrame::bmpNotes);
	UpdateColors();
}

void CViewPattern::OnInitialUpdate()
//----------------------------------
{
	memset(ChnVUMeters, 0, sizeof(ChnVUMeters));
	memset(OldVUMeters, 0, sizeof(OldVUMeters));
	memset(MultiRecordMask, 0, sizeof(MultiRecordMask));
// -> CODE#0012
// -> DESC="midi keyboard split"
	memset(splitActiveNoteChannel, 0xFF, sizeof(splitActiveNoteChannel));
	memset(activeNoteChannel, 0xFF, sizeof(activeNoteChannel));
	oldrow = -1;
	oldchn = -1;
	oldsplitchn = -1;
	m_nPlayPat = 0xFFFF;
	m_nPlayRow = 0;
	m_nMidRow = 0;
	m_nDragItem = 0;
	m_bDragging = FALSE;
	m_bInItemRect = FALSE;
	m_bRecord = TRUE;
	m_bContinueSearch = FALSE;
	m_dwBeginSel = m_dwEndSel = m_dwCursor = m_dwStartSel = m_dwDragPos = 0;
	//m_dwStatus = 0;
	m_dwStatus = PATSTATUS_PLUGNAMESINHEADERS;
	m_nXScroll = m_nYScroll = 0;
	m_nPattern = m_nRow = 0;
	m_nSpacing = 0;
	m_nAccelChar = 0;
// -> CODE#0018
// -> DESC="route PC keyboard inputs to midi in mechanism"
	ignorekey = 0;
// -! BEHAVIOUR_CHANGE#0018
	CScrollView::OnInitialUpdate();
	UpdateSizes();
	UpdateScrollSize();
	SetCurrentPattern(0);
	m_nFoundInstrument=0;
	m_nLastPlayedRow=0;
}


BOOL CViewPattern::SetCurrentPattern(UINT npat, int nrow)
//-------------------------------------------------------
{
	CSoundFile *pSndFile;
	CModDoc *pModDoc = GetDocument();
	BOOL bUpdateScroll;
	
	if ((npat >= MAX_PATTERNS) || (!pModDoc)) return FALSE;
	if ((m_pEditWnd) && (m_pEditWnd->IsWindowVisible())) m_pEditWnd->ShowWindow(SW_HIDE);
	pSndFile = pModDoc->GetSoundFile();
	if ((npat < MAX_PATTERNS-1) && (!pSndFile->Patterns[npat])) npat = 0;
	while ((npat > 0) && (!pSndFile->Patterns[npat])) npat--;
	if (!pSndFile->Patterns[npat])
	{
		pSndFile->PatternSize[npat] = 64;
		pSndFile->Patterns[npat] = CSoundFile::AllocatePattern(64, pSndFile->m_nChannels);
	}
	bUpdateScroll = FALSE;
	m_nPattern = npat;
	if ((nrow >= 0) && (nrow != (int)m_nRow) && (nrow < (int)pSndFile->PatternSize[m_nPattern]))
	{
		m_nRow = nrow;
		bUpdateScroll = TRUE;
	}
	if (m_nRow >= pSndFile->PatternSize[m_nPattern]) m_nRow = 0;
	int sel = m_dwCursor | (m_nRow << 16);
	SetCurSel(sel, sel);
	UpdateSizes();
	UpdateScrollSize();
	UpdateIndicator();
	
	if (m_bWholePatternFitsOnScreen) 		//rewbs.scrollFix
		SetScrollPos(SB_VERT, 0);
	else if (bUpdateScroll) //rewbs.fix3147
		SetScrollPos(SB_VERT, m_nRow * GetColumnHeight());
	
	UpdateScrollPos();
	InvalidatePattern(TRUE);
	SendCtrlMessage(CTRLMSG_PATTERNCHANGED, m_nPattern);
	return TRUE;
}


BOOL CViewPattern::SetCurrentRow(UINT row, BOOL bWrap)
//----------------------------------------------------
{
	CSoundFile *pSndFile;
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc) return FALSE;
	pSndFile = pModDoc->GetSoundFile();
	CRect rect;
	UINT yofs = GetYScrollPos();
	
	if ((bWrap) && (pSndFile->PatternSize[m_nPattern]))
	{
		if ((int)row < (int)0)
		{
			if (m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_MOUSEDRAGSEL))
			{
				row = 0;
			} else
			if (CMainFrame::m_dwPatternSetup & PATTERN_CONTSCROLL)
			{
				UINT nCurOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER);
				if ((nCurOrder > 0) && (nCurOrder < MAX_ORDERS) && (m_nPattern == pSndFile->Order[nCurOrder]))
				{
					UINT nPrevPat = pSndFile->Order[nCurOrder-1];
					if ((nPrevPat < MAX_PATTERNS) && (pSndFile->PatternSize[nPrevPat]))
					{
						SendCtrlMessage(CTRLMSG_SETCURRENTORDER, nCurOrder-1);
						if (SetCurrentPattern(nPrevPat)) return SetCurrentRow(pSndFile->PatternSize[nPrevPat]-1);
					}
				}
				row = 0;
			} else
			if (CMainFrame::m_dwPatternSetup & PATTERN_WRAP)
			{
				if ((int)row < (int)0) row += pSndFile->PatternSize[m_nPattern];
				row %= pSndFile->PatternSize[m_nPattern];
			}
		} else
		if (row >= pSndFile->PatternSize[m_nPattern])
		{
			if (m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_MOUSEDRAGSEL))
			{
				row = pSndFile->PatternSize[m_nPattern]-1;
			} else
			if (CMainFrame::m_dwPatternSetup & PATTERN_CONTSCROLL)
			{
				UINT nCurOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER);
				if ((nCurOrder+1 < MAX_ORDERS) && (m_nPattern == pSndFile->Order[nCurOrder]))
				{
					UINT nNextPat = pSndFile->Order[nCurOrder+1];
					if ((nNextPat < MAX_PATTERNS) && (pSndFile->PatternSize[nNextPat]))
					{
						SendCtrlMessage(CTRLMSG_SETCURRENTORDER, nCurOrder+1);
						if (SetCurrentPattern(nNextPat)) return SetCurrentRow(0);
					}
				}
				row = pSndFile->PatternSize[m_nPattern]-1;
			} else
			if (CMainFrame::m_dwPatternSetup & PATTERN_WRAP)
			{
				row %= pSndFile->PatternSize[m_nPattern];
			}
		}
	}

	//rewbs.fix3168
	if ( (static_cast<int>(row)<0) && !(CMainFrame::m_dwPatternSetup & PATTERN_CONTSCROLL))
		row = 0;
	if (row >= pSndFile->PatternSize[m_nPattern] && !(CMainFrame::m_dwPatternSetup & PATTERN_CONTSCROLL))
		row = pSndFile->PatternSize[m_nPattern]-1;
	//end rewbs.fix3168

	if ((row >= pSndFile->PatternSize[m_nPattern]) || (!m_szCell.cy)) return FALSE;
	GetClientRect(&rect);
	rect.top += m_szHeader.cy;
	int numrows = (rect.bottom - rect.top - 1) / m_szCell.cy;
	if (numrows < 1) numrows = 1;
	if (m_nMidRow)
	{
		UINT newofs = row;
		InvalidateRow();
		if (newofs != yofs)
		{
			CSize sz;
			sz.cx = 0;
			sz.cy = (int)(newofs - yofs) * m_szCell.cy;
			OnScrollBy(sz, TRUE);
		}
		m_nRow = row;
		InvalidateRow();
	} else
	{
		InvalidateRow();
		if (row < yofs)
		{
			CSize sz;
			sz.cx = 0;
			sz.cy = (int)(row - yofs) * m_szCell.cy;
			OnScrollBy(sz, TRUE);
		} else
		if (row > yofs + (UINT)numrows - 1)
		{
			CSize sz;
			sz.cx = 0;
			sz.cy = (int)(row - yofs - numrows + 1) * m_szCell.cy;
			OnScrollBy(sz, TRUE);
		}
		m_nRow = row;
		InvalidateRow();
	}
	int sel = m_dwCursor | (m_nRow << 16);
	int sel0 = sel;
	if ((m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_MOUSEDRAGSEL))
	 && (!(m_dwStatus & PATSTATUS_DRAGNDROPEDIT))) sel0 = m_dwStartSel;
	SetCurSel(sel0, sel);
	UpdateIndicator();

	Log("Row: %d; Chan: %d; ColType: %d; cursor&0xFFFF: %x; cursor>>16: %x;\n", 
		GetRowFromCursor(sel0), 
		GetChanFromCursor(sel0),
		GetColTypeFromCursor(sel0),
		(int)(sel0&0xFFFF),
		(int)(sel0>>16));
	

	return TRUE;
}


BOOL CViewPattern::SetCurrentColumn(UINT ncol)
//--------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	if (!pModDoc) return FALSE;
	pSndFile = pModDoc->GetSoundFile();
	if ((ncol & 0x07) == 7)
	{
		ncol = (ncol & ~0x07) + m_nDetailLevel;
	} else
	if ((ncol & 0x07) > m_nDetailLevel)
	{
		ncol += (((ncol & 0x07) - m_nDetailLevel) << 3) - (m_nDetailLevel+1);
	}
	if ((ncol >> 3) >= pSndFile->m_nChannels) return FALSE;
	m_dwCursor = ncol;
	int sel = m_dwCursor | (m_nRow << 16);
	int sel0 = sel;
	if ((m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_MOUSEDRAGSEL))
	 && (!(m_dwStatus & PATSTATUS_DRAGNDROPEDIT))) sel0 = m_dwStartSel;
	SetCurSel(sel0, sel);
	int xofs = GetXScrollPos();
	int nchn = ncol >> 3;
	if (nchn < xofs)
	{
		CSize size(0,0);
		size.cx = nchn * m_szCell.cx - GetScrollPos(SB_HORZ);
		UpdateWindow();
		if (OnScrollBy(size, TRUE)) UpdateWindow();
	} else
	if (nchn > xofs)
	{
		CRect rect;
		GetClientRect(&rect);
		UINT nw = GetColumnWidth();
		int maxcol = (rect.right - m_szHeader.cx) / nw;
		if ((nchn >= (xofs+maxcol)) && (maxcol >= 0))
		{
			CSize size(0,0);
			size.cx = (nchn - maxcol + 1) * m_szCell.cx - GetScrollPos(SB_HORZ);
			UpdateWindow();
			if (OnScrollBy(size, TRUE)) UpdateWindow();
		}
	}
	UpdateIndicator();
	return TRUE;
}


DWORD CViewPattern::GetDragItem(CPoint point, LPRECT lpRect)
//----------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	CRect rcClient, rect, plugRect; 	//rewbs.patPlugNames
	UINT n, nmax;
	int xofs, yofs;

	if (!pModDoc) return 0;
	GetClientRect(&rcClient);
	xofs = GetXScrollPos();
	yofs = GetYScrollPos();
	rect.SetRect(m_szHeader.cx, 0, m_szHeader.cx + GetColumnWidth() - 2, m_szHeader.cy);
	plugRect.SetRect(m_szHeader.cx, m_szHeader.cy-PLUGNAME_HEIGHT, m_szHeader.cx + GetColumnWidth() - 2, m_szHeader.cy);	//rewbs.patPlugNames
	pSndFile = pModDoc->GetSoundFile();
	nmax = pSndFile->m_nChannels;
	// Checking channel headers
	//rewbs.patPlugNames
	if (m_dwStatus & PATSTATUS_PLUGNAMESINHEADERS)
	{
		n = xofs;
		for (UINT ihdr=0; n<nmax; ihdr++, n++)
		{
			if (plugRect.PtInRect(point))
			{
				if (lpRect) *lpRect = plugRect;
				return (DRAGITEM_PLUGNAME | n);
			}
			plugRect.OffsetRect(GetColumnWidth(), 0);
		}
	}
	//end rewbs.patPlugNames
	n = xofs;
	for (UINT ihdr=0; n<nmax; ihdr++, n++)
	{
		if (rect.PtInRect(point))
		{
			if (lpRect) *lpRect = rect;
			return (DRAGITEM_CHNHEADER | n);
		}
		rect.OffsetRect(GetColumnWidth(), 0);
	}
	if ((pSndFile->Patterns[m_nPattern]) && (pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)))
	{
		rect.SetRect(0, 0, m_szHeader.cx, m_szHeader.cy);
		if (rect.PtInRect(point))
		{
			if (lpRect) *lpRect = rect;
			return DRAGITEM_PATTERNHEADER;
		}
	}
	return 0;
}


BOOL CViewPattern::DragToSel(DWORD dwPos, BOOL bScroll, BOOL bNoMove)
//-------------------------------------------------------------------
{
	CRect rect;
	CSoundFile *pSndFile;
	CModDoc *pModDoc = GetDocument();
	int yofs = GetYScrollPos(), xofs = GetXScrollPos();
	int row, col;
	
	if ((!pModDoc) || (!m_szCell.cy)) return FALSE;
	GetClientRect(&rect);
	pSndFile = pModDoc->GetSoundFile();
	if (!bNoMove) SetCurSel(m_dwStartSel, dwPos);
	if (!bScroll) return TRUE;
	// Scroll to row
	row = dwPos >> 16;
	if (row < (int)pSndFile->PatternSize[m_nPattern])
	{
		row += m_nMidRow;
		rect.top += m_szHeader.cy;
		int numrows = (rect.bottom - rect.top - 1) / m_szCell.cy;
		if (numrows < 1) numrows = 1;
		if (row < yofs)
		{
			CSize sz;
			sz.cx = 0;
			sz.cy = (int)(row - yofs) * m_szCell.cy;
			OnScrollBy(sz, TRUE);
		} else
		if (row > yofs + numrows - 1)
		{
			CSize sz;
			sz.cx = 0;
			sz.cy = (int)(row - yofs - numrows + 1) * m_szCell.cy;
			OnScrollBy(sz, TRUE);
		}
	}
	// Scroll to column
	col = (dwPos & 0xFFFF) >> 3;
	if (col < (int)pSndFile->m_nChannels)
	{
		int maxcol = (rect.right - m_szHeader.cx) - 4;
		maxcol -= GetColumnOffset(dwPos&7);
		maxcol /= GetColumnWidth();
		if (col < xofs)
		{
			CSize size(-m_szCell.cx,0);
			if (!bNoMove) size.cx = (col - xofs) * (int)m_szCell.cx;
			OnScrollBy(size, TRUE);
		} else
		if ((col > xofs+maxcol) && (maxcol > 0))
		{
			CSize size(m_szCell.cx,0);
			if (!bNoMove) size.cx = (col - maxcol + 1) * (int)m_szCell.cx;
			OnScrollBy(size, TRUE);
		}
	}
	UpdateWindow();
	return TRUE;
}


BOOL CViewPattern::SetPlayCursor(UINT nPat, UINT nRow)
//----------------------------------------------------
{
	UINT nOldPat = m_nPlayPat, nOldRow = m_nPlayRow;
	if ((nPat == m_nPlayPat) && (nRow == m_nPlayRow)) return TRUE;
	m_nPlayPat = nPat;
	m_nPlayRow = nRow;
	if (nOldPat == m_nPattern) InvalidateRow(nOldRow);
	if (m_nPlayPat == m_nPattern) InvalidateRow(m_nPlayRow);
	return TRUE;
}


UINT CViewPattern::GetCurrentInstrument() const
//---------------------------------------------
{
	return SendCtrlMessage(CTRLMSG_GETCURRENTINSTRUMENT);
}

BOOL CViewPattern::ShowEditWindow()
//---------------------------------
{
	if (!m_pEditWnd)
	{
		m_pEditWnd = new CEditCommand;
		if (m_pEditWnd) m_pEditWnd->SetParent(this, GetDocument());
	}
	if (m_pEditWnd)
	{
		m_pEditWnd->ShowEditWindow(m_nPattern, (m_nRow << 16) | m_dwCursor);
		return TRUE;
	}
	return FALSE;
}


BOOL CViewPattern::PrepareUndo(DWORD dwBegin, DWORD dwEnd)
//--------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	int x,y,cx,cy;

	x = (dwBegin & 0xFFFF) >> 3;
	y = (dwBegin >> 16);
	cx = ((dwEnd & 0xFFFF) >> 3) - x + 1;
	cy = (dwEnd >> 16) - y + 1;
	if ((x < 0) || (y < 0) || (cx < 1) || (cy < 1)) return FALSE;
	if (pModDoc) return pModDoc->PrepareUndo(m_nPattern, x, y, cx, cy);
	return FALSE;
}


BOOL CViewPattern::PreTranslateMessage(MSG *pMsg)
//-----------------------------------------------
{
	if (pMsg)
	{
		//rewbs.customKeys
		//We handle keypresses before Windows has a chance to handle them (for alt etc..)
		if ((pMsg->message == WM_SYSKEYUP)   || (pMsg->message == WM_KEYUP) || 
			(pMsg->message == WM_SYSKEYDOWN) || (pMsg->message == WM_KEYDOWN))
		{
			CInputHandler* ih = (CMainFrame::GetMainFrame())->GetInputHandler();
			
			//Translate message manually
			UINT nChar = pMsg->wParam;
			UINT nRepCnt = LOWORD(pMsg->lParam);
			UINT nFlags = HIWORD(pMsg->lParam);
			KeyEventType kT = ih->GetKeyEventType(nFlags);
			InputTargetContext ctx = (InputTargetContext)(kCtxViewPatterns+1 + (m_dwCursor & 0x07));
			
			if (ih->KeyEvent(ctx, nChar, nRepCnt, nFlags, kT) != kcNull)
			{	
				return true; // Mapped to a command, no need to pass message on.
			}
		}
		//end rewbs.customKeys
		
	}
	
	return CModScrollView::PreTranslateMessage(pMsg);
}


////////////////////////////////////////////////////////////////////////
// CViewPattern message handlers

void CViewPattern::OnDestroy()
//----------------------------
{
	if (m_pEffectVis)	{
		m_pEffectVis->DoClose(); 
		delete m_pEffectVis;
		m_pEffectVis = NULL;
	}

	if (m_pEditWnd)	{
		m_pEditWnd->DestroyWindow();
		delete m_pEditWnd;
		m_pEditWnd = NULL;
	}

	if (m_pGotoWnd)	{
		m_pGotoWnd->DestroyWindow();
		delete m_pGotoWnd;
		m_pGotoWnd = NULL;
	}

	if (m_pRandomizer) {
		delete m_pRandomizer;
		m_pRandomizer=NULL;
	}

	CModScrollView::OnDestroy();
}


void CViewPattern::OnSetFocus(CWnd *pOldWnd)
//------------------------------------------
{
	CScrollView::OnSetFocus(pOldWnd);
	m_dwStatus |= PATSTATUS_FOCUS;
	InvalidateRow();
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		pModDoc->SetFollowWnd(m_hWnd, MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);
		UpdateIndicator();
	}
}


void CViewPattern::OnKillFocus(CWnd *pNewWnd)
//-------------------------------------------
{
	CScrollView::OnKillFocus(pNewWnd);
	
	//rewbs.customKeys
	//Unset all selection
	m_dwStatus &= ~PATSTATUS_KEYDRAGSEL;
	m_dwStatus &= ~PATSTATUS_CTRLDRAGSEL;
//	CMainFrame::GetMainFrame()->GetInputHandler()->SetModifierMask(0);
	//end rewbs.customKeys	

	m_dwStatus &= ~PATSTATUS_FOCUS;
	InvalidateRow();
}

//rewbs.customKeys
void CViewPattern::OnGrowSelection()
//-----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	MODCOMMAND *p = pSndFile->Patterns[m_nPattern];
	if (!p) return;
	BeginWaitCursor();
	
	DWORD startSel = ((m_dwBeginSel>>16)<(m_dwEndSel>>16))?m_dwBeginSel:m_dwEndSel;
	DWORD endSel   = ((m_dwBeginSel>>16)<(m_dwEndSel>>16))?m_dwEndSel:m_dwBeginSel;
	pModDoc->PrepareUndo(m_nPattern, 0, 0, pSndFile->m_nChannels, pSndFile->PatternSize[m_nPattern]);

	int finalDest = (startSel>>16)+((endSel>>16)-(startSel>>16))*2;
	for (int row=finalDest; row>(int)(startSel >> 16); row-=2)
	{
		int offset = row-(startSel>>16);
		for (UINT i=(startSel & 0xFFFF); i<=(endSel & 0xFFFF); i++) if ((i & 7) < 5)
		{
			UINT chn = i >> 3;
			if ((chn >= pSndFile->m_nChannels) || (row >= pSndFile->PatternSize[m_nPattern])) continue;
			MODCOMMAND *dest = &p[row * pSndFile->m_nChannels + chn];
			MODCOMMAND *src  = &p[(row-offset/2) * pSndFile->m_nChannels + chn];
			MODCOMMAND *blank= &p[(row-1) * pSndFile->m_nChannels + chn];
			//memcpy(dest/*+(i%5)*/, src/*+(i%5)*/, /*sizeof(MODCOMMAND) - (i-chn)*/ sizeof(BYTE));
			//Log("dst: %d; src: %d; blk: %d\n", row, (row-offset/2), (row-1));
			switch(i & 7)
			{
				case 0:	dest->note    = src->note;    blank->note=0;   break;
				case 1: dest->instr   = src->instr;   blank->instr=0;  break;
				case 2:	dest->vol     = src->vol;     blank->vol=0;   
					    dest->volcmd  = src->volcmd;  blank->volcmd=0; break;
				case 3:	dest->command = src->command; blank->command=0;break;
				case 4:	dest->param   = src->param;   blank->param=0;  break;
			}
		}
	}

	m_dwBeginSel = startSel;
	m_dwEndSel   = (min(finalDest,pSndFile->PatternSize[m_nPattern]-1)<<16) | (endSel&0xFFFF);

	InvalidatePattern(FALSE);
	pModDoc->SetModified();
	pModDoc->UpdateAllViews(this, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
	EndWaitCursor();
	SetFocus();
}


void CViewPattern::OnShrinkSelection()
//-----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	MODCOMMAND *p = pSndFile->Patterns[m_nPattern];
	if (!p) return;
	BeginWaitCursor();

	DWORD startSel = ((m_dwBeginSel>>16)<(m_dwEndSel>>16))?m_dwBeginSel:m_dwEndSel;
	DWORD endSel   = ((m_dwBeginSel>>16)<(m_dwEndSel>>16))?m_dwEndSel:m_dwBeginSel;
	pModDoc->PrepareUndo(m_nPattern, 0, 0, pSndFile->m_nChannels, pSndFile->PatternSize[m_nPattern]);
	
	int finalDest = (startSel>>16)+((endSel>>16)-(startSel>>16))/2;

	for (int row=(startSel>>16)+1; row<=finalDest; row++)
	{
		int offset = row-(startSel>>16);
		int srcRow = (startSel>>16)+(offset*2);

		for (UINT i=(startSel & 0xFFFF); i<=(endSel & 0xFFFF); i++) if ((i & 7) < 5)
		{
			UINT chn = i >> 3;
			if ((chn >= pSndFile->m_nChannels) || (srcRow >= pSndFile->PatternSize[m_nPattern])
											   || (row    >= pSndFile->PatternSize[m_nPattern])) continue;
			MODCOMMAND *dest = &p[row    * pSndFile->m_nChannels + chn];
			MODCOMMAND *src  = &p[srcRow * pSndFile->m_nChannels + chn];
			//memcpy(dest/*+(i%5)*/, src/*+(i%5)*/, /*sizeof(MODCOMMAND) - (i-chn)*/ sizeof(BYTE));
			Log("dst: %d; src: %d\n", row, srcRow);
			switch(i & 7)
			{
				case 0:	dest->note    = src->note;    break;
				case 1: dest->instr   = src->instr;   break;
				case 2:	dest->vol     = src->vol;      
					    dest->volcmd  = src->volcmd;  break;
				case 3:	dest->command = src->command; break;
				case 4:	dest->param   = src->param;   break;
			}
		}
	}
	for (int row=finalDest+1; row<=(endSel>>16); row++)
	{
		for (UINT i=(startSel & 0xFFFF); i<=(endSel & 0xFFFF); i++) if ((i & 7) < 5)
		{
			UINT chn = i >> 3;
			MODCOMMAND *blank= &p[row * pSndFile->m_nChannels + chn];
			memset(blank, 0, sizeof(MODCOMMAND));
		}
	}
	m_dwBeginSel = startSel;
	m_dwEndSel   = (min(finalDest,pSndFile->PatternSize[m_nPattern]-1)<<16) | (endSel& 0xFFFF);

	InvalidatePattern(FALSE);
	pModDoc->SetModified();
	pModDoc->UpdateAllViews(this, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
	EndWaitCursor();
	SetFocus();
}
//rewbs.customKeys


void CViewPattern::OnClearSelection(bool ITStyle) //rewbs.customKeys: was OnEditDelete
//-----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc || !(m_dwStatus&PATSTATUS_RECORD)) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	MODCOMMAND *p = pSndFile->Patterns[m_nPattern];
	if (!p) return;
	BeginWaitCursor();
	PrepareUndo(m_dwBeginSel, m_dwEndSel);
	DWORD tmp = m_dwEndSel;
	for (UINT row=(m_dwBeginSel >> 16); row<=(m_dwEndSel >> 16); row++)
	{
		for (UINT i=(m_dwBeginSel & 0xFFFF); i<=(m_dwEndSel & 0xFFFF); i++) if ((i & 7) < 5)
		{
			UINT chn = i >> 3;
			if ((chn >= pSndFile->m_nChannels) || (row >= pSndFile->PatternSize[m_nPattern])) continue;
			MODCOMMAND *m = &p[row * pSndFile->m_nChannels + chn];
			switch(i & 7)
			{
			// Clear note
			case 0:	m->note = 0; if (ITStyle) m->instr = 0; /* if (m_dwBeginSel == m_dwEndSel) { m->instr = 0; tmp = m_dwEndSel+1; }*/ break;
			// Clear instrument
			case 1: m->instr = 0; break;
			// Clear Volume Column
			case 2:	m->volcmd = m->vol = 0; break;
			// Clear Command
			case 3:	m->command = 0; break;
			// Clear Command Param
			case 4:	m->param = 0; break;
			}
		}
	}
	if ((tmp & 7) == 3) tmp++;
	InvalidateArea(m_dwBeginSel, tmp);
	pModDoc->SetModified();
	pModDoc->UpdateAllViews(this, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
	EndWaitCursor();
	SetFocus();
}


void CViewPattern::OnEditCut()
//----------------------------
{
	OnEditCopy();
	OnClearSelection(false);
}


void CViewPattern::OnEditCopy()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		pModDoc->CopyPattern(m_nPattern, m_dwBeginSel, m_dwEndSel);
		SetFocus();
	}
}


void CViewPattern::OnEditPaste()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	
	if (pModDoc && m_dwStatus&PATSTATUS_RECORD)
	{
		pModDoc->PastePattern(m_nPattern, m_dwBeginSel, false);
		InvalidatePattern(FALSE);
		SetFocus();
	}
}

//rewbs.mixPaste
void CViewPattern::OnEditMixPaste()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	
	if (pModDoc && m_dwStatus&PATSTATUS_RECORD)
	{
		pModDoc->PastePattern(m_nPattern, m_dwBeginSel, true);
		InvalidatePattern(FALSE);
		SetFocus();
	}
}

void CViewPattern::OnEditMixPasteITStyle()
//----------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	
	if (pModDoc && m_dwStatus&PATSTATUS_RECORD)
	{
		pModDoc->PastePattern(m_nPattern, m_dwBeginSel, true, true);
		InvalidatePattern(FALSE);
		SetFocus();
	}
}

//end rewbs.mixPaste



BOOL CViewPattern::CheckCustomKeys(UINT nChar, DWORD dwFlags)
//-----------------------------------------------------------
{
	DWORD dwKey = nChar | (CMainFrame::gnHotKeyMask << 16);
	UINT nId = CMainFrame::IsHotKey(dwKey);
	if (nId != NULL)
	{
		if (!(dwKey & 0xFFFF0000))
		{
			if ((CMainFrame::GetNoteFromKey(nChar, HIWORD(dwFlags)))
			 || ((nChar >= '0') && (nChar <= '9') && (m_dwCursor & 0x07))
			 || ((nChar >= 'A') && (nChar <= 'F') && ((m_dwCursor & 0x07) > 2)))
				return FALSE;			 
		}
		m_nMenuParam = (m_nRow << 16) | m_dwCursor;
		if (((nId == ID_CURSORPASTE) && (CMainFrame::m_dwPatternSetup & PATTERN_AUTOSPACEBAR))
		 || (!(dwFlags & 0x40000000))) PostMessage(WM_COMMAND, nId);
		return TRUE;
	}
	return FALSE;
}


void CViewPattern::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (/*(m_bDragging) ||*/ (!pModDoc)) return;
	SetFocus();
	m_nDragItem = GetDragItem(point, &m_rcDragItem);
	m_bDragging = TRUE;
	m_bInItemRect = TRUE;
	SetCapture();
	if ((point.x >= m_szHeader.cx) && (point.y <= m_szHeader.cy)) {
		if (nFlags & MK_CONTROL) {
			TogglePendingMute((GetPositionFromPoint(point)&0xFFFF)>>3);
		}
	}
	else if ((point.x >= m_szHeader.cx) && (point.y > m_szHeader.cy))
	{
		/*if (ih->SelectionPressed()) {
			m_dwEndSel = GetPositionFromPoint(point);
			SetCurSel(m_dwStartSel, m_dwEndSel);
			SetCurrentRow(m_dwEndSel >> 16);
			SetCurrentColumn(m_dwEndSel & 0xFFFF);
			DragToSel(m_dwEndSel, TRUE);
		} else */
		{
			m_dwStartSel = GetPositionFromPoint(point);
			if (((m_dwStartSel & 0xFFFF) >> 3) < pModDoc->GetNumChannels())
			{
				m_dwStatus |= PATSTATUS_MOUSEDRAGSEL;

				if (m_dwStatus & PATSTATUS_CTRLDRAGSEL)
				{
					SetCurSel(m_dwStartSel, m_dwStartSel);
				}
				if ((CMainFrame::m_dwPatternSetup & PATTERN_DRAGNDROPEDIT)
				&& ((m_dwBeginSel != m_dwEndSel) || (m_dwStatus & PATSTATUS_CTRLDRAGSEL))
				&& ((m_dwStartSel >> 16) >= (m_dwBeginSel >> 16))
				&& ((m_dwStartSel >> 16) <= (m_dwEndSel >> 16))
				&& ((m_dwStartSel & 0xFFFF) >= (m_dwBeginSel & 0xFFFF))
				&& ((m_dwStartSel & 0xFFFF) <= (m_dwEndSel & 0xFFFF)))
				{
					m_dwStatus |= PATSTATUS_DRAGNDROPEDIT;
				} else
				if (CMainFrame::m_dwPatternSetup & PATTERN_CENTERROW)
				{
					SetCurSel(m_dwStartSel, m_dwStartSel);
				} else
				{
					SetCurrentRow(m_dwStartSel >> 16);
					SetCurrentColumn(m_dwStartSel & 0xFFFF);
				}
			}
		}
	}
	if (m_nDragItem)
	{
		InvalidateRect(&m_rcDragItem, FALSE);
		UpdateWindow();
	}
}


void CViewPattern::OnLButtonDblClk(UINT uFlags, CPoint point)
//-----------------------------------------------------------
{
	DWORD dwPos = GetPositionFromPoint(point);
	if ((dwPos == ((m_nRow << 16) | m_dwCursor)) && (point.y >= m_szHeader.cy))
	{
		if (ShowEditWindow()) return;
	}
	OnLButtonDown(uFlags, point);
}


void CViewPattern::OnLButtonUp(UINT nFlags, CPoint point)
//-------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL bItemSelected = m_bInItemRect;
	if (/*(!m_bDragging) ||*/ (!pModDoc)) return;
	m_bDragging = FALSE;
	m_bInItemRect = FALSE;
	ReleaseCapture();
	m_dwStatus &= ~PATSTATUS_MOUSEDRAGSEL;
	// Drag & Drop Editing
	if (m_dwStatus & PATSTATUS_DRAGNDROPEDIT)
	{
		if (m_dwStatus & PATSTATUS_DRAGNDROPPING)
		{
			OnDrawDragSel();
			m_dwStatus &= ~PATSTATUS_DRAGNDROPPING;
			OnDropSelection();
		}
		DWORD dwPos = GetPositionFromPoint(point);
		if (dwPos == m_dwStartSel)
		{
			SetCurSel(dwPos, dwPos);
		}
		SetCursor(CMainFrame::curArrow);
		m_dwStatus &= ~PATSTATUS_DRAGNDROPEDIT;
	}
	if (((m_nDragItem & DRAGITEM_MASK) != DRAGITEM_CHNHEADER)
	 && ((m_nDragItem & DRAGITEM_MASK) != DRAGITEM_PATTERNHEADER))
	{
		if ((m_nMidRow) && (m_dwBeginSel == m_dwEndSel))
		{
			DWORD dwPos = m_dwBeginSel;
			SetCurrentRow(dwPos >> 16);
			SetCurrentColumn(dwPos & 0xFFFF);
			//UpdateIndicator();
		}
	}
	if ((!bItemSelected) || (!m_nDragItem)) return;
	InvalidateRect(&m_rcDragItem, FALSE);
	DWORD nItemNo = m_nDragItem & 0xFFFF;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	switch(m_nDragItem & DRAGITEM_MASK)
	{
	case DRAGITEM_CHNHEADER:
		if (nFlags & MK_SHIFT)	{
			if (nItemNo < MAX_CHANNELS)	{
				pModDoc->Record1Channel(nItemNo);
				InvalidateChannelsHeaders();
			}
		} 
		else if (!(nFlags&MK_CONTROL))  {
			pModDoc->MuteChannel(nItemNo, (pSndFile->ChnSettings[nItemNo].dwFlags & CHN_MUTE) ? FALSE : TRUE);
		}
		break;
	case DRAGITEM_PATTERNHEADER:
		OnPatternProperties();
		break;
	case DRAGITEM_PLUGNAME:			//rewbs.patPlugNames
		if (nItemNo < MAX_CHANNELS)	{
			TogglePluginEditor(nItemNo);
		}
		break;
	}
}


void CViewPattern::OnPatternProperties()
//--------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CPatternPropertiesDlg dlg(pModDoc, m_nPattern, this);
		if (dlg.DoModal())
		{
			UpdateScrollSize();
			InvalidatePattern(TRUE);
		}
	}
}


void CViewPattern::OnRButtonDown(UINT, CPoint pt)
//-----------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	CHAR s[256]; //rewbs.patPlugNames
	HMENU hMenu;

	// Too far left to get a ctx menu:
	if ((!pModDoc) || (pt.x < m_szHeader.cx)) { 
		return;
	}

	// Handle drag n drop
	if (m_dwStatus & PATSTATUS_DRAGNDROPEDIT)	{
		if (m_dwStatus & PATSTATUS_DRAGNDROPPING)
		{
			OnDrawDragSel();
			m_dwStatus &= ~PATSTATUS_DRAGNDROPPING;
		}
		m_dwStatus &= ~(PATSTATUS_DRAGNDROPEDIT|PATSTATUS_MOUSEDRAGSEL);
		if (m_bDragging)
		{
			m_bDragging = FALSE;
			m_bInItemRect = FALSE;
			ReleaseCapture();
		}
		SetCursor(CMainFrame::curArrow);
		return;
	}

	if ((hMenu = ::CreatePopupMenu()) == NULL) {
		return;
	}

	pSndFile = pModDoc->GetSoundFile();
	m_nMenuParam = GetPositionFromPoint(pt);
	
	// Right-click outside selection? Reposition cursor to the new location
	if (((m_nMenuParam >> 16) < (m_dwBeginSel >> 16))
	 || ((m_nMenuParam >> 16) > (m_dwEndSel >> 16))
	 || ((m_nMenuParam & 0xFFFF) < (m_dwBeginSel & 0xFFFF))
	 || ((m_nMenuParam & 0xFFFF) > (m_dwEndSel & 0xFFFF)))
	{
		if (pt.y > m_szHeader.cy) { //ensure we're not clicking header
			SetCurrentRow(m_nMenuParam >> 16);
			SetCurrentColumn(m_nMenuParam & 0xFFFF);
		}
	}
	UINT nChn = (m_nMenuParam & 0xFFFF) >> 3;
	if ((nChn < pSndFile->m_nChannels) && (pSndFile->Patterns[m_nPattern]))
	{
		CString MenuText;
		CInputHandler* ih = (CMainFrame::GetMainFrame())->GetInputHandler();
		BOOL bSep = FALSE;

		//------ Plugin Header Menu --------- :
		if ((m_dwStatus & PATSTATUS_PLUGNAMESINHEADERS) && 
			(pt.y > m_szHeader.cy-PLUGNAME_HEIGHT) && (pt.y <= m_szHeader.cy)) {
			BuildPluginCtxMenu(hMenu, nChn, pSndFile);
		}
		
		//------ Header Menu ---------- :
		else if (pt.y <= m_szHeader.cy){
			if (ih->ShiftPressed()) {
				//Don't bring up menu if shift is pressed, else we won't get button up msg.
			} else {
				if (BuildSoloMuteCtxMenu(hMenu, ih, nChn, pSndFile))
					AppendMenu(hMenu, MF_SEPARATOR, 0, "");
				BuildRecordCtxMenu(hMenu, nChn, pModDoc);
			}
		}
		
		//------ Standard Menu ---------- :
		else if ((pt.x >= m_szHeader.cx) && (pt.y > m_szHeader.cy))	{
/*			if (BuildSoloMuteCtxMenu(hMenu, ih, nChn, pSndFile))
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
*/			if (BuildSelectionCtxMenu(hMenu, ih))
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			if (BuildEditCtxMenu(hMenu, ih, pModDoc))
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			if (BuildNoteInterpolationCtxMenu(hMenu, ih, pSndFile) |	//Use bitwise ORs to avoid shortcuts
				BuildVolColInterpolationCtxMenu(hMenu, ih, pSndFile) | 
				BuildEffectInterpolationCtxMenu(hMenu, ih, pSndFile) )
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			if (BuildTransposeCtxMenu(hMenu, ih))
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			if (BuildVisFXCtxMenu(hMenu, ih)   | 	//Use bitwise ORs to avoid shortcuts
				BuildAmplifyCtxMenu(hMenu, ih) |
				BuildSetInstCtxMenu(hMenu, ih) )
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			if (BuildGrowShrinkCtxMenu(hMenu, ih))
				AppendMenu(hMenu, MF_SEPARATOR, 0, "");
			BuildRowInsDelCtxMenu(hMenu, ih);
		}

		ClientToScreen(&pt);
		::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	::DestroyMenu(hMenu);
}

void CViewPattern::OnRButtonUp(UINT nFlags, CPoint point)
//-------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	BOOL bItemSelected = m_bInItemRect;
	if (!pModDoc) return;

	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	m_nDragItem = GetDragItem(point, &m_rcDragItem);
	DWORD nItemNo = m_nDragItem & 0xFFFF;
	switch(m_nDragItem & DRAGITEM_MASK)	{
		case DRAGITEM_CHNHEADER:
			if (nFlags & MK_SHIFT) {
				if (nItemNo < MAX_CHANNELS) {
					pModDoc->Record2Channel(nItemNo);
					InvalidateChannelsHeaders();
				}
			}
		break;
	}

	CModScrollView::OnRButtonUp(nFlags, point);
}


void CViewPattern::OnMouseMove(UINT, CPoint point)
//------------------------------------------------
{
	if (!m_bDragging) return;
	if (m_nDragItem)
	{
		DWORD nItem = GetDragItem(point, NULL);
		BOOL b = (nItem == m_nDragItem) ? TRUE : FALSE;
		if (b != m_bInItemRect)
		{
			m_bInItemRect = b;
			InvalidateRect(&m_rcDragItem, FALSE);
			UpdateWindow();
		}
	}
	if (m_dwStatus & PATSTATUS_MOUSEDRAGSEL)
	{
		CModDoc *pModDoc = GetDocument();
		DWORD dwPos = GetPositionFromPoint(point);
		if ((pModDoc) && (m_nPattern < MAX_PATTERNS))
		{
			UINT row = dwPos >> 16;
			UINT max = pModDoc->GetSoundFile()->PatternSize[m_nPattern];
			if ((row) && (row >= max)) row = max-1;
			dwPos &= 0xFFFF;
			dwPos |= (row << 16);
		}
		// Drag & Drop editing
		if (m_dwStatus & PATSTATUS_DRAGNDROPEDIT)
		{
			if (!(m_dwStatus & PATSTATUS_DRAGNDROPPING)) SetCursor(CMainFrame::curDragging);
			if ((!(m_dwStatus & PATSTATUS_DRAGNDROPPING)) || ((m_dwDragPos & ~7) != (dwPos & ~7)))
			{
				if (m_dwStatus & PATSTATUS_DRAGNDROPPING) OnDrawDragSel();
				m_dwStatus &= ~PATSTATUS_DRAGNDROPPING;
				DragToSel(dwPos, TRUE, TRUE);
				m_dwDragPos = dwPos;
				m_dwStatus |= PATSTATUS_DRAGNDROPPING;
				OnDrawDragSel();
				pModDoc->SetModified();
			}
		} else
		// Default: selection
		if (CMainFrame::m_dwPatternSetup & PATTERN_CENTERROW)
		{
			DragToSel(dwPos, TRUE);
		} else
		{
			SetCurrentRow(dwPos >> 16);
			SetCurrentColumn(dwPos & 0xFFFF);
		}
	}
}

/*
void CViewPattern::OnChar(UINT nChar, UINT, UINT nFlags)
//------------------------------------------------------
{
	m_nAccelChar = 0;
// -> CODE#0018
// -> DESC="route PC keyboard inputs to midi in mechanism"
//	ProcessChar(nChar, nFlags);
	CModDoc *pModDoc = GetDocument();
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CSoundFile *pSndFile = pModDoc->GetSoundFile();

	UINT nNote = pMainFrm->GetNoteFromKey(nChar, nFlags);
	MODCOMMAND * oldcmd = pSndFile->Patterns[m_nPattern];

	BOOL special = (nNote >= 0x80) || (!nNote && nChar >= '0' && nChar <= '9' && oldcmd->note > 0 && oldcmd->note < 128);
	BOOL keymodifier = (GetKeyState(VK_SHIFT) & 0x8000) || (GetKeyState(VK_CONTROL) & 0x8000) || (nFlags & 0x2000);

	UINT nCursor = m_dwCursor & 0x07;
	if( nCursor != 0 && !(nCursor == 1 && (nChar > '9' || nChar == ' ')) ) special = TRUE;

	if(keymodifier || special){
		ProcessChar(nChar, nFlags);
	}
// -! BEHAVIOUR_CHANGE#0018

}
*/

void CViewPattern::OnEditSelectAll()
//----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		SetCurSel(0, ((pSndFile->PatternSize[m_nPattern]-1) << 16)
		 | ((pSndFile->m_nChannels - 1) << 3)
		 | (4));
	}
}


void CViewPattern::OnEditSelectColumn()
//-------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		SetCurSel(m_nMenuParam & 0xFFF8,
		 ((pSndFile->PatternSize[m_nPattern]-1) << 16)
		 | ((m_nMenuParam & 0xFFF8)+4));
	}
}


void CViewPattern::OnSelectCurrentColumn()
//----------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		DWORD dwBeginSel = m_dwCursor & 0xFFF8;
		DWORD dwEndSel = ((pSndFile->PatternSize[m_nPattern]-1) << 16) | ((m_dwCursor & 0xFFF8)+4);
		// If column is already selected, select the current pattern
		if ((dwBeginSel == m_dwBeginSel) && (dwEndSel == m_dwEndSel))
		{
			dwBeginSel = 0;
			dwEndSel &= 0xFFFF0000;
			dwEndSel |= ((pSndFile->m_nChannels-1) << 3) + 4;
		}
		SetCurSel(dwBeginSel, dwEndSel);
	}
}


void CViewPattern::OnMuteFromClick()
{
	OnMuteChannel(false);
}

void CViewPattern::OnMuteChannel(BOOL current)
//--------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		UINT nChn = current ? (m_dwCursor&0xFFFF)>>3 : (m_nMenuParam&0xFFFF)>>3;
		pModDoc->SoloChannel(nChn, FALSE); //rewbs.merge: recover old solo/mute behaviour
		pModDoc->MuteChannel(nChn, !pModDoc->IsChannelMuted(nChn));

		//If we just unmuted a channel, make sure none are still considered "solo".
		if (!pModDoc->IsChannelMuted(nChn)) {
			UINT nNumChn = pModDoc->GetNumChannels();
			for (UINT i=0; i<nNumChn; i++){
				pModDoc->SoloChannel(i, FALSE);
			}
		}

		InvalidateChannelsHeaders();
	}
}

void CViewPattern::OnSoloFromClick()
{
	OnSoloChannel(false);
}


void CViewPattern::OnSoloChannel(BOOL current)
//--------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		UINT nNumChn = pModDoc->GetNumChannels();
		UINT nChn = current ? (m_dwCursor&0xFFFF)>>3 : (m_nMenuParam&0xFFFF)>>3;
		
		if (nChn < nNumChn)		{
			if (pModDoc->IsChannelSolo(nChn)) {
				 //trying to solo a channel that is solo'ed -> unSolo and unMute all
				for (UINT i=0; i<nNumChn; i++){
					pModDoc->MuteChannel(i, FALSE);
					pModDoc->SoloChannel(i, FALSE);
				}
			} 
			else {
				// Soloing a channel when another channel is soloed mutes all except for new one
				pModDoc->SoloChannel(nChn, TRUE);
				for (UINT i=0; i<nNumChn; i++)	{
					pModDoc->MuteChannel(i, !(i == nChn)); //mute all chans except this one
					pModDoc->SoloChannel(i, (i == nChn));  //unsolo all chans except this one, solo this channel
				}
			}
			InvalidateChannelsHeaders();
		}
	}
}


void CViewPattern::OnRecordSelect()
//---------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		UINT nNumChn = pModDoc->GetNumChannels();
		UINT nChn = (m_nMenuParam & 0xFFFF) >> 3;
		if (nChn < nNumChn)
		{
//			MultiRecordMask[nChn>>3] ^= (1 << (nChn & 7));
// -> CODE#0012
// -> DESC="midi keyboard split"
			pModDoc->Record1Channel(nChn);
// -! NEW_FEATURE#0012
			InvalidateChannelsHeaders();
		}
	}
}

// -> CODE#0012
// -> DESC="midi keyboard split"
void CViewPattern::OnSplitRecordSelect()
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc){
		UINT nNumChn = pModDoc->GetNumChannels();
		UINT nChn = (m_nMenuParam & 0xFFFF) >> 3;
		if (nChn < nNumChn){
			pModDoc->Record2Channel(nChn);
			InvalidateChannelsHeaders();
		}
	}
}
// -! NEW_FEATURE#0012


void CViewPattern::OnUnmuteAll()
//------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		UINT nChns = pModDoc->GetNumChannels();
		for (UINT i=0; i<nChns; i++)
		{
			pModDoc->MuteChannel(i, FALSE);
			pModDoc->SoloChannel(i, FALSE); //rewbs.merge: binary solo/mute behaviour 
		}
		InvalidateChannelsHeaders();
	}
}


void CViewPattern::DeleteRows(UINT colmin, UINT colmax, UINT nrows)
//-----------------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	UINT row, maxrow;

	if (!pModDoc || !(m_dwStatus&PATSTATUS_RECORD)) return;
	pSndFile = pModDoc->GetSoundFile();
	if (!pSndFile->Patterns[m_nPattern]) return;
	row = m_dwBeginSel >> 16;
	maxrow = pSndFile->PatternSize[m_nPattern];
	if (colmax >= pSndFile->m_nChannels) colmax = pSndFile->m_nChannels-1;
	if (colmin > colmax) return;
	pModDoc->PrepareUndo(m_nPattern, 0,0, pSndFile->m_nChannels, maxrow);
	for (UINT r=row; r<maxrow; r++)
	{
		MODCOMMAND *m = pSndFile->Patterns[m_nPattern] + r * pSndFile->m_nChannels + colmin;
		for (UINT c=colmin; c<=colmax; c++, m++)
		{
			if (r + nrows >= maxrow)
			{
				m->note = m->instr = 0;
				m->volcmd = m->vol = 0;
				m->command = m->param = 0;
			} else
			{
				*m = *(m+pSndFile->m_nChannels*nrows);
			}
		}
	}
	//rewbs.customKeys
	DWORD finalPos = (min(m_dwEndSel >> 16, m_dwBeginSel >> 16) << 16 | (m_dwEndSel & 0xFFFF));
	SetCurSel(finalPos, finalPos);
	SetCurrentColumn(finalPos & 0xFFFF);
	SetCurrentRow(finalPos >> 16);
	//end rewbs.customKeys
	
	pModDoc->SetModified();
	pModDoc->UpdateAllViews(this, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
	InvalidatePattern(FALSE);
}


void CViewPattern::OnDeleteRows()
//-------------------------------
{
	UINT colmin = (m_dwBeginSel & 0xFFFF) >> 3;
	UINT colmax = (m_dwEndSel & 0xFFFF) >> 3;
	UINT nrows = (m_dwEndSel >> 16) - (m_dwBeginSel >> 16) + 1;
	DeleteRows(colmin, colmax, nrows);
	//m_dwEndSel = (m_dwEndSel & 0x0000FFFF) | (m_dwBeginSel & 0xFFFF0000);
}


void CViewPattern::OnDeleteRowsEx()
//---------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	UINT nrows = (m_dwEndSel >> 16) - (m_dwBeginSel >> 16) + 1;
	DeleteRows(0, pSndFile->m_nChannels-1, nrows);
	m_dwEndSel = (m_dwEndSel & 0x0000FFFF) | (m_dwBeginSel & 0xFFFF0000);
}

void CViewPattern::InsertRows(UINT colmin, UINT colmax) //rewbs.customKeys: added args
//-----------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	UINT row, maxrow;

	if (!pModDoc || !(m_dwStatus&PATSTATUS_RECORD)) return;
	pSndFile = pModDoc->GetSoundFile();
	if (!pSndFile->Patterns[m_nPattern]) return;
	row = m_dwBeginSel >> 16;
	maxrow = pSndFile->PatternSize[m_nPattern];
	if (colmax >= pSndFile->m_nChannels) colmax = pSndFile->m_nChannels-1;
	if (colmin > colmax) return;
	pModDoc->PrepareUndo(m_nPattern, 0,0, pSndFile->m_nChannels, maxrow);

	for (UINT r=maxrow; r>row; )
	{
		r--;
		MODCOMMAND *m = pSndFile->Patterns[m_nPattern] + r * pSndFile->m_nChannels + colmin;
		for (UINT c=colmin; c<=colmax; c++, m++)
		{
			if (r <= row)
			{
				m->note = m->instr = 0;
				m->volcmd = m->vol = 0;
				m->command = m->param = 0;
			} else
			{
				*m = *(m-pSndFile->m_nChannels);
			}
		}
	}
	InvalidatePattern(FALSE);
	pModDoc->SetModified();
	pModDoc->UpdateAllViews(this, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
}


//rewbs.customKeys
void CViewPattern::OnInsertRows()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	UINT colmin, colmax, row, maxrow;

	if (!pModDoc) return;
	pSndFile = pModDoc->GetSoundFile();
	if (!pSndFile->Patterns[m_nPattern]) return;
	row = m_nRow;
	maxrow = pSndFile->PatternSize[m_nPattern];
	colmin = (m_dwBeginSel & 0xFFFF) >> 3;
	colmax = (m_dwEndSel & 0xFFFF) >> 3;
	
	InsertRows(colmin, colmax);
}
//end rewbs.customKeys

void CViewPattern::OnEditFind()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CFindReplaceTab pageFind(IDD_EDIT_FIND, FALSE, pModDoc);
		CFindReplaceTab pageReplace(IDD_EDIT_REPLACE, TRUE, pModDoc);
		CPropertySheet dlg("Find/Replace");

		pageFind.m_nNote = m_cmdFind.note;
		pageFind.m_nInstr = m_cmdFind.instr;
		pageFind.m_nVolCmd = m_cmdFind.volcmd;
		pageFind.m_nVol = m_cmdFind.vol;
		pageFind.m_nCommand = m_cmdFind.command;
		pageFind.m_nParam = m_cmdFind.param;
		pageFind.m_dwFlags = m_dwFindFlags;
		pageFind.m_nMinChannel = m_nFindMinChn;
		pageFind.m_nMaxChannel = m_nFindMaxChn;
		pageReplace.m_nNote = m_cmdReplace.note;
		pageReplace.m_nInstr = m_cmdReplace.instr;
		pageReplace.m_nVolCmd = m_cmdReplace.volcmd;
		pageReplace.m_nVol = m_cmdReplace.vol;
		pageReplace.m_nCommand = m_cmdReplace.command;
		pageReplace.m_nParam = m_cmdReplace.param;
		pageReplace.m_dwFlags = m_dwReplaceFlags;
		dlg.AddPage(&pageFind);
		dlg.AddPage(&pageReplace);
		if (dlg.DoModal() == IDOK)
		{
			m_cmdFind.note = pageFind.m_nNote;
			m_cmdFind.instr = pageFind.m_nInstr;
			m_cmdFind.volcmd = pageFind.m_nVolCmd;
			m_cmdFind.vol = pageFind.m_nVol;
			m_cmdFind.command = pageFind.m_nCommand;
			m_cmdFind.param = pageFind.m_nParam;
			m_nFindMinChn = pageFind.m_nMinChannel;
			m_nFindMaxChn = pageFind.m_nMaxChannel;
			m_dwFindFlags = pageFind.m_dwFlags;
			m_cmdReplace.note = pageReplace.m_nNote;
			m_cmdReplace.instr = pageReplace.m_nInstr;
			m_cmdReplace.volcmd = pageReplace.m_nVolCmd;
			m_cmdReplace.vol = pageReplace.m_nVol;
			m_cmdReplace.command = pageReplace.m_nCommand;
			m_cmdReplace.param = pageReplace.m_nParam;
			m_dwReplaceFlags = pageReplace.m_dwFlags;
			m_bContinueSearch = FALSE;
			OnEditFindNext();
		}
	}
}

void CViewPattern::OnEditGoto()
//-----------------------------
{
	CModDoc* pModDoc = GetDocument();
	if (!pModDoc)
		return;


	if (!m_pGotoWnd) {
		m_pGotoWnd = new CPatternGotoDialog(this);
		//if (m_pGotoWnd) m_pGotoWnd->SetParent(this/*, GetDocument()*/);
	}
	if (m_pGotoWnd)	{
		CSoundFile* pSndFile = pModDoc->GetSoundFile();
		UINT nCurrentOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER);
		UINT nCurrentChannel = ((m_dwCursor & 0xFFFF) >> 3) + 1;
		
		m_pGotoWnd->UpdatePos(m_nRow, nCurrentChannel, m_nPattern, nCurrentOrder, pSndFile);

		if (m_pGotoWnd->DoModal() == IDOK) {
			//Position should be valididated.
			
			if (m_pGotoWnd->m_nPattern != m_nPattern) {
				SetCurrentPattern(m_pGotoWnd->m_nPattern);
			}
			
			if (m_pGotoWnd->m_nOrder != nCurrentOrder) {
				SendCtrlMessage(CTRLMSG_SETCURRENTORDER,  m_pGotoWnd->m_nOrder);
			}

			if (m_pGotoWnd->m_nChannel != nCurrentChannel) {
				SetCurrentColumn((m_pGotoWnd->m_nChannel-1) << 3);
			}

			if (m_pGotoWnd->m_nRow != m_nRow) {
				SetCurrentRow(m_pGotoWnd->m_nRow);
			}
		}
	}
	return;
}

void CViewPattern::OnEditFindNext()
//---------------------------------
{
	CHAR s[512], szFind[64];
	UINT nFound = 0, nPatStart, nPatEnd;
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	BOOL bEffectEx;

	if (!pModDoc) return;
	if (!(m_dwFindFlags & ~PATSEARCH_FULLSEARCH))
	{
		PostMessage(WM_COMMAND, ID_EDIT_FIND);
		return;
	}
	BeginWaitCursor();
	pSndFile = pModDoc->GetSoundFile();
	nPatStart = m_nPattern;
	nPatEnd = m_nPattern+1;
	if (m_dwFindFlags & PATSEARCH_FULLSEARCH)
	{
		nPatStart = 0;
		nPatEnd = MAX_PATTERNS;
	}
	if (m_bContinueSearch)
	{
		nPatStart = m_nPattern;
	}
	bEffectEx = FALSE;
	if (m_dwFindFlags & PATSEARCH_COMMAND)
	{
		UINT fxndx = pModDoc->GetIndexFromEffect(m_cmdFind.command, m_cmdFind.param);
		bEffectEx = pModDoc->IsExtendedEffect(fxndx);
	}
	for (UINT nPat=nPatStart; nPat<nPatEnd; nPat++)
	{
		LPMODCOMMAND m = pSndFile->Patterns[nPat];
		DWORD len = pSndFile->m_nChannels * pSndFile->PatternSize[nPat];
		if ((!m) || (!len)) continue;
		UINT n = 0;
		if ((m_bContinueSearch) && (nPat == nPatStart) && (nPat == m_nPattern))
		{
			n = GetCurrentRow() * pSndFile->m_nChannels + GetCurrentChannel() + 1;
			m += n;
		}
		for (; n<len; n++, m++)
		{
			BOOL bFound = TRUE, bReplace = TRUE;

			if (m_dwFindFlags & PATSEARCH_CHANNEL)
			{
				UINT ch = n % pSndFile->m_nChannels;
				if ((ch < m_nFindMinChn) || (ch > m_nFindMaxChn)) bFound = FALSE;
			}
			if (((m_dwFindFlags & PATSEARCH_NOTE) && ((m->note != m_cmdFind.note) && ((m_cmdFind.note != 0xFD) || (!m->note) || (m->note & 0x80))))
			 || ((m_dwFindFlags & PATSEARCH_INSTR) && (m->instr != m_cmdFind.instr))
			 || ((m_dwFindFlags & PATSEARCH_VOLCMD) && (m->volcmd != m_cmdFind.volcmd))
			 || ((m_dwFindFlags & PATSEARCH_VOLUME) && (m->vol != m_cmdFind.vol))
			 || ((m_dwFindFlags & PATSEARCH_COMMAND) && (m->command != m_cmdFind.command))
			 || ((m_dwFindFlags & PATSEARCH_PARAM) && (m->param != m_cmdFind.param)))
			{
				bFound = FALSE;
			} else
			if (((m_dwFindFlags & (PATSEARCH_COMMAND|PATSEARCH_PARAM)) == PATSEARCH_COMMAND) && (bEffectEx))
			{
				if ((m->param & 0xF0) != (m_cmdFind.param & 0xF0)) bFound = FALSE;
			}
			// Found!
			if (bFound)
			{
				BOOL bUpdPos = TRUE;
				if ((m_dwReplaceFlags & (PATSEARCH_REPLACEALL|PATSEARCH_REPLACE)) == (PATSEARCH_REPLACEALL|PATSEARCH_REPLACE)) bUpdPos = FALSE;
				nFound++;
				if (bUpdPos)
				{
					SetCurrentPattern(nPat);
					SetCurrentRow(n / pSndFile->m_nChannels);
				}
				UINT ncurs = (n % pSndFile->m_nChannels) << 3;
				if (!(m_dwFindFlags & PATSEARCH_NOTE))
				{
					ncurs++;
					if (!(m_dwFindFlags & PATSEARCH_INSTR))
					{
						ncurs++;
						if (!(m_dwFindFlags & (PATSEARCH_VOLCMD|PATSEARCH_VOLUME)))
						{
							ncurs++;
							if (!(m_dwFindFlags & PATSEARCH_COMMAND)) ncurs++;
						}
					}
				}
				if (bUpdPos)
				{
					SetCurrentColumn(ncurs);
				}
				if (!(m_dwReplaceFlags & PATSEARCH_REPLACE)) goto EndSearch;
				if (!(m_dwReplaceFlags & PATSEARCH_REPLACEALL))
				{
					UINT ans = MessageBox("Replace this occurrence ?", "Replace", MB_YESNOCANCEL);
					if (ans == IDYES) bReplace = TRUE; else
					if (ans == IDNO) bReplace = FALSE; else goto EndSearch;
				}
				if (bReplace)
				{
					if ((m_dwReplaceFlags & PATSEARCH_NOTE))
					{
						// -1 octave
						if (m_cmdReplace.note == 0xFA)
						{
							if (m->note > 12) m->note -= 12;
						} else
						// +1 octave
						if (m_cmdReplace.note == 0xFB)
						{
							if (m->note <= 108) m->note += 12;
						} else
						// Note--
						if (m_cmdReplace.note == 0xFC)
						{
							if (m->note > 1) m->note--;
						} else
						// Note++
						if (m_cmdReplace.note == 0xFD)
						{
							if (m->note < 120) m->note++;
						} else m->note = m_cmdReplace.note;
					}
					if ((m_dwReplaceFlags & PATSEARCH_INSTR))
					{
						// Instr--
						if (m_cmdReplace.instr == 0xFC)
						{
							if (m->instr > 1) m->instr--;
						} else
						// Instr++
						if (m_cmdReplace.instr == 0xFD)
						{
							if (m->instr < MAX_INSTRUMENTS-1) m->instr++;
						} else m->instr = m_cmdReplace.instr;
					}
					if ((m_dwReplaceFlags & PATSEARCH_VOLCMD))
					{
						m->volcmd = m_cmdReplace.volcmd;
					}
					if ((m_dwReplaceFlags & PATSEARCH_VOLUME))
					{
						m->vol = m_cmdReplace.vol;
					}
					if ((m_dwReplaceFlags & PATSEARCH_COMMAND))
					{
						m->command = m_cmdReplace.command;
					}
					if ((m_dwReplaceFlags & PATSEARCH_PARAM))
					{
						if ((bEffectEx) && (!(m_dwReplaceFlags & PATSEARCH_COMMAND)))
							m->param = (BYTE)((m->param & 0xF0) | (m_cmdReplace.param & 0x0F));
						else
							m->param = m_cmdReplace.param;
					}
					pModDoc->SetModified();
					if (bUpdPos) InvalidateRow();
				}
			}
		}
	}
EndSearch:
	if (m_dwReplaceFlags & PATSEARCH_REPLACEALL) InvalidatePattern();
	m_bContinueSearch = TRUE;
	EndWaitCursor();
	// Display search results
	m_dwReplaceFlags &= ~PATSEARCH_REPLACEALL;
	if (!nFound)
	{
		if (m_dwFindFlags & PATSEARCH_NOTE)
		{
			UINT note = m_cmdFind.note;
			if (note == 0) strcpy(szFind, "..."); else
			if (note == 0xFE) strcpy(szFind, "^^ "); else
			if (note == 0xFF) strcpy(szFind, "== "); else
			wsprintf(szFind, "%s%d", szNoteNames[(note-1) % 12], (note-1)/12);
		} else strcpy(szFind, "???");
		strcat(szFind, " ");
		if (m_dwFindFlags & PATSEARCH_INSTR)
		{
			if (m_cmdFind.instr)
				wsprintf(&szFind[strlen(szFind)], "%03d", m_cmdFind.instr);
			else
				strcat(szFind, " ..");
		} else strcat(szFind, " ??");
		strcat(szFind, " ");
		if (m_dwFindFlags & PATSEARCH_VOLCMD)
		{
			if (m_cmdFind.volcmd)
				wsprintf(&szFind[strlen(szFind)], "%c", gszVolCommands[m_cmdFind.volcmd]);
			else
				strcat(szFind, ".");
		} else strcat(szFind, "?");
		if (m_dwFindFlags & PATSEARCH_VOLUME)
		{
			wsprintf(&szFind[strlen(szFind)], "%02d", m_cmdFind.vol);
		} else strcat(szFind, "??");
		strcat(szFind, " ");
		if (m_dwFindFlags & PATSEARCH_COMMAND)
		{
			if (m_cmdFind.command)
			{
				if (pSndFile->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
					wsprintf(&szFind[strlen(szFind)], "%c", gszS3mCommands[m_cmdFind.command]);
				else
					wsprintf(&szFind[strlen(szFind)], "%c", gszModCommands[m_cmdFind.command]);
			} else
				strcat(szFind, ".");
		} else strcat(szFind, "?");
		if (m_dwFindFlags & PATSEARCH_PARAM)
		{
			wsprintf(&szFind[strlen(szFind)], "%02X", m_cmdFind.param);
		} else strcat(szFind, "??");
		wsprintf(s, "Cannot find \"%s\"", szFind);
		MessageBox(s, "Find/Replace", MB_OK | MB_ICONINFORMATION);
	}
}


void CViewPattern::OnPatternStep()
//--------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pMainFrm) && (pModDoc))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		if ((!pSndFile->PatternSize[m_nPattern]) || (!pSndFile->Patterns[m_nPattern])) return;
		// Cut instrument/sample
		BEGIN_CRITICAL();
		for (UINT i=pSndFile->m_nChannels; i<MAX_CHANNELS; i++)
		{
			pSndFile->Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
			pSndFile->m_dwSongFlags &= ~SONG_PAUSED;
			pSndFile->LoopPattern(m_nPattern);
			pSndFile->m_nNextRow = GetCurrentRow();
			pSndFile->m_dwSongFlags |= SONG_STEP;
		}
		END_CRITICAL();
		if (pMainFrm->GetModPlaying() != pModDoc)
		{
			pMainFrm->PlayMod(pModDoc, m_hWnd, MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);
		}
		CMainFrame::EnableLowLatencyMode();
		if (CMainFrame::m_dwPatternSetup & PATTERN_CONTSCROLL)
			SetCurrentRow(GetCurrentRow()+1, TRUE);
		else
			SetCurrentRow((GetCurrentRow()+1) % pSndFile->PatternSize[m_nPattern], FALSE);
		SetFocus();
	}
}


void CViewPattern::OnCursorCopy()
//-------------------------------
{
	CModDoc *pModDoc = GetDocument();

	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		if ((!pSndFile->PatternSize[m_nPattern]) || (!pSndFile->Patterns[m_nPattern])) return;
		MODCOMMAND *m = pSndFile->Patterns[m_nPattern] + m_nRow * pSndFile->m_nChannels + ((m_dwCursor & 0xFFFF) >> 3);
		switch(m_dwCursor & 7)
		{
		case 0:
		case 1:
			m_cmdOld.note = m->note;
			m_cmdOld.instr = m->instr;
			SendCtrlMessage(CTRLMSG_SETCURRENTINSTRUMENT, m_cmdOld.instr);
			break;
		case 2:
			m_cmdOld.volcmd = m->volcmd;
			m_cmdOld.vol = m->vol;
			break;
		case 3:
		case 4:
			m_cmdOld.command = m->command;
			m_cmdOld.param = m->param;
			break;
		}
	}
}

void CViewPattern::OnCursorPaste()
//--------------------------------
{
	//rewbs.customKeys
 	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();
	
  	if ((pModDoc) && (pMainFrm))
	{
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		UINT nCursor = m_dwCursor & 0x07;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern] +  m_nRow*pSndFile->m_nChannels + nChn;
		MODCOMMAND oldcmd = *p;

		switch(nCursor)
		{
			case 0:	p->note = m_cmdOld.note;									//Note
			case 1:	p->instr = m_cmdOld.instr; break;							//Octave
			case 2:	p->vol = m_cmdOld.vol; p->volcmd = m_cmdOld.volcmd; break;	//Vol
			case 3:																//Effect
			case 4:	p->command = m_cmdOld.command; p->param = m_cmdOld.param; break;
		}
		pModDoc->SetModified();

		if (((pMainFrm->GetFollowSong(pModDoc) != m_hWnd) || (pSndFile->IsPaused()) || (!(m_dwStatus & PATSTATUS_FOLLOWSONG))))
		{
			DWORD sel = m_dwCursor | (m_nRow << 16);
			InvalidateArea(sel, sel+5); //rewbs.fix3010 (no refresh under on last row)
			SetCurrentRow(m_nRow+m_nSpacing);
			sel = m_dwCursor | (m_nRow << 16);
			SetCurSel(sel, sel);
		}
	}
	//end rewbs.customKeys
}


void CViewPattern::OnInterpolateVolume()
//--------------------------------------
{
	Interpolate(VOL_COLUMN);
}

void CViewPattern::OnOpenRandomizer()
//--------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc) {
		if (m_pRandomizer) {
			if (m_pRandomizer->isGUIVisible()) { //window already there, update data
				//m_pRandomizer->UpdateSelection(rowStart, rowEnd, nchn, pModDoc, m_nPattern);
			} else {
				m_pRandomizer->showGUI();
			}
		}
		else {
			//Open window & send data
			m_pRandomizer = new CPatternRandomizer(this);
			if (m_pRandomizer) {
				m_pRandomizer->showGUI();
			}	
		}
	}
}

//begin rewbs.fxvis
void CViewPattern::OnVisualizeEffect()
//--------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		UINT row0 = m_dwBeginSel >> 16, row1 = m_dwEndSel >> 16, nchn = (m_dwBeginSel & 0xFFFF) >> 3;
		if (m_pEffectVis)
		{
			//window already there, update data
			m_pEffectVis->UpdateSelection(row0, row1, nchn, pModDoc, m_nPattern);
		}
		else
		{
			//Open window & send data
			m_pEffectVis = new CEffectVis(this, row0, row1, nchn, pModDoc, m_nPattern);
			if (m_pEffectVis)
			{
				m_pEffectVis->OpenEditor(CMainFrame::GetMainFrame());
				// HACK: to get status window set up; must create clear destinction between 
				// construction, 1st draw code and all draw code.
				m_pEffectVis->OnSize(0,0,0);

			}	
		}
	}
}
//end rewbs.fxvis


void CViewPattern::OnInterpolateEffect()
//--------------------------------------
{
	Interpolate(EFFECT_COLUMN);
}

void CViewPattern::OnInterpolateNote()
//--------------------------------------
{
	Interpolate(NOTE_COLUMN);
}


//static void CArrayUtils<UINT>::Merge(CArray<UINT,UINT>& Dest, CArray<UINT,UINT>& Src);

void CViewPattern::Interpolate(UINT type)
//---------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (!pModDoc)
		return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile();

	bool changed = false;
	CArray<UINT,UINT> validChans;
	
	if (type==EFFECT_COLUMN || type==PARAM_COLUMN) {
		CArray<UINT,UINT> moreValidChans;
        ListChansWhereColSelected(EFFECT_COLUMN, validChans);
		ListChansWhereColSelected(PARAM_COLUMN, moreValidChans);
		//CArrayUtils<UINT>::Merge(validChans, moreValidChans); //Causes unresolved external, not sure why yet.
		validChans.Append(moreValidChans);						//for now we'll jost interpolate the same data several times. :)
	} else {
		ListChansWhereColSelected(type, validChans);
	}
	
	int nValidChans = validChans.GetCount();

	//for all channels where type is selected
	for (int chnIdx=0; chnIdx<nValidChans; chnIdx++) {
		MODCOMMAND *pcmd = pSndFile->Patterns[m_nPattern];
		UINT nchn = validChans[chnIdx];
		UINT row0 = GetSelectionStartRow();
		UINT row1 = GetSelectionEndRow();
		
		if (!IsInterpolationPossible(row0, row1, nchn, type, pSndFile)) { 
			continue; //skip chans where interpolation isn't possible
		}
		
		if (!changed) { //ensure we save undo buffer only before any channels are interpolated
			PrepareUndo(m_dwBeginSel, m_dwEndSel);
		}
		
		int vsrc, vdest, vcmd, verr, distance;
		distance = row1 - row0;

		switch(type) {
			case NOTE_COLUMN:
				vsrc = pcmd[row0 * pSndFile->m_nChannels + nchn].note;
				vdest = pcmd[row1 * pSndFile->m_nChannels + nchn].note;
				vcmd = pcmd[row0 * pSndFile->m_nChannels + nchn].instr;
				verr = (distance * 59) / 120;
				break;
			case VOL_COLUMN:
				vsrc = pcmd[row0 * pSndFile->m_nChannels + nchn].vol;
				vdest = pcmd[row1 * pSndFile->m_nChannels + nchn].vol;
				vcmd = pcmd[row0 * pSndFile->m_nChannels + nchn].volcmd;
				verr = (distance * 63) / 128;
				break;
			case PARAM_COLUMN:
			case EFFECT_COLUMN:
				vsrc = pcmd[row0 * pSndFile->m_nChannels + nchn].param;
				vdest = pcmd[row1 * pSndFile->m_nChannels + nchn].param;
				vcmd = pcmd[row0 * pSndFile->m_nChannels + nchn].command;
				verr = (distance * 63) / 128;
				break;
			default:
				ASSERT(false);
				return;
		}

		if (vdest < vsrc) verr = -verr;
		pcmd += row0 * pSndFile->m_nChannels + nchn;
		
		for (UINT i=0; i<=distance; i++, pcmd += pSndFile->m_nChannels)	{
		
			switch(type) {
				case NOTE_COLUMN:
					if ((!pcmd->note) || (pcmd->instr == vcmd))	{
						int note = vsrc + ((vdest - vsrc) * (int)i + verr) / distance;
						pcmd->note = (BYTE)note;
						pcmd->instr = vcmd;
					}
					break;
				case VOL_COLUMN:
					if ((!pcmd->volcmd) || (pcmd->volcmd == vcmd))	{
						int vol = vsrc + ((vdest - vsrc) * (int)i + verr) / distance;
						pcmd->vol = (BYTE)vol;
						pcmd->volcmd = vcmd;
					}
					break;
				case EFFECT_COLUMN:
					if ((!pcmd->command) || (pcmd->command == vcmd)) {
						int val = vsrc + ((vdest - vsrc) * (int)i + verr) / distance;
						pcmd->param = (BYTE)val;
						pcmd->command = vcmd;
					}
					break;
				default:
					ASSERT(false);
			}
		}
		
		changed=true;

	} //end for all channels where type is selected

	if (changed) {
		pModDoc->SetModified();
		InvalidatePattern(FALSE);
	}
}


BOOL CViewPattern::TransposeSelection(int transp)
//-----------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		UINT row0 = m_dwBeginSel >> 16, row1 = m_dwEndSel >> 16;
		UINT col0 = ((m_dwBeginSel & 0xFFFF)+7) >> 3, col1 = (m_dwEndSel & 0xFFFF) >> 3;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *pcmd = pSndFile->Patterns[m_nPattern];

		if ((!pcmd) || (col0 > col1) || (col1 >= pSndFile->m_nChannels)
		 || (row0 > row1) || (row1 >= pSndFile->PatternSize[m_nPattern])) return FALSE;
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		for (UINT row=row0; row <= row1; row++)
		{
			MODCOMMAND *m = &pcmd[row * pSndFile->m_nChannels];
			for (UINT col=col0; col<=col1; col++)
			{
				int note = m[col].note;
				if ((note) && (note <= 120))
				{
					note += transp;
					if (note < 1) note = 1;
					if (note > 120) note = 120;
					m[col].note = (BYTE)note;
				}
			}
		}
		pModDoc->SetModified();
		InvalidateSelection();
		return TRUE;
	}
	return FALSE;
}


void CViewPattern::OnDropSelection()
//----------------------------------
{
	MODCOMMAND *pOldPattern, *pNewPattern;
	CModDoc *pModDoc;
	CSoundFile *pSndFile;
	int x1, y1, x2, y2, c1, c2, xc1, xc2, xmc1, xmc2, ym1, ym2;
	int dx, dy, nChannels, nRows;

	if ((pModDoc = GetDocument()) == NULL || !(m_dwStatus&PATSTATUS_RECORD)) return;
	pSndFile = pModDoc->GetSoundFile();
	nChannels = pSndFile->m_nChannels;
	nRows = pSndFile->PatternSize[m_nPattern];
	pOldPattern = pSndFile->Patterns[m_nPattern];
	if ((nChannels < 1) || (nRows < 1) || (!pOldPattern)) return;
	dx = (int)((m_dwDragPos & 0xFFF8) >> 3) - (int)((m_dwStartSel & 0xFFF8) >> 3);
	dy = (int)(m_dwDragPos >> 16) - (int)(m_dwStartSel >> 16);
	if ((!dx) && (!dy)) return;
	pModDoc->PrepareUndo(m_nPattern, 0,0, nChannels, nRows);
	pNewPattern = CSoundFile::AllocatePattern(nRows, nChannels);
	if (!pNewPattern) return;
	x1 = (m_dwBeginSel & 0xFFF8) >> 3;
	y1 = (m_dwBeginSel) >> 16;
	x2 = (m_dwEndSel & 0xFFF8) >> 3;
	y2 = (m_dwEndSel) >> 16;
	c1 = (m_dwBeginSel&7);
	c2 = (m_dwEndSel&7);
	if (c1 > 3) c1 = 3;
	if (c2 > 3) c2 = 3;
	xc1 = x1*4+c1;
	xc2 = x2*4+c2;
	xmc1 = xc1+dx*4;
	xmc2 = xc2+dx*4;
	ym1 = y1+dy;
	ym2 = y2+dy;
	BeginWaitCursor();
	MODCOMMAND *p = pNewPattern;
	for (int y=0; y<nRows; y++)
	{
		for (int x=0; x<nChannels; x++)
		{
			for (int c=0; c<4; c++)
			{
				int xsrc=x, ysrc=y, xc=x*4+c;
				
				// Destination is from destination selection
				if ((xc >= xmc1) && (xc <= xmc2) && (y >= ym1) && (y <= ym2))
				{
					xsrc -= dx;
					ysrc -= dy;
				} else
				// Destination is from source rectangle (clear)
				if ((xc >= xc1) && (xc <= xc2) && (y >= y1) && (y <= y2))
				{
					if (!(m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_CTRLDRAGSEL))) xsrc = -1;
				}
				// Copy the data
				if ((xsrc >= 0) && (xsrc < nChannels) && (ysrc >= 0) && (ysrc < nRows))
				{
					MODCOMMAND *psrc = &pOldPattern[ysrc*nChannels+xsrc];
					switch(c)
					{
					case 0:	p->note = psrc->note; break;
					case 1: p->instr = psrc->instr; break;
					case 2: p->vol = psrc->vol; p->volcmd = psrc->volcmd; break;
					default: p->command = psrc->command; p->param = psrc->param; break;
					}
				}
			}
			p++;
		}
	}
	BEGIN_CRITICAL();
	pSndFile->Patterns[m_nPattern] = pNewPattern;
	END_CRITICAL();
	x1 += dx;
	x2 += dx;
	y1 += dy;
	y2 += dy;
	if (x1<0) { x1=0; c1=0; }
	if (x1>=nChannels) x1=nChannels-1;
	if (x2<0) x2 = 0;
	if (x2>=nChannels) { x2=nChannels-1; c2=4; }
	if (y1<0) y1=0;
	if (y1>=nRows) y1=nRows-1;
	if (y2<0) y2=0;
	if (y2>=nRows) y2=nRows-1;
	if (c2 >= 3) c2 = 4;
	SetCurrentRow(y1);
	SetCurrentColumn((x1<<3)|c1);
	SetCurSel((y1<<16)|(x1<<3)|c1, (y2<<16)|(x2<<3)|c2);
	InvalidatePattern();
	CSoundFile::FreePattern(pOldPattern);
	pModDoc->SetModified();
	EndWaitCursor();
}


void CViewPattern::OnSetSelInstrument()
//-------------------------------------
{
	CModDoc *pModDoc;
	CSoundFile *pSndFile;
	UINT nIns = GetCurrentInstrument();
	MODCOMMAND *p;
	BOOL bModified;
	
	if (!nIns) return;
	if ((pModDoc = GetDocument()) == NULL) return;
	pSndFile = pModDoc->GetSoundFile();
	p = pSndFile->Patterns[m_nPattern];
	if (!p) return;
	BeginWaitCursor();
	PrepareUndo(m_dwBeginSel, m_dwEndSel);
	bModified = FALSE;

	//rewbs: re-written to work regardless of selection
	UINT startRow  = GetSelectionStartRow();
	UINT endRow    = GetSelectionEndRow();
	UINT startChan = GetSelectionStartChan();
	UINT endChan   = GetSelectionEndChan();

	for (UINT r=startRow; r<endRow+1; r++)
	{
		for (UINT c=startChan; c<endChan+1; c++)
		{
			p = pSndFile->Patterns[m_nPattern] + r * pSndFile->m_nChannels + c;
			if (p->instr && p->instr != (BYTE)nIns)
			{
				p->instr = (BYTE)nIns;
				bModified = TRUE;
			}
		}
	}

	if (bModified)	{
		pModDoc->SetModified();
		pModDoc->UpdateAllViews(NULL, HINT_PATTERNDATA | (m_nPattern << 24), NULL);
	}
	EndWaitCursor();
}


void CViewPattern::OnTransposeUp()
//--------------------------------
{
	TransposeSelection(1);
}


void CViewPattern::OnTransposeDown()
//----------------------------------
{
	TransposeSelection(-1);
}


void CViewPattern::OnTransposeOctUp()
//-----------------------------------
{
	TransposeSelection(12);
}


void CViewPattern::OnTransposeOctDown()
//-------------------------------------
{
	TransposeSelection(-12);
}


void CViewPattern::OnSwitchToOrderList()
//--------------------------------------
{
	PostCtrlMessage(CTRLMSG_SETFOCUS);
}


void CViewPattern::OnPrevOrder()
//------------------------------
{
	PostCtrlMessage(CTRLMSG_PREVORDER);
}


void CViewPattern::OnNextOrder()
//------------------------------
{
	PostCtrlMessage(CTRLMSG_NEXTORDER);
}


void CViewPattern::OnUpdateUndo(CCmdUI *pCmdUI)
//---------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((pCmdUI) && (pModDoc))
	{
		pCmdUI->Enable(pModDoc->CanUndo());
	}
}


void CViewPattern::OnEditUndo()
//-----------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc && m_dwStatus&PATSTATUS_RECORD)
	{
		UINT nPat = pModDoc->DoUndo();
		if (nPat < MAX_PATTERNS)
		{
			pModDoc->SetModified();
			if (nPat != m_nPattern)
				SetCurrentPattern(nPat);
			else
				InvalidatePattern(TRUE);
		}
	}
}


void CViewPattern::OnPatternAmplify()
//-----------------------------------
{
	static UINT snOldAmp = 100;
	CAmpDlg dlg(this, snOldAmp);
	CModDoc *pModDoc = GetDocument();
	BYTE chvol[MAX_BASECHANNELS];

	if ((pModDoc) && (dlg.DoModal() == IDOK))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		BeginWaitCursor();
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		snOldAmp = dlg.m_nFactor;
		memset(chvol, 64, sizeof(chvol));
		if (pSndFile->Patterns[m_nPattern])
		{
			MODCOMMAND *p = pSndFile->Patterns[m_nPattern];

			for (UINT j=0; j<pSndFile->m_nChannels; j++)
			{
				for (UINT i=0; i<pSndFile->PatternSize[m_nPattern]; i++)
				{
					if ((i >= (m_dwBeginSel >> 16)) && (i <= (m_dwEndSel >> 16)))
					{
						MODCOMMAND *m = p + i*pSndFile->m_nChannels+j;
						if ((m->command == CMD_VOLUME) && (m->param <= 64))
						{
							chvol[j] = m->param;
							break;
						}
						if (m->volcmd == VOLCMD_VOLUME)
						{
							chvol[j] = m->vol;
							break;
						}
						if ((m->note) && (m->note < 0x80) && (m->instr))
						{
							UINT nSmp = m->instr;
							if (pSndFile->m_nInstruments)
							{
								if ((nSmp <= pSndFile->m_nInstruments) && (pSndFile->Headers[nSmp]))
								{
									nSmp = pSndFile->Headers[nSmp]->Keyboard[m->note];
								} else
								{
									nSmp = 0;
								}
							}
							if ((nSmp) && (nSmp <= pSndFile->m_nSamples))
							{
								chvol[j] = (BYTE)(pSndFile->Ins[nSmp].nVolume >> 2);
								break;
							}
						}
					}
				}
			}
			for (UINT y=0; y<pSndFile->PatternSize[m_nPattern]; y++)
			{
				if ((y >= (m_dwBeginSel >> 16)) && (y <= (m_dwEndSel >> 16)))
				{
					int cy = (m_dwEndSel>>16) - (m_dwBeginSel>>16) + 1;
					int y0 = (m_dwBeginSel>>16);
					for (UINT x=0; x<pSndFile->m_nChannels; x++)
					{
						UINT col = (x<<3) | 2;
						if ((col >= (m_dwBeginSel & 0xFFFF)) && (col <= (m_dwEndSel & 0xFFFF)))
						{
							if ((!p[x].volcmd) && (p[x].command != CMD_VOLUME)
							 && (p[x].note) && (p[x].note < 0x80) && (p[x].instr))
							{
								UINT nSmp = p[x].instr;
								if (pSndFile->m_nInstruments)
								{
									if ((nSmp <= pSndFile->m_nInstruments) && (pSndFile->Headers[nSmp]))
									{
										nSmp = pSndFile->Headers[nSmp]->Keyboard[p[x].note];
									} else
									{
										nSmp = 0;
									}
								}
								if ((nSmp) && (nSmp <= pSndFile->m_nSamples))
								{
									p[x].volcmd = VOLCMD_VOLUME;
									p[x].vol = pSndFile->Ins[nSmp].nVolume >> 2;
								}
							}
							if (p[x].volcmd == VOLCMD_VOLUME) chvol[x] = (BYTE)p[x].vol;
							if (((dlg.m_bFadeIn) || (dlg.m_bFadeOut)) && (p[x].command != CMD_VOLUME) && (!p[x].volcmd))
							{
								p[x].volcmd = VOLCMD_VOLUME;
								p[x].vol = chvol[x];
							}
							if (p[x].volcmd == VOLCMD_VOLUME)
							{
								int vol = p[x].vol * dlg.m_nFactor;
								if (dlg.m_bFadeIn) vol = (vol * (y+1-y0)) / cy;
								if (dlg.m_bFadeOut) vol = (vol * (cy+y0-y)) / cy;
								vol = (vol+50) / 100;
								if (vol > 64) vol = 64;
								p[x].vol = (BYTE)vol;
							}
						}
						col = (x<<3) | 3;
						if ((col >= (m_dwBeginSel & 0xFFFF)) && (col <= (m_dwEndSel & 0xFFFF)))
						{
							if ((p[x].command == CMD_VOLUME) && (p[x].param <= 64))
							{
								int vol = p[x].param * dlg.m_nFactor;
								if (dlg.m_bFadeIn) vol = (vol * (y+1-y0)) / cy;
								if (dlg.m_bFadeOut) vol = (vol * (cy+y0-y)) / cy;
								vol = (vol+50) / 100;
								if (vol > 64) vol = 64;
								p[x].param = (BYTE)vol;
							}
						}
					}
				}
				p += pSndFile->m_nChannels;
			}
		}
		pModDoc->SetModified();
		InvalidateSelection();
		EndWaitCursor();
	}
}


LRESULT CViewPattern::OnPlayerNotify(MPTNOTIFICATION *pnotify)
//------------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if ((!pModDoc) || (!pnotify)) return 0;
	if (pnotify->dwType & MPTNOTIFY_POSITION)
	{
		UINT nOrd = pnotify->nOrder;
		UINT nRow = pnotify->nRow;
		UINT nPat = 0xFFFF;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		
		if (nRow < m_nLastPlayedRow) {
			InvalidateChannelsHeaders();
		}
		m_nLastPlayedRow = nRow;

		if (pSndFile->m_dwSongFlags & (SONG_PAUSED|SONG_STEP)) return 0;
		if (pSndFile->m_dwSongFlags & SONG_PATTERNLOOP)
		{
			nPat = pSndFile->m_nPattern;
			nOrd = 0xFFFF;
		} else
		if (nOrd < MAX_ORDERS)
		{
			nPat = pSndFile->Order[nOrd];
		}
		//rewbs.fxVis: 
		if (m_pEffectVis && m_pEffectVis->m_hWnd) {	
			m_pEffectVis->SetPlayCursor(nPat, nRow);
		}

		if ((m_dwStatus & (PATSTATUS_FOLLOWSONG|PATSTATUS_DRAGVSCROLL|PATSTATUS_DRAGHSCROLL|PATSTATUS_MOUSEDRAGSEL)) == PATSTATUS_FOLLOWSONG)
		{
			if (nPat < MAX_PATTERNS)
			{
				if (nPat != m_nPattern) SetCurrentPattern(nPat, nRow);
				if (nRow != m_nRow)	SetCurrentRow((nRow < pSndFile->PatternSize[nPat]) ? nRow : 0);
			}
			SetPlayCursor(0xFFFF, 0);
			if (nOrd < MAX_ORDERS) SendCtrlMessage(CTRLMSG_SETCURRENTORDER, nOrd);
		} else
		{
			SetPlayCursor(nPat, nRow);
		}

	}
	if ((pnotify->dwType & (MPTNOTIFY_VUMETERS|MPTNOTIFY_STOP)) && (m_dwStatus & PATSTATUS_VUMETERS))
	{
		UpdateAllVUMeters(pnotify);
	}
	
	UpdateIndicator();

	return 0;


}


LRESULT CViewPattern::OnMidiMsg(WPARAM dwMidiData, LPARAM)
//--------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((!pModDoc) || (!pMainFrm)) return 0;

//Midi message from our perspective: 
//     +---------------------------+---------------------------+-------------+-------------+
//bit: | 24.23.22.21 | 20.19.18.17 | 16.15.14.13 | 12.11.10.09 | 08.07.06.05 | 04.03.02.01 |
//     +---------------------------+---------------------------+-------------+-------------+
//     |     Velocity (0-127)      |  Note (middle C is 60)    |   Event     |   Channel   |
//     +---------------------------+---------------------------+-------------+-------------+
//(http://www.borg.com/~jglatt/tech/midispec.htm)

	//Notes:
	//. If no event is recieved, previous event is assumed.
	//. A note-on (event=8) with velocity 0 is equivalent to a note off.
	//. We only handle note-on and note off events.
	//. Basing the event solely on the velocity as follows is incorrect,
	//  since a note-off can have a velocity too:
	//  BYTE event  = (dwMidiData>>16) & 0x64; 

	BYTE nNote  = ((dwMidiData >> 8) & 0xFF) +1; // +1 is for MPT, where middle C is 61
	BYTE nVol   = (dwMidiData >> 16) & 0xFF;   // At this stage nVol is a non linear value in [0;127]
	                                           // Need to convert to linear in [0;64] - see below
	BYTE event  = dwMidiData & 0xF0;
	if ((event == 0x90) && !nVol) event = 0x80;	//Convert event to note-off if req'd
	
	//Event nibble: 8 if Note Off, 9 is Note on. Don't care about other events ATM.
	switch(event) 
	{
	case 0x80: // Note Off
		// The following method takes care of:
		// . Silencing specific active notes (just setting nNote to 255 as was done before is not acceptible)
		// . Entering a note off in pattern if required
		TempStopNote(nNote, CMainFrame::m_dwMidiSetup & MIDISETUP_RECORDNOTEOFF);
		break;

	case 0x90: // Note On
		if (CMainFrame::m_dwMidiSetup & MIDISETUP_RECORDVELOCITY)
		{   
			//Convert non linear value in [0;127] to linear value in [0;64]:
			nVol = (CDLSBank::DLSMidiVolumeToLinear(nVol)+1023) >> 10;
			//Amplify if requested:
			if (CMainFrame::m_dwMidiSetup & MIDISETUP_AMPLIFYVELOCITY) nVol *= 2;
			//Bound, in case of overamplification:
			if (nVol < 1) nVol = 1;
			if (nVol > 64) nVol = 64;
		}
		else
		{	//Use default volume
			nVol = -1; 
		}

		TempEnterNote(nNote, true, nVol);
		break;
	}

	
	return 0;
}




LRESULT CViewPattern::OnModViewMsg(WPARAM wParam, LPARAM lParam)
//--------------------------------------------------------------
{
	switch(wParam)
	{

// -> CODE#0012
// -> DESC="midi keyboard split"
//rewbs.merge: inverted message direction.
	case VIEWMSG_SETSPLITINSTRUMENT:
		m_nSplitInstrument = lParam;
		break;
	case VIEWMSG_SETSPLITNOTE:
		m_nSplitNote = lParam;
		break;
	case VIEWMSG_SETOCTAVEMODIFIER:
		m_nOctaveModifier = lParam;
		break;
	case VIEWMSG_SETOCTAVELINK:
		m_bOctaveLink = lParam;
		break;
	case VIEWMSG_SETSPLITVOLUME:
		m_nSplitVolume = lParam;
		break;
// -! NEW_FEATURE#0012

	case VIEWMSG_SETCTRLWND:
		m_hWndCtrl = (HWND)lParam;
		SetCurrentPattern(SendCtrlMessage(CTRLMSG_GETCURRENTPATTERN));
		break;

	case VIEWMSG_GETCURRENTPATTERN:
		return m_nPattern;

	case VIEWMSG_SETCURRENTPATTERN:
		if (m_nPattern != (UINT)lParam) SetCurrentPattern(lParam);
		break;

	case VIEWMSG_GETCURRENTPOS:
		return (m_nPattern << 16) | (m_nRow);

	case VIEWMSG_FOLLOWSONG:
		m_dwStatus &= ~PATSTATUS_FOLLOWSONG;
		if (lParam)
		{
			CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
			CModDoc *pModDoc = GetDocument();
			m_dwStatus |= PATSTATUS_FOLLOWSONG;
			if (pModDoc) pModDoc->SetFollowWnd(m_hWnd, MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);
			if (pMainFrm) pMainFrm->SetFollowSong(pModDoc, m_hWnd, TRUE, MPTNOTIFY_POSITION|MPTNOTIFY_VUMETERS);
			SetFocus();
		} else
		{
			InvalidateRow();
		}
		break;

	case VIEWMSG_SETRECORD:
		if (lParam) m_dwStatus |= PATSTATUS_RECORD; else m_dwStatus &= ~PATSTATUS_RECORD;
		break;

	case VIEWMSG_SETSPACING:
		m_nSpacing = lParam;
		break;

	case VIEWMSG_PATTERNPROPERTIES:
		OnPatternProperties();
		GetParentFrame()->SetActiveView(this);
		break;

	case VIEWMSG_SETVUMETERS:
		if (lParam) m_dwStatus |= PATSTATUS_VUMETERS; else m_dwStatus &= ~PATSTATUS_VUMETERS;
		UpdateSizes();
		UpdateScrollSize();
		InvalidatePattern(TRUE);
		break;

	case VIEWMSG_SETPLUGINNAMES:
		if (lParam) m_dwStatus |= PATSTATUS_PLUGNAMESINHEADERS; else m_dwStatus &= ~PATSTATUS_PLUGNAMESINHEADERS;
		UpdateSizes();
		UpdateScrollSize();
		InvalidatePattern(TRUE);
		break;

	case VIEWMSG_DOMIDISPACING:
		if (m_nSpacing)
		{
// -> CODE#0012
// -> DESC="midi keyboard split"
			CModDoc *pModDoc = GetDocument();
			CSoundFile * pSndFile = pModDoc->GetSoundFile();
//			if (timeGetTime() - lParam >= 10)
			int temp = timeGetTime();
			if (temp - lParam >= 60)
// -! NEW_FEATURE#0012
			{
				CModDoc *pModDoc = GetDocument();
				CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
				if ((!(m_dwStatus & PATSTATUS_FOLLOWSONG))
				 || (pMainFrm->GetFollowSong(pModDoc) != m_hWnd)
				 || (pModDoc->GetSoundFile()->IsPaused()))
				{
					SetCurrentRow(m_nRow + m_nSpacing);
				}
				m_dwStatus &= ~PATSTATUS_MIDISPACINGPENDING;
			} else
			{
// -> CODE#0012
// -> DESC="midi keyboard split"
//				Sleep(1);
				Sleep(0);
				PostMessage(WM_MOD_VIEWMSG, VIEWMSG_DOMIDISPACING, lParam);
			}
		} else
		{
			m_dwStatus &= ~PATSTATUS_MIDISPACINGPENDING;
		}
		break;

	case VIEWMSG_LOADSTATE:
		if (lParam)
		{
			PATTERNVIEWSTATE *pState = (PATTERNVIEWSTATE *)lParam;
			if (pState->nDetailLevel) m_nDetailLevel = pState->nDetailLevel;
			if (/*(pState->nPattern == m_nPattern) && */(pState->cbStruct == sizeof(PATTERNVIEWSTATE)))
			{
				SetCurrentPattern(pState->nPattern);
				SetCurrentRow(pState->nRow);
				SetCurrentColumn(pState->nCursor);
				SetCurSel(pState->dwBeginSel, pState->dwEndSel);
			}
		}
		break;

	case VIEWMSG_SAVESTATE:
		if (lParam)
		{
			PATTERNVIEWSTATE *pState = (PATTERNVIEWSTATE *)lParam;
			pState->cbStruct = sizeof(PATTERNVIEWSTATE);
			pState->nPattern = m_nPattern;
			pState->nRow = m_nRow;
			pState->nCursor = m_dwCursor;
			pState->dwBeginSel = m_dwBeginSel;
			pState->dwEndSel = m_dwEndSel;
			pState->nDetailLevel = m_nDetailLevel;
			pState->nOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER); //rewbs.playSongFromCursor
		}
		break;

	case VIEWMSG_EXPANDPATTERN:
		{
			CModDoc *pModDoc = GetDocument();
			if (pModDoc->ExpandPattern(m_nPattern)) { m_nRow *= 2; SetCurrentPattern(m_nPattern); }
		}
		break;

	case VIEWMSG_SHRINKPATTERN:
		{
			CModDoc *pModDoc = GetDocument();
			if (pModDoc->ShrinkPattern(m_nPattern)) { m_nRow /= 2; SetCurrentPattern(m_nPattern); }
		}
		break;

	case VIEWMSG_COPYPATTERN:
		{
			CModDoc *pModDoc = GetDocument();
			if (pModDoc)
			{
				CSoundFile *pSndFile = pModDoc->GetSoundFile();
				pModDoc->CopyPattern(m_nPattern, 0, ((pSndFile->PatternSize[m_nPattern]-1)<<16)|(pSndFile->m_nChannels<<3));
			}
		}
		break;

	case VIEWMSG_PASTEPATTERN:
		{
			CModDoc *pModDoc = GetDocument();
			if (pModDoc) pModDoc->PastePattern(m_nPattern, 0, false);
		}
		break;

	case VIEWMSG_AMPLIFYPATTERN:
		OnPatternAmplify();
		break;

	case VIEWMSG_SETDETAIL:
		if (lParam != (LONG)m_nDetailLevel)
		{
			m_nDetailLevel = lParam;
			UpdateSizes();
			UpdateScrollSize();
			SetCurrentColumn(m_dwCursor);
			InvalidatePattern(TRUE);
		}
		break;
	case VIEWMSG_DOSCROLL: 
		{
			CPoint p(0,0);  //dummy point;
			CModScrollView::OnMouseWheel(0, lParam, p);
		}
		break;


	default:
		return CModScrollView::OnModViewMsg(wParam, lParam);
	}
	return 0;
}

//rewbs.customKeys
void CViewPattern::CursorJump(DWORD distance, bool direction, bool snap)
{											  //up is true
	switch(snap)
	{
		case false: //no snap
			SetCurrentRow(m_nRow + ((int)(direction?-1:1))*distance, TRUE); 
			break;

		case true: //snap
			SetCurrentRow((((m_nRow+(int)(direction?-1:0))/distance)+(int)(direction?0:1))*distance, TRUE);
			break;
	}

}

LRESULT CViewPattern::OnCustomKeyMsg(WPARAM wParam, LPARAM lParam)
{
	if (wParam == kcNull)
		return NULL;
	
	CModDoc *pModDoc = GetDocument();	
	if (!pModDoc) return NULL;
	
	CSoundFile *pSndFile = pModDoc->GetSoundFile();	
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	switch(wParam)
	{
		case kcPrevInstrument:				OnPrevInstrument(); return wParam;
		case kcNextInstrument:				OnNextInstrument(); return wParam;
		case kcPrevOrder:					OnPrevOrder(); return wParam;
		case kcNextOrder:					OnNextOrder(); return wParam;
		case kcPatternPlayRow:				OnPatternStep(); return wParam;
		case kcPatternRecord:				OnPatternRecord(); return wParam;
		case kcCursorCopy:					OnCursorCopy(); return wParam;
		case kcCursorPaste:					OnCursorPaste(); return wParam;
		case kcChannelMute:					OnMuteChannel(true); return wParam;
		case kcChannelSolo:					OnSoloChannel(true); return wParam;
		case kcToggleChanMuteOnPatTransition: TogglePendingMute((m_dwCursor&0xFFFF)>>3); return wParam;
		case kcTransposeUp:					OnTransposeUp(); return wParam;
		case kcTransposeDown:				OnTransposeDown(); return wParam;
		case kcTransposeOctUp:				OnTransposeOctUp(); return wParam;
		case kcTransposeOctDown:			OnTransposeOctDown(); return wParam;
		case kcSelectColumn:				OnSelectCurrentColumn(); return wParam;
		case kcPatternAmplify:				OnPatternAmplify(); return wParam;
		case kcPatternSetInstrument:		OnSetSelInstrument(); return wParam;
		case kcPatternInterpolateNote:		OnInterpolateNote(); return wParam;
		case kcPatternInterpolateVol:		OnInterpolateVolume(); return wParam;
		case kcPatternInterpolateEffect:	OnInterpolateEffect(); return wParam;
		case kcPatternVisualizeEffect:		OnVisualizeEffect(); return wParam;
		case kcPatternOpenRandomizer:		OnOpenRandomizer(); return wParam;
		case kcPatternGrowSelection:		OnGrowSelection(); return wParam;
		case kcPatternShrinkSelection:		OnShrinkSelection(); return wParam;

		// Pattern navigation:
		case kcPatternJumpUph1Select:
		case kcPatternJumpUph1:			CursorJump(CMainFrame::m_nRowSpacing, true, false); return wParam;
		case kcPatternJumpDownh1Select:
		case kcPatternJumpDownh1:		CursorJump(CMainFrame::m_nRowSpacing, false, false);  return wParam;
		case kcPatternJumpUph2Select:
		case kcPatternJumpUph2:			CursorJump(CMainFrame::m_nRowSpacing2, true, false);  return wParam;
		case kcPatternJumpDownh2Select:
		case kcPatternJumpDownh2:		CursorJump(CMainFrame::m_nRowSpacing2, false, false);  return wParam;

		case kcPatternSnapUph1Select:
		case kcPatternSnapUph1:			CursorJump(CMainFrame::m_nRowSpacing, true, true); return wParam;
		case kcPatternSnapDownh1Select:
		case kcPatternSnapDownh1:		CursorJump(CMainFrame::m_nRowSpacing, false, true);  return wParam;
		case kcPatternSnapUph2Select:
		case kcPatternSnapUph2:			CursorJump(CMainFrame::m_nRowSpacing2, true, true);  return wParam;
		case kcPatternSnapDownh2Select:
		case kcPatternSnapDownh2:		CursorJump(CMainFrame::m_nRowSpacing2, false, true);  return wParam;
		
		case kcNavigateDownSelect:
		case kcNavigateDown:	SetCurrentRow(m_nRow+1, TRUE); return wParam;
		case kcNavigateUpSelect:
		case kcNavigateUp:		SetCurrentRow(m_nRow-1, TRUE); return wParam;
		
		case kcNavigateDownBySpacingSelect:
		case kcNavigateDownBySpacing:	SetCurrentRow(m_nRow+m_nSpacing, TRUE); return wParam;
		case kcNavigateUpBySpacingSelect:
		case kcNavigateUpBySpacing:		SetCurrentRow(m_nRow-m_nSpacing, TRUE); return wParam;
		
		case kcNavigateLeftSelect:
		case kcNavigateLeft:	if ((CMainFrame::m_dwPatternSetup & PATTERN_WRAP) && (!m_dwCursor))
									SetCurrentColumn((((pSndFile->m_nChannels-1) << 3) | 4));
								else
									SetCurrentColumn(m_dwCursor - 1);
								return wParam;
		case kcNavigateRightSelect:
		case kcNavigateRight:	if ((CMainFrame::m_dwPatternSetup & PATTERN_WRAP) && (m_dwCursor >= (((pSndFile->m_nChannels-1) << 3) | 4)))
									SetCurrentColumn(0);
								else
									SetCurrentColumn(m_dwCursor + 1);
								return wParam;
		case kcNavigateNextChanSelect:
		case kcNavigateNextChan:SetCurrentColumn(((((m_dwCursor >> 3) + 1) % pSndFile->m_nChannels) << 3) | (m_dwCursor & 0x07)); return wParam;
		case kcNavigatePrevChanSelect:
		case kcNavigatePrevChan:{if (m_dwCursor >> 3)
									SetCurrentColumn(((((m_dwCursor >> 3) - 1) % pSndFile->m_nChannels) << 3) | (m_dwCursor & 0x07));
								else
									SetCurrentColumn((m_dwCursor & 0x07) | ((pSndFile->m_nChannels-1) << 3));
								UINT n = (m_nRow << 16) | (m_dwCursor);
								SetCurSel(n, n);  return wParam;}

		case kcHomeHorizontalSelect:
		case kcHomeHorizontal:	if (m_dwCursor) SetCurrentColumn(0); 
								else if (m_nRow > 0) SetCurrentRow(0); 
								return wParam;
		case kcHomeVerticalSelect:
		case kcHomeVertical:	if (m_nRow > 0) SetCurrentRow(0); 
								else if (m_dwCursor) SetCurrentColumn(0);
								return wParam; 
		case kcHomeAbsoluteSelect:
		case kcHomeAbsolute:	if (m_dwCursor) SetCurrentColumn(0); if (m_nRow > 0) SetCurrentRow(0); return wParam; 

		case kcEndHorizontalSelect:
		case kcEndHorizontal:	if (m_dwCursor!=(((pSndFile->m_nChannels-1) << 3) | 4)) SetCurrentColumn(((pSndFile->m_nChannels-1) << 3) | 4);
								else if (m_nRow < pModDoc->GetPatternSize(m_nPattern) - 1) SetCurrentRow(pModDoc->GetPatternSize(m_nPattern) - 1);
								return wParam; 
		case kcEndVerticalSelect:
		case kcEndVertical:		if (m_nRow < pModDoc->GetPatternSize(m_nPattern) - 1) SetCurrentRow(pModDoc->GetPatternSize(m_nPattern) - 1); 
								else if (m_dwCursor!=(((pSndFile->m_nChannels-1) << 3) | 4)) SetCurrentColumn(((pSndFile->m_nChannels-1) << 3) | 4);
								return wParam;
		case kcEndAbsoluteSelect:
		case kcEndAbsolute:		SetCurrentColumn(((pSndFile->m_nChannels-1) << 3) | 4); if (m_nRow < pModDoc->GetPatternSize(m_nPattern) - 1) SetCurrentRow(pModDoc->GetPatternSize(m_nPattern) - 1); return wParam;

		case kcNextPattern:	{	UINT n = m_nPattern + 1;
								while ((n < MAX_PATTERNS) && (!pSndFile->Patterns[n])) n++;
								SetCurrentPattern((n < MAX_PATTERNS) ? n : 0);	
								return wParam; }
		case kcPrevPattern: {	UINT n = (m_nPattern) ? m_nPattern - 1 : MAX_PATTERNS-1;
								while ((n > 0) && (!pSndFile->Patterns[n])) n--;
								SetCurrentPattern(n);
								return wParam; }
		case kcSelectWithCopySelect:
		case kcSelectWithNav:
		case kcSelect:			if (!(m_dwStatus & PATSTATUS_DRAGNDROPEDIT)) m_dwStartSel = (m_nRow << 16) | m_dwCursor;
									m_dwStatus |= PATSTATUS_KEYDRAGSEL;	return wParam;
		case kcSelectOffWithCopySelect:
		case kcSelectOffWithNav:
		case kcSelectOff:		m_dwStatus &= ~PATSTATUS_KEYDRAGSEL; return wParam;
		case kcCopySelectWithSelect:
		case kcCopySelectWithNav:
		case kcCopySelect:		if (!(m_dwStatus & PATSTATUS_DRAGNDROPEDIT)) m_dwStartSel = (m_nRow << 16) | m_dwCursor;
									m_dwStatus |= PATSTATUS_CTRLDRAGSEL; return wParam;
		case kcCopySelectOffWithSelect:
		case kcCopySelectOffWithNav:
		case kcCopySelectOff:	m_dwStatus &= ~PATSTATUS_CTRLDRAGSEL; return wParam;
								
		case kcClearRow:		OnClearField(-1, false);	return wParam;
		case kcClearField:		OnClearField(m_dwCursor & 0x07, false);	return wParam;
		case kcClearFieldITStyle: OnClearField(m_dwCursor & 0x07, false, true);	return wParam;
		case kcClearRowStep:	OnClearField(-1, true);	return wParam;
		case kcClearFieldStep:	OnClearField(m_dwCursor & 0x07, true);	return wParam;
		case kcClearFieldStepITStyle:	OnClearField(m_dwCursor & 0x07, true, true);	return wParam;
		case kcDeleteRows:		OnDeleteRows(); return wParam;
		case kcDeleteAllRows:	DeleteRows(0, pSndFile->m_nChannels-1,1); return wParam;
		case kcInsertRow:		OnInsertRows(); return wParam;
		case kcInsertAllRows:	InsertRows(0, pSndFile->m_nChannels-1); return wParam;		

		case kcShowNoteProperties: ShowEditWindow();return wParam;
		case kcShowEditMenu:	{CPoint pt =	GetPointFromPosition((m_nRow << 16) | m_dwCursor);
								OnRButtonDown(0, pt); }
								return wParam; 
		case kcPatternGoto:		OnEditGoto(); return wParam;

		case kcNoteCut:			TempEnterNote(254, false); return wParam;
		case kcNoteCutOld:		TempEnterNote(254, true);  return wParam;
		case kcNoteOff:			TempEnterNote(255, false); return wParam;
		case kcNoteOffOld:		TempEnterNote(255, true);  return wParam;

		case kcEditUndo:		OnEditUndo(); return wParam;
		case kcEditFind:		OnEditFind(); return wParam;
		case kcEditFindNext:	OnEditFindNext(); return wParam;
		case kcEditCut:			OnEditCut(); return wParam;
		case kcEditCopy:		OnEditCopy(); return wParam;
		case kcCopyAndLoseSelection:
								{
								OnEditCopy(); 
								int sel = m_dwCursor | (m_nRow << 16);
								SetCurSel(sel, sel);
								return wParam;
								}
		case kcEditPaste:		OnEditPaste(); return wParam;
		case kcEditMixPaste:	OnEditMixPaste(); return wParam;
		case kcEditMixPasteITStyle:	OnEditMixPasteITStyle(); return wParam;
		case kcEditSelectAll:	OnEditSelectAll(); return wParam;
		case kcTogglePluginEditor: TogglePluginEditor((m_dwCursor & 0xFFFF) >> 3); return wParam;
		case kcToggleFollowSong: SendCtrlMessage(CTRLMSG_PAT_FOLLOWSONG); return wParam;
		case kcNewPattern:		 SendCtrlMessage(CTRLMSG_PAT_NEWPATTERN); return wParam;
		case kcSwitchToOrderList: OnSwitchToOrderList();

	}
	//Ranges:
	if (wParam>=kcVPStartNotes && wParam<=kcVPEndNotes)
	{
		TempEnterNote(wParam-kcVPStartNotes+1+pMainFrm->GetBaseOctave()*12);
		return wParam;
	}
	if (wParam>=kcVPStartChords && wParam<=kcVPEndChords)
	{
		TempEnterChord(wParam-kcVPStartChords+1+pMainFrm->GetBaseOctave()*12);
		return wParam;
	}

	if (wParam>=kcVPStartNoteStops && wParam<=kcVPEndNoteStops)
	{
		TempStopNote(wParam-kcVPStartNoteStops+1+pMainFrm->GetBaseOctave()*12);
		return wParam;
	}
	if (wParam>=kcVPStartChordStops && wParam<=kcVPEndChordStops)
	{
		TempStopChord(wParam-kcVPStartChordStops+1+pMainFrm->GetBaseOctave()*12);
		return wParam;
	}

	if (wParam>=kcSetSpacing0 && wParam<kcSetSpacing9)
	{
		SetSpacing(wParam - kcSetSpacing0); 
		return wParam;
	}

	if (wParam>=kcSetIns0 && wParam<=kcSetIns9)
	{
		if ((m_dwStatus & PATSTATUS_RECORD))
			TempEnterIns(wParam-kcSetIns0);
		return wParam;
	}

	if (wParam>=kcSetOctave0 && wParam<=kcSetOctave9)
	{
		if ((m_dwStatus & PATSTATUS_RECORD))
			TempEnterOctave(wParam-kcSetOctave0);
		return wParam;
	}
	if (wParam>=kcSetVolumeStart && wParam<=kcSetVolumeEnd)
	{
		if ((m_dwStatus & PATSTATUS_RECORD))
			TempEnterVol(wParam-kcSetVolumeStart);
		return wParam;
	}
	if (wParam>=kcSetFXStart && wParam<=kcSetFXEnd)
	{
		if ((m_dwStatus & PATSTATUS_RECORD))
			TempEnterFX(wParam-kcSetFXStart+1);
		return wParam;
	}
	if (wParam>=kcSetFXParam0 && wParam<=kcSetFXParamF)
	{
		if ((m_dwStatus & PATSTATUS_RECORD))
			TempEnterFXparam(wParam-kcSetFXParam0);
		return wParam;
	}

	return NULL;
}

void CViewPattern::TempEnterVol(int v)
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (pMainFrm))
	{	

		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
	
		// -- Work out where to put the new data
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;

		UINT volcmd = p->volcmd;
		UINT vol = p->vol;
		if ((v >= 0) && (v <= 9))
		{
			vol = ((vol * 10) + v) % 100;
			if (!volcmd) volcmd = VOLCMD_VOLUME;
		} 
		else
			switch(v+kcSetVolumeStart)
			{
			case kcSetVolumeVol:			volcmd = VOLCMD_VOLUME; break;
			case kcSetVolumePan:			volcmd = VOLCMD_PANNING; break;
			case kcSetVolumeVolSlideUp:		volcmd = VOLCMD_VOLSLIDEUP; break;
			case kcSetVolumeVolSlideDown:	volcmd = VOLCMD_VOLSLIDEDOWN; break;
			case kcSetVolumeFineVolUp:		volcmd = VOLCMD_FINEVOLUP; break;
			case kcSetVolumeFineVolDown:	volcmd = VOLCMD_FINEVOLDOWN; break;
			case kcSetVolumeVibratoSpd:		volcmd = VOLCMD_VIBRATOSPEED; break;
			case kcSetVolumeVibrato:		volcmd = VOLCMD_VIBRATO; break;
			case kcSetVolumeXMPanLeft:		if (pSndFile->m_nType & MOD_TYPE_XM) volcmd = VOLCMD_PANSLIDELEFT; break;
			case kcSetVolumeXMPanRight:		if (pSndFile->m_nType & MOD_TYPE_XM) volcmd = VOLCMD_PANSLIDERIGHT; break;
			case kcSetVolumePortamento:		volcmd = VOLCMD_TONEPORTAMENTO; break;
			case kcSetVolumeITPortaUp:		if (pSndFile->m_nType & MOD_TYPE_IT) volcmd = VOLCMD_PORTAUP; break;
			case kcSetVolumeITPortaDown:	if (pSndFile->m_nType & MOD_TYPE_IT) volcmd = VOLCMD_PORTADOWN; break;
			case kcSetVolumeITVelocity:		if (pSndFile->m_nType & MOD_TYPE_IT) volcmd = VOLCMD_VELOCITY; break;	//rewbs.velocity
			case kcSetVolumeITOffset:		if (pSndFile->m_nType & MOD_TYPE_IT) volcmd = VOLCMD_OFFSET; break;		//rewbs.volOff
			}
		if ((pSndFile->m_nType & MOD_TYPE_MOD) && (volcmd > VOLCMD_PANNING)) volcmd = vol = 0;

		UINT max = 64;
		if (volcmd > VOLCMD_PANNING)
		{
			max = (pSndFile->m_nType == MOD_TYPE_XM) ? 0x0F : 9;
		}

		if (vol > max) vol %= 10;
		p->volcmd = volcmd;
		p->vol = vol;

		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))
			{
				pModDoc->SetModified();
				InvalidateArea(sel, sel+5);
				UpdateIndicator();
			}
		}
		else
		{
			// recording disabled
			*p = oldcmd;
		}
	}
}

void CViewPattern::SetSpacing(int n)
{
	if (n != m_nSpacing)
	{
		m_nSpacing = n;
		PostCtrlMessage(CTRLMSG_SETSPACING, m_nSpacing);
	}
}

void CViewPattern::TempEnterFX(int c)
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (pMainFrm))
	{	
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		PrepareUndo(m_dwBeginSel, m_dwEndSel);

		// -- Work out where to put the new data
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;

		LPCSTR lpcmd = (pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM)) ? gszModCommands : gszS3mCommands;
		if (c)
		{
			if ((c == m_cmdOld.command) && (!p->param) && (!p->command)) p->param = m_cmdOld.param;
			else m_cmdOld.param = 0;
			m_cmdOld.command = c;
		}
		p->command = c;
		
		// Check for MOD/XM Speed/Tempo command
		if ((pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))
		 && ((p->command == CMD_SPEED) || (p->command == CMD_TEMPO)))
		{
			UINT maxspd = (pSndFile->m_nType & MOD_TYPE_XM) ? 0x1F : 0x20;
			p->command = (p->param <= maxspd) ? CMD_SPEED : CMD_TEMPO;
		}

		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))
			{
				pModDoc->SetModified();
				InvalidateArea(sel, sel+5);
				UpdateIndicator();
			}
		}
	}	// end if mainframe & moddoc exist
}

void CViewPattern::TempEnterFXparam(int v)
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (pMainFrm))
	{	
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		PrepareUndo(m_dwBeginSel, m_dwEndSel);

		// -- Work out where to put the new data
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;

//		if (v >= 0 && v <= 9)
//		{
			p->param = (p->param << 4) | v;
			if (p->command == m_cmdOld.command) m_cmdOld.param = p->param;
/*		} 
		else if (v >= 10 && v <= 15)
		{
			p->param = (p->param << 4) | (v + 0x0A);
			if (p->command == m_cmdOld.command) m_cmdOld.param = p->param;
		}
*/
		// Check for MOD/XM Speed/Tempo command
		if ((pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))
		 && ((p->command == CMD_SPEED) || (p->command == CMD_TEMPO)))
		{
			UINT maxspd = (pSndFile->m_nType & MOD_TYPE_XM) ? 0x1F : 0x20;
			p->command = (p->param <= maxspd) ? CMD_SPEED : CMD_TEMPO;
		}

		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))
			{
				pModDoc->SetModified();
				InvalidateArea(sel, sel+5);
				UpdateIndicator();
			}
		}
	}
}
void CViewPattern::TempStopNote(int note, bool fromMidi)
//------------------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	bool isSplit = (note<m_nSplitNote);
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if (pModDoc)
	{
		UINT ins = 0;
		if (isSplit)
		{
			ins = m_nSplitInstrument;
			if (m_bOctaveLink)		  note += 12*(m_nOctaveModifier-9);
			if (note>120 && note<254) note=120;
			if (note<0)				  note=1;
		}
		if (!ins)    ins = GetCurrentInstrument();
		if (!ins)	 ins = m_nFoundInstrument;
		pModDoc->NoteOff(note, TRUE, ins, (m_dwCursor & 0xFFFF) >> 3);
	}
	
	//Enter note off in pattern?
	if ((note < 1) || (note > 120))
		return;
	if ((m_dwCursor & 7) > 1 && !fromMidi)
		return;
	if (!pModDoc || !pMainFrm || !(m_dwStatus & PATSTATUS_RECORD))
		return;

	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
	UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
	PrepareUndo(m_dwBeginSel, m_dwEndSel);

	int releaseChan = -1;

	BYTE* activeNoteMap = isSplit?splitActiveNoteChannel:activeNoteChannel;
	releaseChan = activeNoteMap[note];
	if (releaseChan<0 || releaseChan>pSndFile->m_nChannels)
		releaseChan = nChn;

	activeNoteMap[note] = 0xFF;	//unlock channel

	if  (!(CMainFrame::m_dwPatternSetup&PATTERN_KBDNOTEOFF || fromMidi))
		return;
	
	//Work out where to put the note off
	prowbase = p + m_nRow * pSndFile->m_nChannels;
	if (releaseChan>=0 && releaseChan<pSndFile->m_nChannels) 
		p=prowbase+releaseChan;
	else 
		p=prowbase+nChn;

	//don't overwrite:
	if (p->note || p->instr || p->volcmd) {
		//if there's a note in the current location and the song is playing and following,
		//the user probably just tapped the key - let's try the next row down.
		if (p->note==note && (m_dwStatus&PATSTATUS_FOLLOWSONG) && !(pSndFile->IsPaused())) {
			p=pSndFile->Patterns[m_nPattern]+(m_nRow+1)*pSndFile->m_nChannels+releaseChan;
			if (p->note || p->instr || p->volcmd) {
				return;
			}
		} else {
			return;	
		}
	}		

	//Enter note off
	p->note		= 0xFF;
    p->instr	= 0;
	p->volcmd	= 0;
	p->vol		= 0;
//	p->command	= 0;
//	p->param	= 0;

	pModDoc->SetModified();
	DWORD sel = (m_nRow << 16) | (releaseChan << 3);
	InvalidateArea(sel, sel+5);
	UpdateIndicator();

	return;
	
}

void CViewPattern::TempStopChord(int note)
//---------------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	bool isSplit = (note<m_nSplitNote);
	if (pModDoc)
	{
		UINT ins = 0;
		if (isSplit)
		{
			ins = m_nSplitInstrument;
			if (m_bOctaveLink)		  note += 12*(m_nOctaveModifier-9);
			if (note>120 && note<254) note=120;
			if (note<0)				  note=1;
		}
		if (!ins)    ins = GetCurrentInstrument();
		if (!ins)	 ins = m_nFoundInstrument;

		m_dwStatus &= ~PATSTATUS_CHORDPLAYING;
		pModDoc->NoteOff(0, TRUE, ins, m_dwCursor & 0xFFFF);
	}
	
	//Enter note off in pattern?
	if ((note < 1) || (note > 120))
		return;
	if ((m_dwCursor & 7) > 1)
		return;
	if (!pModDoc || !pMainFrm || !(m_dwStatus & PATSTATUS_RECORD))
		return;

	CSoundFile *pSndFile = pModDoc->GetSoundFile();
	MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
	UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
	PrepareUndo(m_dwBeginSel, m_dwEndSel);

	int releaseChan = -1;

	BYTE* activeNoteMap = isSplit?splitActiveNoteChannel:activeNoteChannel;
	releaseChan = activeNoteMap[note];
	if (releaseChan<0 || releaseChan>pSndFile->m_nChannels)
		releaseChan = nChn;

	activeNoteMap[note] = 0xFF;	//unlock channel

	if  (!(CMainFrame::m_dwPatternSetup&PATTERN_KBDNOTEOFF ))
		return;
	
	//Work out where to put the note off
	prowbase = p + m_nRow * pSndFile->m_nChannels;
	p=prowbase+nChn;

	//don't overwrite:
	if (p->note)
	{
		//if there's a note in the current location and the song is playing and following,
		//the user probably just tapped the key - let's try the nex row down.
		if (p->note==note && (m_dwStatus&PATSTATUS_FOLLOWSONG) && !(pSndFile->IsPaused()))
		{
			p=pSndFile->Patterns[m_nPattern]+(m_nRow+1)*pSndFile->m_nChannels+nChn;
			if (p->note)
				return;
		}
		else
		{
			return;	
		}

	}	
		
	p->note		= 0xFF;
    p->instr	= 0;
	p->volcmd	= 0;
	p->vol		= 0;
	p->command	= 0;
	p->param	= 0;

	pModDoc->SetModified();
	DWORD sel = (m_nRow << 16) | (releaseChan << 3);
	InvalidateArea(sel, sel+5);
	UpdateIndicator();

	return;
}

void CViewPattern::TempEnterOctave(int val)
//---------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (pMainFrm))
	{	

		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		PrepareUndo(m_dwBeginSel, m_dwEndSel);

		// -- Work out where to put the new oct
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;
		if (oldcmd.note)
			TempEnterNote(((oldcmd.note-1)%12)+val*12+1);
	}

}

void CViewPattern::TempEnterIns(int val)
//---------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();

	if ((pModDoc) && (pMainFrm))
	{	

		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		PrepareUndo(m_dwBeginSel, m_dwEndSel);

		// -- Work out where to put the new ins
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;

		UINT instr  = p->instr;
		instr = ((instr * 10) + val) % 1000;
		if (instr > MAX_INSTRUMENTS) instr = instr % 100;
		if ((pSndFile->m_nSamples < 100) && (pSndFile->m_nInstruments < 100) && (instr >= 100)) instr = instr % 100;
		p->instr = instr;

		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))
			{
				pModDoc->SetModified();
				InvalidateArea(sel, sel+5);
				UpdateIndicator();
			}
		}
		else
		{
			// recording disabled
			*p = oldcmd;
		}
	}
}

void CViewPattern::TempEnterNote(int note, bool oldStyle, int vol)
//--------------------------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();
	UINT nPlayChord = 0;
	bool isSplit;

	if ((pModDoc) && (pMainFrm))
	{	
		UINT nRow = m_nRow;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// a copy of the command we are about to overwrite
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		BYTE recordGroup = pModDoc->IsChannelRecord(nChn);
		UINT nPlayIns = 0;
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		if (note>120 && note<254) note=120;

		// -- Chord autodetection: step back if we just entered a note
		if ((m_dwStatus & PATSTATUS_RECORD) && (recordGroup) &&
			((pMainFrm->GetFollowSong(pModDoc) != m_hWnd) || (pSndFile->IsPaused())  
			                                              || (!(m_dwStatus & PATSTATUS_FOLLOWSONG))))
		{
			if ((m_nSpacing > 0) && (m_nSpacing <= MAX_SPACING))
			{
				if ((timeGetTime() - m_dwLastNoteEntryTime < CMainFrame::gnAutoChordWaitTime) 
					&& (nRow>=m_nSpacing) && (!m_bLastNoteEntryBlocked))
					nRow -= m_nSpacing;
			}
		}
		m_dwLastNoteEntryTime=timeGetTime();

		// -- Work out where to put the new note
		prowbase = p + nRow * pSndFile->m_nChannels;
	   	p = prowbase + nChn;
	
		oldcmd = *p;	//take backup copy of the command we're about to overwrite

		// -- write note and instrument data
		isSplit = HandleSplit(p, note);

		// -- write vol data
		if (!isSplit && vol>=0 && vol<=64)
		{
			p->volcmd=VOLCMD_VOLUME;
			p->vol = vol;
		}

		// -- old style note cut/off: erase instrument number
        if (oldStyle && ((p->note==254) || (p->note==255)))
			p->instr=0;

		// -- if recording, handle post note entry behaviour (move cursor etc..)
		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND))) //has it really changed?
			{
				pModDoc->SetModified();
				InvalidateArea(sel, sel+5);
				UpdateIndicator();
			}

			//Move cursor down if follow song off or song is paused
			if ((pMainFrm->GetFollowSong(pModDoc) != m_hWnd) || (pSndFile->IsPaused())
					|| (!(m_dwStatus & PATSTATUS_FOLLOWSONG)))
			{
				if ((m_nSpacing > 0) && (m_nSpacing <= MAX_SPACING)) {
					if (nRow+m_nSpacing<pSndFile->PatternSize[m_nPattern]) {
						SetCurrentRow(nRow+m_nSpacing);
						m_bLastNoteEntryBlocked=false;
					} else {
						m_bLastNoteEntryBlocked=true;  // if the cursor is block by the end of the pattern here,
					}								   // we must remember to not step back should the next note form a chord.
						
				}
				DWORD sel = m_dwCursor | (m_nRow << 16);
				SetCurSel(sel, sel);
			}
			
			BYTE* activeNoteMap = isSplit?splitActiveNoteChannel:activeNoteChannel;
			if (p->note <= 120)
				activeNoteMap[p->note]=nChn; 
	
			//Move to next channel if required
			if (recordGroup)
			{
				bool channelLocked;

				UINT n = nChn;
				for (UINT i=0; i<pSndFile->m_nChannels; i++)
				{
					if (++n > pSndFile->m_nChannels) n = 0; //loop around
					
					channelLocked = false; 
					for (int k=0; k<120; k++)
					{
						if (activeNoteChannel[k]==n  || splitActiveNoteChannel[k]==n)
						{
							channelLocked = true;
							break;
						}
					}

					if (pModDoc->IsChannelRecord(n)==recordGroup && !channelLocked)
					{
						SetCurrentColumn(n<<3);
						break;
					}
				}
			}
		}

		// -- play note
		if ((CMainFrame::m_dwPatternSetup & PATTERN_PLAYNEWNOTE) || (!(m_dwStatus & PATSTATUS_RECORD)))
		{
			if (p->instr) nPlayIns = p->instr;
		
			else if ((!p->instr) && (p->note < 128))
			{
				MODCOMMAND *search = p;
				UINT srow = nRow;
				while (srow > 0)
				{
					srow--;
					search -= pSndFile->m_nChannels;
					if (search->instr)
					{
						nPlayIns = search->instr;
						m_nFoundInstrument = nPlayIns;  //used to figure out which instrument to stop on key release.
						break;
					}
				}
			}
			BOOL bNotPlaying = ((pMainFrm->GetModPlaying() == pModDoc) && (pMainFrm->IsPlaying())) ? FALSE : TRUE;
			pModDoc->PlayNote(p->note, nPlayIns, 0, bNotPlaying, -1, 0, 0, nChn);	//rewbs.vstiLive - added extra args
/*			for (UINT kplchrd=0; kplchrd<nPlayChord; kplchrd++)
			{
				if (chordplaylist[kplchrd])
				{
					pModDoc->PlayNote(chordplaylist[kplchrd], nPlayIns, 0, FALSE, -1, 0, 0, nChn);	//rewbs.vstiLive - 	- added extra args
					m_dwStatus |= PATSTATUS_CHORDPLAYING;
				}
			}
*/
		}

		//-- if not recording, restore old command.
		if (!(m_dwStatus & PATSTATUS_RECORD))
		{	
			*p = oldcmd;
		}
	}

}

void CViewPattern::TempEnterChord(int note)
//---------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();
	UINT nPlayChord = 0;
	BYTE chordplaylist[3];

	if ((pModDoc) && (pMainFrm))
	{		
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern], *prowbase;
		MODCOMMAND oldcmd;		// This is the command we are about to overwrite
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		UINT nPlayIns = 0;
		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		bool isSplit;

		// -- Work out where to put the new note
		prowbase = p + m_nRow * pSndFile->m_nChannels;
		p = prowbase + nChn;
		oldcmd = *p;

		// -- establish note data
		isSplit = HandleSplit(p, note);
	
		PMPTCHORD pChords = pMainFrm->GetChords();
		UINT baseoctave = pMainFrm->GetBaseOctave();
		UINT nchord = note - baseoctave * 12 - 1;
		UINT nNote;
		if (nchord < 3*12)
		{
			UINT nchordnote;
            if (isSplit)
				nchordnote = pChords[nchord].key + baseoctave*(p->note%12) + 1;
			else
				nchordnote = pChords[nchord].key + baseoctave*12 + 1;
			if (nchordnote <= 120)
			{
				UINT nchordch = nChn, nchno = 0;
				nNote = nchordnote;
				p->note = nNote;
				BYTE recordGroup, currentRecordGroup = 0;
				
				recordGroup = pModDoc->IsChannelRecord(nChn);

				for (UINT kchrd=1; kchrd<pSndFile->m_nChannels; kchrd++)
				{
					if ((nchno > 2) || (!pChords[nchord].notes[nchno])) break;
					if (++nchordch >= pSndFile->m_nChannels) nchordch = 0;

					currentRecordGroup = pModDoc->IsChannelRecord(nchordch);
					if (!recordGroup)
						recordGroup = currentRecordGroup;  //record group found

					UINT n = ((nchordnote-1)/12) * 12 + pChords[nchord].notes[nchno];
					if (m_dwStatus & PATSTATUS_RECORD)
					{
						if ((nchordch != nChn) && recordGroup && (currentRecordGroup == recordGroup) && (n <= 120))
						{
							prowbase[nchordch].note = n;
							if (p->instr) prowbase[nchordch].instr = p->instr;
							nchno++;
							if (CMainFrame::m_dwPatternSetup & PATTERN_PLAYNEWNOTE)
							{
								if ((n) && (n<=120)) chordplaylist[nPlayChord++] = n;
							}
						}
					} else
					{
						nchno++;
						if ((n) && (n<=120)) chordplaylist[nPlayChord++] = n;
					}
				}
			}
		}

		// -- write notedata
		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))
			{
				pModDoc->SetModified();
				InvalidateRow();
				UpdateIndicator();
			}
			if (((pMainFrm->GetFollowSong(pModDoc) != m_hWnd) || (pSndFile->IsPaused())
			 || (!(m_dwStatus & PATSTATUS_FOLLOWSONG))))
			{
				if ((m_nSpacing > 0) && (m_nSpacing <= MAX_SPACING)) 
					SetCurrentRow(m_nRow+m_nSpacing);
				DWORD sel = m_dwCursor | (m_nRow << 16);
				SetCurSel(sel, sel);

			}
		} else
		{
			// recording disabled
			*p = oldcmd;
		}

		// -- play note
		if ((CMainFrame::m_dwPatternSetup & PATTERN_PLAYNEWNOTE) || (!(m_dwStatus & PATSTATUS_RECORD)))
		{
			if (p->instr) nPlayIns = p->instr;
		
			else if ((!p->instr) && (p->note < 128))
			{
				MODCOMMAND *search = p;
				UINT srow = m_nRow;
				while (srow > 0)
				{
					srow--;
					search -= pSndFile->m_nChannels;
					if (search->instr)
					{
						nPlayIns = search->instr;
						m_nFoundInstrument = nPlayIns;  //used to figure out which instrument to stop on key release.
						break;
					}
				}
			}
			BOOL bNotPlaying = ((pMainFrm->GetModPlaying() == pModDoc) && (pMainFrm->IsPlaying())) ? FALSE : TRUE;
			pModDoc->PlayNote(p->note, nPlayIns, 0, bNotPlaying, -1, 0, 0, nChn);	//rewbs.vstiLive - added extra args
			for (UINT kplchrd=0; kplchrd<nPlayChord; kplchrd++)
			{
				if (chordplaylist[kplchrd])
				{
					pModDoc->PlayNote(chordplaylist[kplchrd], nPlayIns, 0, FALSE, -1, 0, 0, nChn);	//rewbs.vstiLive - 	- added extra args
					m_dwStatus |= PATSTATUS_CHORDPLAYING;
				}
			}
		} // end play note
	} // end mainframe and moddoc exist
}


void CViewPattern::OnClearField(int field, bool step, bool ITStyle)
//--------------------------------------------------
{
 	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();
	
	if ((pModDoc) && (pMainFrm))
	{
        //if we have a selection we want to do something different
		if (m_dwBeginSel!=m_dwEndSel)
		{	
			OnClearSelection(ITStyle);
			return;
		}

		PrepareUndo(m_dwBeginSel, m_dwEndSel);
		UINT nChn = (m_dwCursor & 0xFFFF) >> 3;
		UINT nCursor = m_dwCursor & 0x07;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		MODCOMMAND *p = pSndFile->Patterns[m_nPattern] +  m_nRow*pSndFile->m_nChannels + nChn;
		MODCOMMAND oldcmd = *p;

		switch(field)
		{
			case 0:	p->note = 0; if (ITStyle) p->instr = 0;  break;		//Note
			case 1:	p->instr = 0; break;				//instr
			case 2:	p->vol = 0; p->volcmd = 0; break;	//Vol
			case 3:	p->command = 0;	break;				//Effect							
			case 4:	p->param = 0; break;				//Param
			default: p->note = 0;						//If not specified, delete them all! :)
					 p->instr = 0;
					 p->vol = 0; p->volcmd = 0;
					 p->command = 0;
					 p->param = 0;
		}			

		if (m_dwStatus & PATSTATUS_RECORD)
		{
			DWORD sel = (m_nRow << 16) | m_dwCursor;
			SetCurSel(sel, sel);
			sel &= ~7;
			if (memcmp(&oldcmd, p, sizeof(MODCOMMAND)))	
			{
				pModDoc->SetModified();
				InvalidateRow();
				UpdateIndicator();
			}
			if (step && (((pMainFrm->GetFollowSong(pModDoc) != m_hWnd) || (pSndFile->IsPaused())
			 || (!(m_dwStatus & PATSTATUS_FOLLOWSONG)))))
			{
				if ((m_nSpacing > 0) && (m_nSpacing <= MAX_SPACING)) 
					SetCurrentRow(m_nRow+m_nSpacing);
				DWORD sel = m_dwCursor | (m_nRow << 16);
				SetCurSel(sel, sel);
			}
		} else
		{
			*p = oldcmd;
		}
	}
}



void CViewPattern::OnInitMenu(CMenu* pMenu)
{
	CModScrollView::OnInitMenu(pMenu);

	//rewbs: ensure modifiers are reset when we go into menu
/*	m_dwStatus &= ~PATSTATUS_KEYDRAGSEL;	
	m_dwStatus &= ~PATSTATUS_CTRLDRAGSEL;
	CMainFrame::GetMainFrame()->GetInputHandler()->SetModifierMask(0);
*/	//end rewbs

}

void CViewPattern::TogglePluginEditor(int chan)
{
	CModDoc *pModDoc = GetDocument(); if (!pModDoc) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile(); if (!pSndFile) return;

	int plug = pSndFile->ChnSettings[chan].nMixPlugin;
	if (plug > 0)
		pModDoc->TogglePluginEditor(plug-1);
	
	return;
}

void CViewPattern::OnSelectPlugin(UINT nID)
{
	CModDoc *pModDoc = GetDocument(); if (!pModDoc) return;
	CSoundFile *pSndFile = pModDoc->GetSoundFile(); if (!pSndFile) return;

	if (m_nMenuOnChan)
	{
		int newPlug = nID-ID_PLUGSELECT;
		if (newPlug != pSndFile->ChnSettings[m_nMenuOnChan-1].nMixPlugin) {
			pSndFile->ChnSettings[m_nMenuOnChan-1].nMixPlugin = newPlug;
			pModDoc->SetModified();
			InvalidateChannelsHeaders();
		}
	}
}

//rewbs.merge
bool CViewPattern::HandleSplit(MODCOMMAND* p, int note)
//-----------------------------------------------------
{
	if (note>=m_nSplitNote)
	{
		p->note = note;	
		UINT nins = GetCurrentInstrument();
		if (nins) 
			p->instr = nins;
		return false;
	}
	else
	{
		if (m_nSplitInstrument)
			p->instr = m_nSplitInstrument;
		else
			p->instr = GetCurrentInstrument();
		if (m_nSplitVolume)
		{
			p->volcmd=VOLCMD_VOLUME;
			p->vol = m_nSplitVolume;
		}
		if (m_bOctaveLink)
			note += 12*(m_nOctaveModifier-9);
		if (note>120 && note<254) note=120;
		if (note<0) note=1;

		p->note = note;	
		return true;
	}
}

bool CViewPattern::BuildPluginCtxMenu(HMENU hMenu, UINT nChn, CSoundFile* pSndFile)
//---------------------------------------------------------------------------------
{
	CHAR s[512];
	bool itemFound;
	for (UINT plug=0; plug<=MAX_MIXPLUGINS; plug++)	{
		
		itemFound=false;
		s[0] = 0;

		if (!plug) { 
			strcpy(s, "No plugin");
			itemFound=true;
		} else	{
			PSNDMIXPLUGIN p = &(pSndFile->m_MixPlugins[plug-1]);
			if (p->Info.szLibraryName[0]) {
				wsprintf(s, "FX%d: %s", plug, p->Info.szName);
				itemFound=true;
			}
		}

		if (itemFound){
			m_nMenuOnChan=nChn+1;
			if (plug == pSndFile->ChnSettings[nChn].nMixPlugin) {
				AppendMenu(hMenu, (MF_STRING|MF_CHECKED), ID_PLUGSELECT+plug, s);
			} else {
				AppendMenu(hMenu, MF_STRING, ID_PLUGSELECT+plug, s);
			}
		}
	}
	return true;
}

bool CViewPattern::BuildSoloMuteCtxMenu(HMENU hMenu, CInputHandler* ih, UINT nChn, CSoundFile* pSndFile)
//------------------------------------------------------------------------------------------------------
{
	AppendMenu(hMenu, (pSndFile->ChnSettings[nChn].dwFlags & CHN_MUTE) ? 
					   (MF_STRING|MF_CHECKED) : MF_STRING, ID_PATTERN_MUTE, 
					   "Mute Channel\t" + ih->GetKeyTextFromCommand(kcChannelMute));
	BOOL b, bAll;
	b = FALSE;
	bAll = FALSE;
	for (UINT i=0; i<pSndFile->m_nChannels; i++) {
		if (i != nChn) {
			if (!(pSndFile->ChnSettings[i].dwFlags & CHN_MUTE)) b = TRUE;
		} else {
			if (pSndFile->ChnSettings[i].dwFlags & CHN_MUTE) b = TRUE;
		}
		if (pSndFile->ChnSettings[i].dwFlags & CHN_MUTE) bAll = TRUE;
	}
	if (b) AppendMenu(hMenu, MF_STRING, ID_PATTERN_SOLO, "Solo Channel\t" + ih->GetKeyTextFromCommand(kcChannelSolo));
	if (bAll) AppendMenu(hMenu, MF_STRING, ID_PATTERN_UNMUTEALL, "Unmute All");

	AppendMenu(hMenu, 
			pSndFile->m_bChannelMuteTogglePending[nChn] ? 
					(MF_STRING|MF_CHECKED) : MF_STRING,
			 ID_PATTERN_TRANSITIONMUTE,
			(pSndFile->ChnSettings[nChn].dwFlags & CHN_MUTE) ?
			"Unmute on Transition\t" + ih->GetKeyTextFromCommand(kcToggleChanMuteOnPatTransition) :
			"Mute on Transition\t" + ih->GetKeyTextFromCommand(kcToggleChanMuteOnPatTransition));

	return true;
}

bool CViewPattern::BuildRecordCtxMenu(HMENU hMenu, UINT nChn, CModDoc* pModDoc)
//-----------------------------------------------------------------------------
{
	AppendMenu(hMenu, pModDoc->IsChannelRecord1(nChn) ? (MF_STRING|MF_CHECKED) : MF_STRING, ID_EDIT_RECSELECT, "Record select");
	AppendMenu(hMenu, pModDoc->IsChannelRecord2(nChn) ? (MF_STRING|MF_CHECKED) : MF_STRING, ID_EDIT_SPLITRECSELECT, "Split Record select");
	return true;
}



bool CViewPattern::BuildRowInsDelCtxMenu(HMENU hMenu, CInputHandler* ih)
//----------------------------------------------------------------------
{
	CString label = "";
	if (GetSelectionStartRow() != GetSelectionEndRow()) {
		label = "Rows";
	} else { 
		label = "Row";
	}

	AppendMenu(hMenu, MF_STRING, ID_PATTERN_INSERTROW, "Insert " + label + "\t" + ih->GetKeyTextFromCommand(kcInsertRow));
	AppendMenu(hMenu, MF_STRING, ID_PATTERN_DELETEROW, "Delete " + label + "\t" + ih->GetKeyTextFromCommand(kcDeleteRows) );
	return true;
}

bool CViewPattern::BuildSelectionCtxMenu(HMENU hMenu, CInputHandler* ih)
//----------------------------------------------------------------------
{
	AppendMenu(hMenu, MF_STRING, ID_EDIT_SELECTCOLUMN, "Select Column\t" + ih->GetKeyTextFromCommand(kcSelectColumn));
	AppendMenu(hMenu, MF_STRING, ID_EDIT_SELECT_ALL, "Select Pattern\t" + ih->GetKeyTextFromCommand(kcEditSelectAll));
	return true;
}

bool CViewPattern::BuildGrowShrinkCtxMenu(HMENU hMenu, CInputHandler* ih)
//----------------------------------------------------------------------
{
	AppendMenu(hMenu, MF_STRING, ID_GROW_SELECTION, "Grow selection\t" + ih->GetKeyTextFromCommand(kcPatternGrowSelection));
	AppendMenu(hMenu, MF_STRING, ID_SHRINK_SELECTION, "Shrink selection\t" + ih->GetKeyTextFromCommand(kcPatternShrinkSelection));
	return true;
}

bool CViewPattern::BuildNoteInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile)
//----------------------------------------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = MF_GRAYED;

	UINT startRow = GetSelectionStartRow();
	UINT endRow   = GetSelectionEndRow();
	
	if ((startRow != endRow) && ListChansWhereColSelected(NOTE_COLUMN, validChans)>0) {
		for (int valChnIdx=0; valChnIdx<validChans.GetCount(); valChnIdx++) {
			if (IsInterpolationPossible(startRow, endRow, 
									    validChans[valChnIdx], NOTE_COLUMN, pSndFile)) {
				greyed=0;	//Can do interpolation.
				break;
			}
		}

	}
	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_INTERPOLATE_NOTE, "Interpolate Note\t" + ih->GetKeyTextFromCommand(kcPatternInterpolateNote));
		return true;
	}
	return false;
}

bool CViewPattern::BuildVolColInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile)
//------------------------------------------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = MF_GRAYED;

	UINT startRow = GetSelectionStartRow();
	UINT endRow   = GetSelectionEndRow();
	
	if ((startRow != endRow) && ListChansWhereColSelected(VOL_COLUMN, validChans)>0) {
		for (int valChnIdx=0; valChnIdx<validChans.GetCount(); valChnIdx++) {
			if (IsInterpolationPossible(startRow, endRow, 
									    validChans[valChnIdx], VOL_COLUMN, pSndFile)) {
				greyed=0;	//Can do interpolation.
				break;
			}
		}
	}
	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_INTERPOLATE_VOLUME, "Interpolate Vol\t" + ih->GetKeyTextFromCommand(kcPatternInterpolateVol));
		return true;
	}
	return false;
}


bool CViewPattern::BuildEffectInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile)
//------------------------------------------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = MF_GRAYED;

	UINT startRow = GetSelectionStartRow();
	UINT endRow   = GetSelectionEndRow();
	
	if ((startRow != endRow) && ListChansWhereColSelected(EFFECT_COLUMN, validChans)>0) {
		for (int valChnIdx=0; valChnIdx<validChans.GetCount(); valChnIdx++) {
			if  (IsInterpolationPossible(startRow, endRow, validChans[valChnIdx], EFFECT_COLUMN, pSndFile)) {
				greyed=0;	//Can do interpolation.
				break;
			}
		}
	}

	if ((startRow != endRow) && ListChansWhereColSelected(PARAM_COLUMN, validChans)>0) {
		for (int valChnIdx=0; valChnIdx<validChans.GetCount(); valChnIdx++) {
			if  (IsInterpolationPossible(startRow, endRow, validChans[valChnIdx], EFFECT_COLUMN, pSndFile)) {
				greyed=0;	//Can do interpolation.
				break;
			}
		}
	}


	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_INTERPOLATE_EFFECT, "Interpolate Effect\t" + ih->GetKeyTextFromCommand(kcPatternInterpolateEffect));
		return true;
	}
	return false;
}

bool CViewPattern::BuildEditCtxMenu(HMENU hMenu, CInputHandler* ih, CModDoc* pModDoc)
//-----------------------------------------------------------------------------------
{
	AppendMenu(hMenu, MF_STRING, ID_EDIT_CUT, "Cut\t" + ih->GetKeyTextFromCommand(kcEditCut));
	AppendMenu(hMenu, MF_STRING, ID_EDIT_COPY, "Copy\t" + ih->GetKeyTextFromCommand(kcEditCopy));
	AppendMenu(hMenu, MF_STRING, ID_EDIT_PASTE, "Paste\t" + ih->GetKeyTextFromCommand(kcEditPaste));
	AppendMenu(hMenu, MF_STRING, ID_EDIT_MIXPASTE, "Mix Paste\t" + ih->GetKeyTextFromCommand(kcEditMixPaste));
	
	DWORD greyed = pModDoc->CanUndo()?FALSE:MF_GRAYED;
	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_EDIT_UNDO, "Undo\t" + ih->GetKeyTextFromCommand(kcEditUndo));
	}
	return true;
}

bool CViewPattern::BuildVisFXCtxMenu(HMENU hMenu, CInputHandler* ih)
//------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = (ListChansWhereColSelected(EFFECT_COLUMN, validChans)>0)?FALSE:MF_GRAYED;

	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_VISUALIZE_EFFECT, "Visualize Effect\t" + ih->GetKeyTextFromCommand(kcPatternVisualizeEffect));
		return true;
	}
	return false;
}

bool CViewPattern::BuildRandomCtxMenu(HMENU hMenu, CInputHandler* ih)
//------------------------------------------------------------------
{
	AppendMenu(hMenu, MF_STRING, ID_PATTERN_OPEN_RANDOMIZER, "Randomize...\t" + ih->GetKeyTextFromCommand(kcPatternOpenRandomizer));
	return true;
}

bool CViewPattern::BuildTransposeCtxMenu(HMENU hMenu, CInputHandler* ih)
//----------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = (ListChansWhereColSelected(NOTE_COLUMN, validChans)>0)?FALSE:MF_GRAYED;

	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_TRANSPOSE_UP, "Transpose +1\t" + ih->GetKeyTextFromCommand(kcTransposeUp));
		AppendMenu(hMenu, MF_STRING|greyed, ID_TRANSPOSE_DOWN, "Transpose -1\t" + ih->GetKeyTextFromCommand(kcTransposeDown));
		AppendMenu(hMenu, MF_STRING|greyed, ID_TRANSPOSE_OCTUP, "Transpose +12\t" + ih->GetKeyTextFromCommand(kcTransposeOctUp));
		AppendMenu(hMenu, MF_STRING|greyed, ID_TRANSPOSE_OCTDOWN, "Transpose -12\t" + ih->GetKeyTextFromCommand(kcTransposeOctDown));
		return true;
	}
	return false;
}

bool CViewPattern::BuildAmplifyCtxMenu(HMENU hMenu, CInputHandler* ih)
//--------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = (ListChansWhereColSelected(VOL_COLUMN, validChans)>0)?FALSE:MF_GRAYED;

	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_AMPLIFY, "Amplify\t" + ih->GetKeyTextFromCommand(kcPatternAmplify));
		return true;
	}
	return false;
}

bool CViewPattern::BuildSetInstCtxMenu(HMENU hMenu, CInputHandler* ih)
//--------------------------------------------------------------------
{
	CArray<UINT, UINT> validChans;
	DWORD greyed = (ListChansWhereColSelected(INST_COLUMN, validChans)>0)?FALSE:MF_GRAYED;

	if (!greyed || !(CMainFrame::m_dwPatternSetup&PATTERN_OLDCTXMENUSTYLE)) {
		AppendMenu(hMenu, MF_STRING|greyed, ID_PATTERN_SETINSTRUMENT, "Change Instrument\t" + ih->GetKeyTextFromCommand(kcPatternSetInstrument));
		return true;
	}
	return false;
}


UINT CViewPattern::GetSelectionStartRow() {
//-----------------------------------------
	return min(GetRowFromCursor(m_dwBeginSel), GetRowFromCursor(m_dwEndSel));
}

UINT CViewPattern::GetSelectionEndRow() {
//---------------------------------------
	return max(GetRowFromCursor(m_dwBeginSel), GetRowFromCursor(m_dwEndSel));
}

UINT CViewPattern::GetSelectionStartChan() {
//------------------------------------------
	return min(GetChanFromCursor(m_dwBeginSel), GetChanFromCursor(m_dwEndSel));
}

UINT CViewPattern::GetSelectionEndChan() {
//----------------------------------------
	return max(GetChanFromCursor(m_dwBeginSel), GetChanFromCursor(m_dwEndSel));
}

UINT CViewPattern::ListChansWhereColSelected(UINT colType, CArray<UINT,UINT> &chans) {
//----------------------------------------------------------------------------------
	chans.RemoveAll();
	UINT startChan = GetSelectionStartChan();
	UINT endChan   = GetSelectionEndChan();

	if (GetColTypeFromCursor(m_dwBeginSel) <= colType) {
		chans.Add(startChan);	//first selected chan includes this col type
	}
	for (int chan=startChan+1; chan<endChan; chan++) {
		chans.Add(chan); //All chans between first & last must include this col type
	}
	if ((startChan != endChan) && colType <= GetColTypeFromCursor(m_dwEndSel)) {
		chans.Add(endChan);		//last selected chan includes this col type
	}

	return chans.GetCount();
}

UINT CViewPattern::GetRowFromCursor(DWORD cursor) {
//-----------------------------------------------
	return cursor >> 16;
}

UINT CViewPattern::GetChanFromCursor(DWORD cursor) {
//------------------------------------------------
	return (cursor & 0xFFFF) >> 3;
}

UINT CViewPattern::GetColTypeFromCursor(DWORD cursor) {
//--------------------------------------------------
	return cursor & 0x07;
}

bool CViewPattern::IsInterpolationPossible(UINT startRow, UINT endRow, 
										   UINT chan, UINT colType, CSoundFile* pSndFile) {
//---------------------------------------------------------------------------------------
	bool result = false;
	MODCOMMAND *pcmd = pSndFile->Patterns[m_nPattern];
	UINT startRowCmd, endRowCmd;
	switch (colType) {
		case NOTE_COLUMN:
			startRowCmd = pcmd[startRow*pSndFile->m_nChannels+chan].note;
			endRowCmd = pcmd[endRow*pSndFile->m_nChannels+chan].note;
			result = (startRowCmd>0 && endRowCmd>0);
			break;
		case EFFECT_COLUMN:
			startRowCmd = pcmd[startRow*pSndFile->m_nChannels+chan].command;
			endRowCmd = pcmd[endRow*pSndFile->m_nChannels+chan].command;
			result = (startRowCmd == endRowCmd) && (startRowCmd>0 && endRowCmd>0);
			break;
		case VOL_COLUMN:
			startRowCmd = pcmd[startRow*pSndFile->m_nChannels+chan].volcmd;
			endRowCmd = pcmd[endRow*pSndFile->m_nChannels+chan].volcmd;
			result = (startRowCmd == endRowCmd) && (startRowCmd>0 && endRowCmd>0);
			break;
		default:
			result = false;
	}
	return result;
}

void CViewPattern::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	OnRButtonDown(nFlags, point);
	CModScrollView::OnRButtonDblClk(nFlags, point);
}

void CViewPattern::OnTogglePendingMuteFromClick()
{
	TogglePendingMute((m_nMenuParam&0xFFFF)>>3);
}

void CViewPattern::TogglePendingMute(UINT nChn)
{
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	pSndFile = pModDoc->GetSoundFile();
	pSndFile->m_bChannelMuteTogglePending[nChn]=!pSndFile->m_bChannelMuteTogglePending[nChn];
	InvalidateChannelsHeaders();
}
