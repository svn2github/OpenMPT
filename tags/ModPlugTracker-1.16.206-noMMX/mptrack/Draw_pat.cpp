#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "moddoc.h"
#include "dlg_misc.h"
#include "globals.h"
#include "view_pat.h"

// Headers
#define ROWHDR_WIDTH		32	// Row header
#define COLHDR_HEIGHT		16	// Column header
#define COLUMN_HEIGHT		13
#define	VUMETERS_HEIGHT		13	// Height of vu-meters
#define VUMETERS_BMPWIDTH		32
#define VUMETERS_BMPHEIGHT		10
#define VUMETERS_MEDWIDTH		24
#define VUMETERS_LOWIDTH		16


typedef struct PATTERNFONT
{
	int nWidth, nHeight;		// Column Width & Height, including 4-pixels border
	int nClrX, nClrY;			// Clear (empty note) location
	int nSpaceX, nSpaceY;		// White location (must be big enough)
	UINT nEltWidths[5];			// Elements Sizes
	int nNumX, nNumY;			// Vertically-oriented numbers 0x00-0x0F
	int nNum10X, nNum10Y;		// Numbers 10-19
	int nAlphaAM_X,nAlphaAM_Y;	// Letters A-M +#
	int nAlphaNZ_X,nAlphaNZ_Y;	// Letters N-Z +?
	int nNoteX, nNoteY;			// Notes ..., C-, C#, ...
	int nNoteWidth;				// NoteWidth
	int nOctaveWidth;			// Octave Width
	int nVolX, nVolY;			// Volume Column Effects
	int nVolCmdWidth;			// Width of volume effect
	int nVolHiWidth;			// Width of first character in volume parameter
	int nCmdOfs;				// XOffset (-xxx) around the command letter
	int nParamHiWidth;
	int nInstrOfs, nInstr10Ofs, nInstrHiWidth;
} PATTERNFONT;

typedef const PATTERNFONT * PCPATTERNFONT;

//////////////////////////////////////////////
// Font Definitions

// Medium Font (Default)
const PATTERNFONT gDefaultPatternFont = 
{
	92,13,	// Column Width & Height
	0,0,	// Clear location
	32,207,	// Space Location
	{20, 20, 24, 9, 15},		// Element Widths
	20,13,	// Numbers 0-F (hex)
	30,13,	// Numbers 10-19 (dec)
	64,26,	// A-M#
	78,26,	// N-Z?										// MEDIUM FONT !!!
	0, 0,
	12,8,	// Note & Octave Width
	42,13,	// Volume Column Effects
	8,8,
	-1,
	8,		// 8+7 = 15
	-3, -1, 12,
};


//////////////////////////////////////////////////
// Small Font

const PATTERNFONT gSmallPatternFont = 
{
	70,11,	// Column Width & Height
	92,0,	// Clear location
	32,207,	// Space Location
	{16, 14, 18, 7, 11},		// Element Widths
	108,13,	// Numbers 0-F (hex)
	120,13,	// Numbers 10-19 (dec)
	142,26,	// A-M#
	150,26,	// N-Z?										// SMALL FONT !!!
	92, 0,	// Notes
	10,6,	// Note & Octave Width
	132,13,	// Volume Column Effects
	6,5,
	-1,
	6,		// 8+7 = 15
	-3,	1, 9,	// InstrOfs + nInstrHiWidth
};



/////////////////////////////////////////////////////////////////////////////
// CViewPattern Drawing Implementation

inline PCPATTERNFONT GetCurrentPatternFont()
{
	return (CMainFrame::m_dwPatternSetup & PATTERN_SMALLFONT) ? &gSmallPatternFont : &gDefaultPatternFont;
}


static BYTE hilightcolor(int c0, int c1)
{
	int cf0, cf1;

	cf0 = 0xC0 - (c1>>2) - (c0>>3);
	if (cf0 < 0x40) cf0 = 0x40;
	if (cf0 > 0xC0) cf0 = 0xC0;
	cf1 = 0x100 - cf0;
	return (BYTE)((c0*cf0+c1*cf1)>>8);
}


void CViewPattern::UpdateColors()
//-------------------------------
{
	BYTE r,g,b;

	m_Dib.SetAllColors(0, MAX_MODCOLORS, CMainFrame::rgbCustomColors);
	m_Dib.SetColor(MODSYSCOLOR_LO, GetSysColor(COLOR_BTNSHADOW));
	m_Dib.SetColor(MODSYSCOLOR_MED, GetSysColor(COLOR_BTNFACE));
	m_Dib.SetColor(MODSYSCOLOR_HI, GetSysColor(COLOR_BTNHIGHLIGHT));
	r = hilightcolor(GetRValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKHILIGHT]),
					GetRValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKNORMAL]));
	g = hilightcolor(GetGValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKHILIGHT]),
					GetGValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKNORMAL]));
	b = hilightcolor(GetBValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKHILIGHT]),
					GetBValue(CMainFrame::rgbCustomColors[MODCOLOR_BACKNORMAL]));
	m_Dib.SetColor(MODCOLOR_2NDHIGHLIGHT, RGB(r,g,b));
	m_Dib.SetBlendColor(GetSysColor(COLOR_BTNFACE));
}


BOOL CViewPattern::UpdateSizes()
//------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	int oldx = m_szCell.cx;
	m_szHeader.cx = ROWHDR_WIDTH;
	m_szHeader.cy = COLHDR_HEIGHT;
	if (m_dwStatus & PATSTATUS_VUMETERS) m_szHeader.cy += VUMETERS_HEIGHT;
	m_szCell.cx = 4 + pfnt->nEltWidths[0];
	if (m_nDetailLevel > 0) m_szCell.cx += pfnt->nEltWidths[1];
	if (m_nDetailLevel > 1) m_szCell.cx += pfnt->nEltWidths[2];
	if (m_nDetailLevel > 2) m_szCell.cx += pfnt->nEltWidths[3] + pfnt->nEltWidths[4];
	m_szCell.cy = pfnt->nHeight;
	return (oldx != m_szCell.cx);
}


UINT CViewPattern::GetColumnOffset(DWORD dwPos) const
//---------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	UINT n = 0;
	dwPos &= 7;
	if (dwPos > 4) dwPos = 4;
	for (UINT i=0; i<dwPos; i++) n += pfnt->nEltWidths[i];
	return n;
}


void CViewPattern::UpdateView(DWORD dwHintMask, CObject *)
//--------------------------------------------------------
{
	if (dwHintMask & HINT_MPTOPTIONS)
	{
		UpdateColors();
		UpdateSizes();
		UpdateScrollSize();
		InvalidatePattern(TRUE);
		return;
	}
	if (dwHintMask & HINT_MODCHANNELS)
	{
		InvalidateChannelsHeaders();
	}
	if (((dwHintMask & 0xFFFFFF) == HINT_PATTERNDATA) & (m_nPattern != (dwHintMask >> 24))) return;
	if (dwHintMask & (HINT_MODTYPE|HINT_PATTERNDATA))
	{
		InvalidatePattern(FALSE);
	} else
	if (dwHintMask & HINT_PATTERNROW)
	{
		InvalidateRow(dwHintMask >> 24);
	}
}


POINT CViewPattern::GetPointFromPosition(DWORD dwPos)
//---------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	POINT pt;
	int xofs = GetXScrollPos();
	int yofs = GetYScrollPos();
	pt.x = (((dwPos >> 3) & 0xFF) - xofs) * GetColumnWidth();
	UINT imax = (dwPos & 7);
	if (imax > 5) imax = 5;
	if (imax > m_nDetailLevel+1) imax = m_nDetailLevel+1;
	for (UINT i=0; i<imax; i++)
	{
		pt.x += pfnt->nEltWidths[i];
	}
	if (pt.x < 0) pt.x = 0;
	pt.x += ROWHDR_WIDTH;
	pt.y = ((dwPos >> 16) - yofs + m_nMidRow) * m_szCell.cy;
	if (pt.y < 0) pt.y = 0;
	pt.y += m_szHeader.cy;
	return pt;
}


DWORD CViewPattern::GetPositionFromPoint(POINT pt)
//------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	int xofs = GetXScrollPos();
	int yofs = GetYScrollPos();
	int x = xofs + (pt.x - m_szHeader.cx) / GetColumnWidth();
	if (pt.x < m_szHeader.cx) x = (xofs) ? xofs - 1 : 0;
	int y = yofs - m_nMidRow + (pt.y - m_szHeader.cy) / m_szCell.cy;
	if (y < 0) y = 0;
	int xx = (pt.x - m_szHeader.cx) % GetColumnWidth(), dx = 0;
	int imax = 4;
	if (imax > (int)m_nDetailLevel+1) imax = m_nDetailLevel+1;
	for (int i=0; i<imax; i++)
	{
		dx += pfnt->nEltWidths[i];
		if (xx < dx) break;
	}
	return (y << 16) | (x << 3) | i;
}


void CViewPattern::DrawLetter(int x, int y, char letter, int sizex, int ofsx)
//---------------------------------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	int srcx = pfnt->nSpaceX, srcy = pfnt->nSpaceY;

	if ((letter >= '0') && (letter <= '9'))
	{
		srcx = pfnt->nNumX;
		srcy = pfnt->nNumY + (letter - '0') * COLUMN_HEIGHT;
	} else
	if ((letter >= 'A') && (letter < 'N'))
	{
		srcx = pfnt->nAlphaAM_X;
		srcy = pfnt->nAlphaAM_Y + (letter - 'A') * COLUMN_HEIGHT;
	} else
	if ((letter >= 'N') && (letter <= 'Z'))
	{
		srcx = pfnt->nAlphaNZ_X;
		srcy = pfnt->nAlphaNZ_Y + (letter - 'N') * COLUMN_HEIGHT;
	} else
	switch(letter)
	{
	case '?':
		srcx = pfnt->nAlphaNZ_X;
		srcy = pfnt->nAlphaNZ_Y + 13 * COLUMN_HEIGHT;
		break;
	case '#':
		srcx = pfnt->nAlphaAM_X;
		srcy = pfnt->nAlphaAM_Y + 13 * COLUMN_HEIGHT;
		break;
	}
	m_Dib.TextBlt(x, y, sizex, COLUMN_HEIGHT, srcx+ofsx, srcy);
}


void CViewPattern::DrawNote(int x, int y, UINT note)
//--------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	UINT xsrc = pfnt->nNoteX, ysrc = pfnt->nNoteY, dx = pfnt->nEltWidths[0];
	if (!note)
	{
		m_Dib.TextBlt(x, y, dx, COLUMN_HEIGHT, xsrc, ysrc);
	} else
	if (note == 0xFE)
	{
		m_Dib.TextBlt(x, y, dx, COLUMN_HEIGHT, xsrc, ysrc + 13*COLUMN_HEIGHT);
	} else
	if (note >= 0xFF)
	{
		m_Dib.TextBlt(x, y, dx, COLUMN_HEIGHT, xsrc, ysrc + 14*COLUMN_HEIGHT);
	} else
	{
		UINT o = (note-1) / 12;
		UINT n = (note-1) % 12;
		m_Dib.TextBlt(x, y, pfnt->nNoteWidth, COLUMN_HEIGHT, xsrc, ysrc+(n+1)*COLUMN_HEIGHT);
		m_Dib.TextBlt(x+pfnt->nNoteWidth, y, pfnt->nOctaveWidth, COLUMN_HEIGHT,
						pfnt->nNumX, pfnt->nNumY+o*COLUMN_HEIGHT);
	}
}


void CViewPattern::DrawInstrument(int x, int y, UINT instr)
//---------------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	if (instr)
	{
		UINT dx = pfnt->nInstrHiWidth;
		if (instr < 100)
		{
			m_Dib.TextBlt(x, y, dx, COLUMN_HEIGHT, pfnt->nNumX+pfnt->nInstrOfs, pfnt->nNumY+(instr / 10)*COLUMN_HEIGHT);
		} else
		{
			m_Dib.TextBlt(x, y, dx, COLUMN_HEIGHT, pfnt->nNum10X+pfnt->nInstr10Ofs, pfnt->nNum10Y+((instr-100) / 10)*COLUMN_HEIGHT);
		}
		m_Dib.TextBlt(x+dx, y, pfnt->nEltWidths[1]-dx, COLUMN_HEIGHT, pfnt->nNumX+1, pfnt->nNumY+(instr % 10)*COLUMN_HEIGHT);
	} else
	{
		m_Dib.TextBlt(x, y, pfnt->nEltWidths[1], COLUMN_HEIGHT, pfnt->nClrX+pfnt->nEltWidths[0], pfnt->nClrY);
	}
}


void CViewPattern::DrawVolumeCommand(int x, int y, UINT volcmd, UINT vol)
//-----------------------------------------------------------------------
{
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	if (volcmd)
	{
		volcmd &= 0x0F;
		vol &= 0x7F;
		m_Dib.TextBlt(x, y, pfnt->nVolCmdWidth, COLUMN_HEIGHT,
						pfnt->nVolX, pfnt->nVolY+volcmd*COLUMN_HEIGHT);
		m_Dib.TextBlt(x+pfnt->nVolCmdWidth, y, pfnt->nVolHiWidth, COLUMN_HEIGHT,
						pfnt->nNumX, pfnt->nNumY+(vol / 10)*COLUMN_HEIGHT);
		m_Dib.TextBlt(x+pfnt->nVolCmdWidth+pfnt->nVolHiWidth, y, pfnt->nEltWidths[2]-(pfnt->nVolCmdWidth+pfnt->nVolHiWidth), COLUMN_HEIGHT,
						pfnt->nNumX, pfnt->nNumY+(vol % 10)*COLUMN_HEIGHT);
	} else
	{
		int srcx = pfnt->nEltWidths[0] + pfnt->nEltWidths[1];
		m_Dib.TextBlt(x, y, pfnt->nEltWidths[2], COLUMN_HEIGHT, pfnt->nClrX+srcx, pfnt->nClrY);
	}
}


void CViewPattern::OnDraw(CDC *pDC)
//---------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CHAR s[256];
	HGDIOBJ oldpen;
	CRect rcClient, rect, rc;
	CModDoc *pModDoc;
	CSoundFile *pSndFile;
	HDC hdc;
	UINT xofs, yofs, nColumnWidth, ncols, nrows, ncolhdr;
	int xpaint, ypaint;

	ASSERT(pDC);
	UpdateSizes();
	if ((pModDoc = GetDocument()) == NULL) return;
	GetClientRect(&rcClient);
	hdc = pDC->m_hDC;
	oldpen = ::SelectObject(hdc, CMainFrame::penDarkGray);
	xofs = GetXScrollPos();
	yofs = GetYScrollPos();
	pSndFile = pModDoc->GetSoundFile();
	nColumnWidth = m_szCell.cx;
	nrows = pSndFile->PatternSize[m_nPattern];
	ncols = pSndFile->m_nChannels;
	xpaint = m_szHeader.cx;
	ypaint = rcClient.top;
	ncolhdr = xofs;
	rect.SetRect(0, rcClient.top, rcClient.right, rcClient.top + m_szHeader.cy);
	if (::RectVisible(hdc, &rect))
	{
		wsprintf(s, "#%d", m_nPattern);
		rect.right = m_szHeader.cx;
		DrawButtonRect(hdc, &rect, s, FALSE,
			((m_bInItemRect) && ((m_nDragItem & DRAGITEM_MASK) == DRAGITEM_PATTERNHEADER)) ? TRUE : FALSE);
		// Drawing Channel Headers
		while (xpaint < rcClient.right)
		{
			rect.SetRect(xpaint, ypaint, xpaint+nColumnWidth, ypaint + m_szHeader.cy);
			if (ncolhdr < ncols)
			{
				const char *pszfmt = "Channel %d";
				if ((pSndFile->m_nType & (MOD_TYPE_XM|MOD_TYPE_IT)) && ((BYTE)pSndFile->ChnSettings[ncolhdr].szName[0] > 0x20))
					pszfmt = "%d: %s";
				else if (m_nDetailLevel < 2) pszfmt = "Ch%d";
				else if (m_nDetailLevel < 3) pszfmt = "Chn %d";
				wsprintf(s, pszfmt, ncolhdr+1, pSndFile->ChnSettings[ncolhdr].szName);
				DrawButtonRect(hdc, &rect, s,
					(pSndFile->ChnSettings[ncolhdr].dwFlags & CHN_MUTE) ? TRUE : FALSE,
					((m_bInItemRect) && ((m_nDragItem & DRAGITEM_MASK) == DRAGITEM_CHNHEADER) && ((m_nDragItem & 0xFFFF) == ncolhdr)) ? TRUE : FALSE, DT_CENTER);
				rect.bottom = rect.top + COLHDR_HEIGHT;
				if (MultiRecordMask[ncolhdr>>3] & (1 << (ncolhdr&7)))
				{
					rect.DeflateRect(1, 1);
					InvertRect(hdc, &rect);
					rect.InflateRect(1, 1);
				}
				if (m_dwStatus & PATSTATUS_VUMETERS)
				{
					OldVUMeters[ncolhdr] = 0;
					DrawChannelVUMeter(hdc, rect.left + 1, rect.bottom, ncolhdr);
				}
			} else break;
			ncolhdr++;
			xpaint += nColumnWidth;
		}
	}
	ypaint += m_szHeader.cy;
	if (m_nMidRow)
	{
		if (yofs >= m_nMidRow)
		{
			yofs -= m_nMidRow;
		} else
		{
			UINT nSkip = m_nMidRow - yofs;
			UINT nPrevPat = m_nPattern;
			BOOL bPrevPatFound = FALSE;

			if (CMainFrame::m_dwPatternSetup & PATTERN_SHOWPREVIOUS)
			{
				UINT nCurOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER);
			
				if ((nCurOrder > 0) && (nCurOrder < MAX_ORDERS) && (pSndFile->Order[nCurOrder] == m_nPattern))
				{
					nPrevPat = pSndFile->Order[nCurOrder-1];
					bPrevPatFound = TRUE;
				}
			}
			if ((bPrevPatFound) && (nPrevPat < MAX_PATTERNS) && (pSndFile->Patterns[nPrevPat]))
			{
				UINT nPrevRows = pSndFile->PatternSize[nPrevPat];
				UINT n = (nSkip < nPrevRows) ? nSkip : nPrevRows;

				ypaint += (nSkip-n)*m_szCell.cy;
				rect.SetRect(0, m_szHeader.cy, nColumnWidth * ncols + m_szHeader.cx, ypaint - 1);
				m_Dib.SetBlendMode(0x80);
				DrawPatternData(hdc, pSndFile, nPrevPat, FALSE, FALSE,
						nPrevRows-n, nPrevRows, xofs, rcClient, &ypaint);
				m_Dib.SetBlendMode(0);
			} else
			{
				ypaint += nSkip * m_szCell.cy;
				rect.SetRect(0, m_szHeader.cy, nColumnWidth * ncols + m_szHeader.cx, ypaint - 1);
			}
			if ((rect.bottom > rect.top) && (rect.right > rect.left))
			{
				::FillRect(hdc, &rect, CMainFrame::brushGray);
				::MoveToEx(hdc, 0, rect.bottom, NULL);
				::LineTo(hdc, rect.right, rect.bottom);
			}
			yofs = 0;
		}
	}
	int ypatternend = ypaint + (nrows-yofs)*m_szCell.cy;
	DrawPatternData(hdc, pSndFile, m_nPattern, TRUE, (pMainFrm->GetModPlaying() == pModDoc) ? TRUE : FALSE,
					yofs, nrows, xofs, rcClient, &ypaint);
	// Display next pattern
	if ((CMainFrame::m_dwPatternSetup & PATTERN_SHOWPREVIOUS) && (ypaint < rcClient.bottom) && (ypaint == ypatternend))
	{
		int nVisRows = (rcClient.bottom - ypaint + m_szCell.cy - 1) / m_szCell.cy;
		if ((nVisRows > 0) && (m_nMidRow))
		{
			UINT nNextPat = m_nPattern;
			BOOL bNextPatFound = FALSE;
			UINT nCurOrder = SendCtrlMessage(CTRLMSG_GETCURRENTORDER);
			
			if ((nCurOrder+1 < MAX_ORDERS) && (pSndFile->Order[nCurOrder] == m_nPattern))
			{
				nNextPat = pSndFile->Order[nCurOrder+1];
				bNextPatFound = TRUE;
			}
			if ((bNextPatFound) && (nNextPat < MAX_PATTERNS) && (pSndFile->Patterns[nNextPat]))
			{
				UINT nNextRows = pSndFile->PatternSize[nNextPat];
				UINT n = ((UINT)nVisRows < nNextRows) ? nVisRows : nNextRows;

				m_Dib.SetBlendMode(0x80);
				DrawPatternData(hdc, pSndFile, nNextPat, FALSE, FALSE,
						0, n, xofs, rcClient, &ypaint);
				m_Dib.SetBlendMode(0);
			}
		}
	}
	// Drawing outside pattern area
	xpaint = m_szHeader.cx + (ncols - xofs) * nColumnWidth;
	if ((xpaint < rcClient.right) && (ypaint > rcClient.top))
	{
		rc.SetRect(xpaint, rcClient.top, rcClient.right, ypaint);
		::FillRect(hdc, &rc, CMainFrame::brushGray);
	}
	if (ypaint < rcClient.bottom)
	{
		rc.SetRect(0, ypaint, rcClient.right+1, rcClient.bottom+1);
		DrawButtonRect(hdc, &rc, "");
	}
	// Drawing pattern selection
	if (m_dwStatus & PATSTATUS_DRAGNDROPPING)
	{
		DrawDragSel(hdc);
	}
	if (oldpen) ::SelectObject(hdc, oldpen);
}


void CViewPattern::DrawPatternData(HDC hdc,	CSoundFile *pSndFile, UINT nPattern, BOOL bSelEnable,
						BOOL bPlaying, UINT yofs, UINT nrows, UINT xofs, CRect &rcClient, int *pypaint)
//-----------------------------------------------------------------------------------------------------
{
	BYTE bColSel[MAX_CHANNELS];
	PCPATTERNFONT pfnt = GetCurrentPatternFont();
	MODCOMMAND m0, *pPattern = pSndFile->Patterns[nPattern];
	CHAR s[256];
	CRect rect;
	int xpaint, ypaint = *pypaint;
	int row_col, row_bkcol;
	UINT bRowSel, bSpeedUp, nColumnWidth, ncols, maxcol;
	
	ncols = pSndFile->m_nChannels;
	m0.note = m0.instr = m0.vol = m0.volcmd = m0.command = m0.param = 0;
	nColumnWidth = m_szCell.cx;
	rect.SetRect(m_szHeader.cx, rcClient.top, m_szHeader.cx+nColumnWidth, rcClient.bottom);
	for (UINT cmk=xofs; cmk<ncols; cmk++)
	{
		UINT c = cmk << 3;
		bColSel[cmk] = 0;
		if (bSelEnable)
		{
			for (UINT n=0; n<5; n++)
			{
				if ((c+n >= (m_dwBeginSel & 0xFFFF)) && (c+n <= (m_dwEndSel & 0xFFFF))) bColSel[cmk] |= 1 << n;
			}
		}
		if (!::RectVisible(hdc, &rect)) bColSel[cmk] |= 0x80;
		rect.left += nColumnWidth;
		rect.right += nColumnWidth;
	}
	// Max Visible Column
	maxcol = ncols;
	while ((maxcol > xofs) && (bColSel[maxcol-1] & 0x80)) maxcol--;
	// Init bitmap border
	{
		UINT maxndx = pSndFile->m_nChannels * m_szCell.cx;
		UINT ibmp = 0;
		if (maxndx > FASTBMP_MAXWIDTH) maxndx = FASTBMP_MAXWIDTH;
		do
		{
			ibmp += nColumnWidth;
			m_Dib.TextBlt(ibmp-4, 0, 4, m_szCell.cy, pfnt->nClrX+pfnt->nWidth-4, pfnt->nClrY);
		} while (ibmp + nColumnWidth <= maxndx);
	}
	bRowSel = FALSE;
	row_col = row_bkcol = -1;
	for (UINT row=yofs; row<nrows; row++)
	{
		UINT col, xbmp, nbmp, oldrowcolor;
		
		wsprintf(s, (CMainFrame::m_dwPatternSetup & PATTERN_HEXDISPLAY) ? "%02X" : "%d", row);
		rect.left = 0;
		rect.top = ypaint;
		rect.right = rcClient.right;
		rect.bottom = rect.top + m_szCell.cy;
		if (!::RectVisible(hdc, &rect)) 
		{
			// No speedup for these columns next time
			for (UINT iup=xofs; iup<maxcol; iup++) bColSel[iup] &= ~0x40;
			goto SkipRow;
		}
		rect.right = rect.left + m_szHeader.cx;
		DrawButtonRect(hdc, &rect, s, !bSelEnable);
		oldrowcolor = (row_bkcol << 16) | (row_col << 8) | (bRowSel);
		bRowSel = ((row >= (m_dwBeginSel >> 16)) && (row <= (m_dwEndSel >> 16))) ? TRUE : FALSE;
		row_col = MODCOLOR_TEXTNORMAL;
		row_bkcol = MODCOLOR_BACKNORMAL;
		if ((CMainFrame::m_dwPatternSetup & PATTERN_2NDHIGHLIGHT)
		 && (CMainFrame::m_nRowSpacing2) && (CMainFrame::m_nRowSpacing2 < nrows))
		{
			if (!(row % CMainFrame::m_nRowSpacing2))
			{
				row_bkcol = MODCOLOR_2NDHIGHLIGHT;
			}
		}
		if ((CMainFrame::m_dwPatternSetup & PATTERN_STDHIGHLIGHT)
		 && (CMainFrame::m_nRowSpacing) && (CMainFrame::m_nRowSpacing < nrows))
		{
			if (!(row % CMainFrame::m_nRowSpacing))
			{
				row_bkcol = MODCOLOR_BACKHILIGHT;
			}
		}
		if (bSelEnable)
		{
			if ((row == m_nPlayRow) && (nPattern == m_nPlayPat))
			{
				row_col = MODCOLOR_TEXTPLAYCURSOR;
				row_bkcol = MODCOLOR_BACKPLAYCURSOR;
			}
			if (row == m_nRow)
			{
				if (m_dwStatus & PATSTATUS_FOCUS)
				{
					row_col = MODCOLOR_TEXTCURROW;
					row_bkcol = MODCOLOR_BACKCURROW;
				} else
				if ((m_dwStatus & PATSTATUS_FOLLOWSONG) && (bPlaying))
				{
					row_col = MODCOLOR_TEXTPLAYCURSOR;
					row_bkcol = MODCOLOR_BACKPLAYCURSOR;
				}
			}
		}
		// Eliminate non-visible column
		xpaint = m_szHeader.cx;
		col = xofs;
		while ((bColSel[col] & 0x80) && (col < maxcol))
		{
			bColSel[col] &= ~0x40;
			col++;
			xpaint += nColumnWidth;
		}
		// Optimization: same row color ?
		bSpeedUp = (oldrowcolor == (UINT)((row_bkcol << 16) | (row_col << 8) | bRowSel)) ? TRUE : FALSE;
		xbmp = nbmp = 0;
		do
		{
			DWORD dwSpeedUpMask;
			MODCOMMAND *m;
			int x, bk_col, tx_col, col_sel, fx_col;

			m = (pPattern) ? &pPattern[row*ncols+col] : &m0;
			dwSpeedUpMask = 0;
			if ((bSpeedUp) && (bColSel[col] & 0x40) && (pPattern) && (row))
			{
				MODCOMMAND *mold = m - ncols;
				if (m->note == mold->note) dwSpeedUpMask |= 0x01;
				if ((m->instr == mold->instr) || (m_nDetailLevel < 1)) dwSpeedUpMask |= 0x02;
				if (((m->volcmd == mold->volcmd) && ((!m->volcmd) || (m->vol == mold->vol))) || (m_nDetailLevel < 2)) dwSpeedUpMask |= 0x04;
				if ((m->command == mold->command) || (m_nDetailLevel < 3)) dwSpeedUpMask |= (m->command) ? 0x08 : 0x18;
				if (dwSpeedUpMask == 0x1F) goto DoBlit;
			}
			bColSel[col] |= 0x40;
			col_sel = 0;
			if (bRowSel) col_sel = bColSel[col] & 0x3F;
			tx_col = row_col;
			bk_col = row_bkcol;
			if (col_sel)
			{
				tx_col = MODCOLOR_TEXTSELECTED;
				bk_col = MODCOLOR_BACKSELECTED;
			}
			if ((!*((LPDWORD)m)) && (!*(((LPWORD)m)+2)) && ((!col_sel) || (col_sel == 0x1F)))
			{
				m_Dib.SetTextColor(tx_col, bk_col);
				m_Dib.TextBlt(xbmp, 0, nColumnWidth-4, m_szCell.cy, pfnt->nClrX, pfnt->nClrY);
				goto DoBlit;
			}
			x = 0;
			// Note
			if (!(dwSpeedUpMask & 0x01))
			{
				tx_col = row_col;
				bk_col = row_bkcol;
				if ((CMainFrame::m_dwPatternSetup & PATTERN_EFFECTHILIGHT) && (m->note) && (m->note <= 120))
				{
					tx_col = MODCOLOR_NOTE;
				}
				if (col_sel & 0x01)
				{
					tx_col = MODCOLOR_TEXTSELECTED;
					bk_col = MODCOLOR_BACKSELECTED;
				}
				// Drawing note
				m_Dib.SetTextColor(tx_col, bk_col);
				DrawNote(xbmp+x, 0, m->note);
			}
			x += pfnt->nEltWidths[0];
			// Instrument
			if (m_nDetailLevel > 0)
			{
				if (!(dwSpeedUpMask & 0x02))
				{
					tx_col = row_col;
					bk_col = row_bkcol;
					if ((CMainFrame::m_dwPatternSetup & PATTERN_EFFECTHILIGHT) && (m->instr))
					{
						tx_col = MODCOLOR_INSTRUMENT;
					}
					if (col_sel & 0x02)
					{
						tx_col = MODCOLOR_TEXTSELECTED;
						bk_col = MODCOLOR_BACKSELECTED;
					}
					// Drawing instrument
					m_Dib.SetTextColor(tx_col, bk_col);
					DrawInstrument(xbmp+x, 0, m->instr);
				}
				x += pfnt->nEltWidths[1];
			}
			// Volume
			if (m_nDetailLevel > 1)
			{
				if (!(dwSpeedUpMask & 0x04))
				{
					tx_col = row_col;
					bk_col = row_bkcol;
					if (col_sel & 0x04)
					{
						tx_col = MODCOLOR_TEXTSELECTED;
						bk_col = MODCOLOR_BACKSELECTED;
					} else
					if ((m->volcmd) && (CMainFrame::m_dwPatternSetup & PATTERN_EFFECTHILIGHT))
					{
						switch(m->volcmd)
						{
						case VOLCMD_VOLUME:
						case VOLCMD_VOLSLIDEUP:
						case VOLCMD_VOLSLIDEDOWN:
						case VOLCMD_FINEVOLUP:
						case VOLCMD_FINEVOLDOWN:
							tx_col = MODCOLOR_VOLUME;
							break;
						case VOLCMD_PANNING:
						case VOLCMD_PANSLIDELEFT:
						case VOLCMD_PANSLIDERIGHT:
							tx_col = MODCOLOR_PANNING;
							break;
						case VOLCMD_VIBRATOSPEED:
						case VOLCMD_VIBRATO:
						case VOLCMD_TONEPORTAMENTO:
						case VOLCMD_PORTAUP:
						case VOLCMD_PORTADOWN:
							tx_col = MODCOLOR_PITCH;
							break;
						}
					}
					// Drawing Volume
					m_Dib.SetTextColor(tx_col, bk_col);
					DrawVolumeCommand(xbmp+x, 0, m->volcmd, m->vol);
				}
				x += pfnt->nEltWidths[2];
			}
			// Command & param
			if (m_nDetailLevel > 2)
			{
				fx_col = row_col;
				if ((m->command) && (m->command < MAX_EFFECTS) && (CMainFrame::m_dwPatternSetup & PATTERN_EFFECTHILIGHT))
				{
					switch(gEffectColors[m->command])
					{
					case MODCOLOR_VOLUME:
						fx_col = MODCOLOR_VOLUME;
						break;
					case MODCOLOR_PANNING:
						fx_col = MODCOLOR_PANNING;
						break;
					case MODCOLOR_PITCH:
						fx_col = MODCOLOR_PITCH;
						break;
					case MODCOLOR_GLOBALS:
						fx_col = MODCOLOR_GLOBALS;
						break;
					}
				}
				if (!(dwSpeedUpMask & 0x08))
				{
					tx_col = fx_col;
					bk_col = row_bkcol;
					if (col_sel & 0x08)
					{
						tx_col = MODCOLOR_TEXTSELECTED;
						bk_col = MODCOLOR_BACKSELECTED;
					}
					// Drawing Command
					m_Dib.SetTextColor(tx_col, bk_col);
					if (m->command)
					{
						UINT command = m->command & 0x3F;
						int n =	(pSndFile->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM)) ? gszModCommands[command] : gszS3mCommands[command];
						if (n <= ' ') n = '?';
						DrawLetter(xbmp+x, 0, (char)n, pfnt->nEltWidths[3], pfnt->nCmdOfs);
					} else
					{
						m_Dib.TextBlt(xbmp+x, 0, pfnt->nEltWidths[3], COLUMN_HEIGHT, pfnt->nClrX+x, pfnt->nClrY);
					}
				}
				x += pfnt->nEltWidths[3];
				// Param
				if (!(dwSpeedUpMask & 0x10))
				{
					tx_col = fx_col;
					bk_col = row_bkcol;
					if (col_sel & 0x10)
					{
						tx_col = MODCOLOR_TEXTSELECTED;
						bk_col = MODCOLOR_BACKSELECTED;
					}
					// Drawing param
					m_Dib.SetTextColor(tx_col, bk_col);
					if (m->command)
					{
						m_Dib.TextBlt(xbmp+x, 0, pfnt->nParamHiWidth, m_szCell.cy, pfnt->nNumX, pfnt->nNumY+(m->param >> 4)*COLUMN_HEIGHT);
						m_Dib.TextBlt(xbmp+x+pfnt->nParamHiWidth, 0, pfnt->nEltWidths[4]-pfnt->nParamHiWidth, m_szCell.cy, pfnt->nNumX+1, pfnt->nNumY+(m->param & 0x0F)*COLUMN_HEIGHT);
					} else
					{
						m_Dib.TextBlt(xbmp+x, 0, pfnt->nEltWidths[4], m_szCell.cy, pfnt->nClrX+x, pfnt->nClrY);
					}
				}
			}
		DoBlit:
			nbmp++;
			xbmp += nColumnWidth;
			xpaint += nColumnWidth;
			if ((xbmp + nColumnWidth >= FASTBMP_MAXWIDTH) || (xpaint >= rcClient.right)) break;
		} while (++col < maxcol);
		m_Dib.Blit(hdc, xpaint-xbmp, ypaint, xbmp, m_szCell.cy);
	SkipRow:
		ypaint += m_szCell.cy;
		if (ypaint >= rcClient.bottom) break;
	}
	*pypaint = ypaint;
}


void CViewPattern::DrawChannelVUMeter(HDC hdc, int x, int y, UINT nChn)
//---------------------------------------------------------------------
{
	if (ChnVUMeters[nChn] != OldVUMeters[nChn])
	{
		UINT vul, vur;
		vul = ChnVUMeters[nChn] & 0xFF;
		vur = (ChnVUMeters[nChn] & 0xFF00) >> 8;
		vul /= 15;
		vur /= 15;
		if (vul > 8) vul = 8;
		if (vur > 8) vur = 8;
		x += (m_szCell.cx / 2);
		if (m_nDetailLevel <= 1)
		{
			DibBlt(hdc, x-VUMETERS_LOWIDTH-1, y, VUMETERS_LOWIDTH, VUMETERS_BMPHEIGHT,
				VUMETERS_BMPWIDTH*2+VUMETERS_MEDWIDTH*2, vul * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
			DibBlt(hdc, x-1, y, VUMETERS_LOWIDTH, VUMETERS_BMPHEIGHT,
				VUMETERS_BMPWIDTH*2+VUMETERS_MEDWIDTH*2+VUMETERS_LOWIDTH, vur * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
		} else
		if (m_nDetailLevel <= 2)
		{
			DibBlt(hdc, x - VUMETERS_MEDWIDTH-1, y, VUMETERS_MEDWIDTH, VUMETERS_BMPHEIGHT,
				VUMETERS_BMPWIDTH*2, vul * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
			DibBlt(hdc, x, y, VUMETERS_MEDWIDTH, VUMETERS_BMPHEIGHT,
				VUMETERS_BMPWIDTH*2+VUMETERS_MEDWIDTH, vur * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
		} else
		{
			DibBlt(hdc, x - VUMETERS_BMPWIDTH - 1, y, VUMETERS_BMPWIDTH, VUMETERS_BMPHEIGHT,
				0, vul * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
			DibBlt(hdc, x + 1, y, VUMETERS_BMPWIDTH, VUMETERS_BMPHEIGHT,
				VUMETERS_BMPWIDTH, vur * VUMETERS_BMPHEIGHT, CMainFrame::bmpVUMeters);
		}
		OldVUMeters[nChn] = ChnVUMeters[nChn];
	}
}


void CViewPattern::DrawDragSel(HDC hdc)
//-------------------------------------
{
	CModDoc *pModDoc;
	CSoundFile *pSndFile;
	CRect rect;
	POINT ptTopLeft, ptBottomRight;
	DWORD dwTopLeft, dwBottomRight;
	BOOL bLeft, bTop, bRight, bBottom;
	int x1, y1, x2, y2, dx, dy, c1, c2;
	int nChannels, nRows;

	if ((pModDoc = GetDocument()) == NULL) return;
	pSndFile = pModDoc->GetSoundFile();
	bLeft = bTop = bRight = bBottom = TRUE;
	x1 = (m_dwBeginSel & 0xFFF8) >> 3;
	y1 = (m_dwBeginSel) >> 16;
	x2 = (m_dwEndSel & 0xFFF8) >> 3;
	y2 = (m_dwEndSel) >> 16;
	c1 = (m_dwBeginSel&7);
	c2 = (m_dwEndSel&7);
	dx = (int)((m_dwDragPos & 0xFFF8) >> 3) - (int)((m_dwStartSel & 0xFFF8) >> 3);
	dy = (int)(m_dwDragPos >> 16) - (int)(m_dwStartSel >> 16);
	x1 += dx;
	x2 += dx;
	y1 += dy;
	y2 += dy;
	nChannels = pSndFile->m_nChannels;
	nRows = pSndFile->PatternSize[m_nPattern];
	if (x1 < GetXScrollPos()) bLeft = FALSE;
	if (x1 >= nChannels) x1 = nChannels-1;
	if (x1 < 0) { x1 = 0; c1 = 0; bLeft = FALSE; }
	if (x2 >= nChannels) { x2 = nChannels-1; c2 = 4; bRight = FALSE; }
	if (x2 < 0) x2 = 0;
	if (y1 < GetYScrollPos() - (int)m_nMidRow) bTop = FALSE;
	if (y1 >= nRows) y1 = nRows-1;
	if (y1 < 0) { y1 = 0; bTop = FALSE; }
	if (y2 >= nRows) { y2 = nRows-1; bBottom = FALSE; }
	if (y2 < 0) y2 = 0;
	dwTopLeft = (y1<<16)|(x1<<3)|c1;
	dwBottomRight = ((y2+1)<<16)|(x2<<3)|(c2+1);
	ptTopLeft = GetPointFromPosition(dwTopLeft);
	ptBottomRight = GetPointFromPosition(dwBottomRight);
	if ((ptTopLeft.x >= ptBottomRight.x) || (ptTopLeft.y >= ptBottomRight.y)) return;
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	if (pBrush != NULL)
	{
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, pBrush->m_hObject);
		// Top
		if (bTop)
		{
			rect.SetRect(ptTopLeft.x+4, ptTopLeft.y, ptBottomRight.x, ptTopLeft.y+4);
			if (!bLeft) rect.left -= 4;
			PatBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		}
		// Bottom
		if (bBottom)
		{
			rect.SetRect(ptTopLeft.x, ptBottomRight.y-4, ptBottomRight.x-4, ptBottomRight.y);
			if (!bRight) rect.right += 4;
			PatBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		}
		// Left
		if (bLeft)
		{
			rect.SetRect(ptTopLeft.x, ptTopLeft.y, ptTopLeft.x+4, ptBottomRight.y-4);
			if (!bBottom) rect.bottom += 4;
			PatBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		}
		// Right
		if (bRight)
		{
			rect.SetRect(ptBottomRight.x-4, ptTopLeft.y+4, ptBottomRight.x, ptBottomRight.y);
			if (!bTop) rect.top -= 4;
			PatBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		}
		if (hOldBrush != NULL) SelectObject(hdc, hOldBrush);
	}

}


void CViewPattern::OnDrawDragSel()
//--------------------------------
{
	HDC hdc = ::GetDC(m_hWnd);
	if (hdc != NULL)
	{
		DrawDragSel(hdc);
		::ReleaseDC(m_hWnd, hdc);
	}
}


///////////////////////////////////////////////////////////////////////////////
// CViewPattern Scrolling Functions


void CViewPattern::UpdateScrollSize()
//-----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CRect rect;
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		SIZE sizeTotal, sizePage, sizeLine;
		sizeTotal.cx = m_szHeader.cx + pSndFile->m_nChannels * m_szCell.cx;
		sizeTotal.cy = m_szHeader.cy + pSndFile->PatternSize[m_nPattern] * m_szCell.cy;
		sizeLine.cx = m_szCell.cx;
		sizeLine.cy = m_szCell.cy;
		sizePage.cx = sizeLine.cx * 2;
		sizePage.cy = sizeLine.cy * 8;
		GetClientRect(&rect);
		m_nMidRow = 0;
		if (CMainFrame::m_dwPatternSetup & PATTERN_CENTERROW) m_nMidRow = (rect.Height() - m_szHeader.cy) / (m_szCell.cy << 1);
		if (m_nMidRow) sizeTotal.cy += m_nMidRow * m_szCell.cy * 2;
		SetScrollSizes(MM_TEXT, sizeTotal, sizePage, sizeLine);
	}
}


void CViewPattern::UpdateScrollPos()
//----------------------------------
{
	int x = GetScrollPos(SB_HORZ);
	if (x < 0) x = 0;
	m_nXScroll = (x + m_szCell.cx - 1) / m_szCell.cx;
	int y = GetScrollPos(SB_VERT);
	if (y < 0) y = 0;
	m_nYScroll = (y + m_szCell.cy - 1) / m_szCell.cy;
}


BOOL CViewPattern::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
//-------------------------------------------------------------
{
	int xOrig, xNew, x;
	int yOrig, yNew, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	x += sizeScroll.cx;
	if (x < 0) x = 0; else if (x > xMax) x = xMax;

	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	y += sizeScroll.cy;
	if (y < 0) y = 0; else if (y > yMax) y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig) return FALSE;

	if (!bDoScroll) return TRUE;
	xNew = x;
	yNew = y;

	if (x > 0) x = (x + m_szCell.cx - 1) / m_szCell.cx; else x = 0;
	if (y > 0) y = (y + m_szCell.cy - 1) / m_szCell.cy; else y = 0;
	if ((x != m_nXScroll) || (y != m_nYScroll))
	{
		CRect rect;
		GetClientRect(&rect);
		if (x != m_nXScroll)
		{
			rect.left = m_szHeader.cx;
			rect.top = 0;
			ScrollWindow((m_nXScroll-x)*GetColumnWidth(), 0, &rect, &rect);
			m_nXScroll = x;
		}
		if (y != m_nYScroll)
		{
			rect.left = 0;
			rect.top = m_szHeader.cy;
			ScrollWindow(0, (m_nYScroll-y)*GetColumnHeight(), &rect, &rect);
			m_nYScroll = y;
		}
	}
	if (xNew != xOrig) SetScrollPos(SB_HORZ, xNew);
	if (yNew != yOrig) SetScrollPos(SB_VERT, yNew);
	return TRUE;
}


void CViewPattern::OnSize(UINT nType, int cx, int cy)
//---------------------------------------------------
{
	CScrollView::OnSize(nType, cx, cy);
	if (((nType == SIZE_RESTORED) || (nType == SIZE_MAXIMIZED)) && (cx > 0) && (cy > 0))
	{
		UpdateSizes();
		UpdateScrollSize();
		OnScroll(0,0,TRUE);
	}
}


void CViewPattern::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//---------------------------------------------------------------------------
{
	if (nSBCode == (SB_THUMBTRACK|SB_THUMBPOSITION)) m_dwStatus |= PATSTATUS_DRAGVSCROLL;
	CModScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
	if (nSBCode == SB_ENDSCROLL) m_dwStatus &= ~PATSTATUS_DRAGVSCROLL;
}


void CViewPattern::SetCurSel(DWORD dwBegin, DWORD dwEnd)
//------------------------------------------------------
{
	RECT rect1, rect2, rect, rcInt, rcUni;
	POINT pt;
	int x1, y1, x2, y2;
	
	x1 = dwBegin & 0xFFFF;
	y1 = dwBegin >> 16;
	x2 = dwEnd & 0xFFFF;
	y2 = dwEnd >> 16;
	if (x1 > x2)
	{
		int x = x2;
		x2 = x1;
		x1 = x;
	}
	if (y1 > y2)
	{
		int y = y2;
		y2 = y1;
		y1 = y;
	}
	// Get current selection area
	pt = GetPointFromPosition(m_dwBeginSel);
	rect1.left = pt.x;
	rect1.top = pt.y;
	pt = GetPointFromPosition(m_dwEndSel + 0x10001);
	rect1.right = pt.x;
	rect1.bottom = pt.y;
	if (rect1.left < m_szHeader.cx) rect1.left = m_szHeader.cx;
	if (rect1.top < m_szHeader.cy) rect1.top = m_szHeader.cy;
	// Get new selection area
	m_dwBeginSel = (y1 << 16) | (x1);
	m_dwEndSel = (y2 << 16) | (x2);
	pt = GetPointFromPosition(m_dwBeginSel);
	rect2.left = pt.x;
	rect2.top = pt.y;
	pt = GetPointFromPosition(m_dwEndSel + 0x10001);
	rect2.right = pt.x;
	rect2.bottom = pt.y;
	if (rect2.left < m_szHeader.cx) rect2.left = m_szHeader.cx;
	if (rect2.top < m_szHeader.cy) rect2.top = m_szHeader.cy;
	IntersectRect(&rcInt, &rect1, &rect2);
	UnionRect(&rcUni, &rect1, &rect2);
	SubtractRect(&rect, &rcUni, &rcInt);
	if ((rect.left == rcUni.left) && (rect.top == rcUni.top)
	 && (rect.right == rcUni.right) && (rect.bottom == rcUni.bottom))
	{
		InvalidateRect(&rect1, FALSE);
		InvalidateRect(&rect2, FALSE);
	} else
	{
		InvalidateRect(&rect, FALSE);
	}
}


void CViewPattern::InvalidatePattern(BOOL bHdr)
//---------------------------------------------
{
	CRect rect;
	GetClientRect(&rect);
	if (!bHdr)
	{
		rect.left += m_szHeader.cx;
		rect.top += m_szHeader.cy;
	}
	InvalidateRect(&rect, FALSE);
}


void CViewPattern::InvalidateRow(int n)
//-------------------------------------
{
	CModDoc *pModDoc = GetDocument();
	if (pModDoc)
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		int yofs = GetYScrollPos() - m_nMidRow;
		if (n == -1) n = m_nRow;
		if ((n < yofs) || (n >= (int)pSndFile->PatternSize[m_nPattern])) return;
		CRect rect;
		GetClientRect(&rect);
		rect.left = m_szHeader.cx;
		rect.top = m_szHeader.cy;
		rect.top += (n - yofs) * m_szCell.cy;
		rect.bottom = rect.top + m_szCell.cy;
		InvalidateRect(&rect, FALSE);
	}
}


void CViewPattern::InvalidateArea(DWORD dwBegin, DWORD dwEnd)
//-----------------------------------------------------------
{
	RECT rect;
	POINT pt;
	pt = GetPointFromPosition(dwBegin);
	rect.left = pt.x;
	rect.top = pt.y;
	pt = GetPointFromPosition(dwEnd + 0x10001);
	rect.right = pt.x;
	rect.bottom = pt.y;
	InvalidateRect(&rect, FALSE);
}


void CViewPattern::InvalidateChannelsHeaders()
//--------------------------------------------
{
	CRect rect;
	GetClientRect(&rect);
	rect.bottom = rect.top + m_szHeader.cy;
	InvalidateRect(&rect, FALSE);
}


void CViewPattern::UpdateIndicator()
//----------------------------------
{
	CModDoc *pModDoc = GetDocument();
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if ((pMainFrm) && (pModDoc))
	{
		CSoundFile *pSndFile = pModDoc->GetSoundFile();
		CHAR s[256];
		UINT nChn;
		wsprintf(s, "Row %d, Col %d", GetCurrentRow(), GetCurrentChannel()+1);
		pMainFrm->SetUserText(s);
		if (::GetFocus() == m_hWnd)
		{
			nChn = (m_dwCursor & 0xFFFF) >> 3;
			s[0] = 0;
			if ((!(m_dwStatus & (PATSTATUS_KEYDRAGSEL|PATSTATUS_MOUSEDRAGSEL)))
			 && (m_dwBeginSel == m_dwEndSel) && (pSndFile->Patterns[m_nPattern])
			 && (m_nRow < pSndFile->PatternSize[m_nPattern]) && (nChn < pSndFile->m_nChannels))
			{
				MODCOMMAND *m = &pSndFile->Patterns[m_nPattern][m_nRow*pSndFile->m_nChannels+nChn];
				switch (m_dwCursor & 7)
				{
				case 1:
					if (m->instr)
					{
						CHAR sztmp[128] = "";
						if (pSndFile->m_nInstruments)
						{
							if ((m->instr <= pSndFile->m_nInstruments) && (pSndFile->Headers[m->instr]))
							{
								INSTRUMENTHEADER *penv = pSndFile->Headers[m->instr];
								memcpy(sztmp, penv->name, 32);
								sztmp[32] = 0;
								if ((m->note) && (m->note <= 120))
								{
									UINT nsmp = penv->Keyboard[m->note-1];
									if ((nsmp) && (nsmp <= pSndFile->m_nSamples))
									{
										CHAR sztmp2[64] = "";
										memcpy(sztmp2, pSndFile->m_szNames[nsmp], 32);
										sztmp2[32] = 0;
										if (sztmp2[0])
										{
											wsprintf(sztmp+strlen(sztmp), " (%d: %s)", nsmp, sztmp2);
										}
									}
								}
							}
						} else
						{
							if (m->instr <= pSndFile->m_nSamples)
							{
								memcpy(sztmp, pSndFile->m_szNames[m->instr], 32);
								sztmp[32] = 0;
							}
						}
						if (sztmp[0]) wsprintf(s, "%d: %s", m->instr, sztmp);
					}
					break;
				case 2:
					if (!pModDoc->GetVolCmdInfo(pModDoc->GetIndexFromVolCmd(m->volcmd), s)) s[0] = 0;
					break;
				case 3:
				case 4:
					if (!pModDoc->GetEffectName(s, m->command, m->param)) s[0] = 0;
					break;
				}
			}
			pMainFrm->SetInfoText(s);
		}
	}
}


void CViewPattern::UpdateAllVUMeters(MPTNOTIFICATION *pnotify)
//------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	CModDoc *pModDoc = GetDocument();
	CSoundFile *pSndFile;
	CRect rcClient;
	HDC hdc;
	BOOL bPlaying;
	UINT nChn;
	int x, xofs;
	
	if ((!pModDoc) || (!pMainFrm)) return;
	memset(ChnVUMeters, 0, sizeof(ChnVUMeters));
	GetClientRect(&rcClient);
	xofs = GetXScrollPos();
	pSndFile = pModDoc->GetSoundFile();
	hdc = ::GetDC(m_hWnd);
	bPlaying = (pMainFrm->GetFollowSong(pModDoc) == m_hWnd) ? TRUE : FALSE;
	x = m_szHeader.cx;
	nChn = xofs;
	while ((nChn < pSndFile->m_nChannels) && (x < rcClient.right))
	{
		ChnVUMeters[nChn] = (WORD)pnotify->dwPos[nChn];
		if ((!bPlaying) || (pnotify->dwType & MPTNOTIFY_STOP)) ChnVUMeters[nChn] = 0;
		DrawChannelVUMeter(hdc, x + 1, rcClient.top + COLHDR_HEIGHT, nChn);
		nChn++;
		x += m_szCell.cx;
	}
	::ReleaseDC(m_hWnd, hdc);
}





