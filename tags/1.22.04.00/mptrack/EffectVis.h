/*
 * EffectVis.h
 * -----------
 * Purpose: Implementation of parameter visualisation dialog.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "afxwin.h"
#include "EffectInfo.h"
class CViewPattern;
//class CModScrollView;

#define FXVSTATUS_LDRAGGING		0x01
#define FXVSTATUS_RDRAGGING		0x02
//#define FXVSTATUS_NCLBTNDOWN	0x02
//#define INSSTATUS_SPLITCURSOR	0x04

// EffectVis dialog
class CEffectVis : public CDialog
{
	DECLARE_DYNAMIC(CEffectVis)

public:
	enum 
	{
		kAction_OverwriteFX=0,
		kAction_FillFX,
		kAction_OverwritePC,
		kAction_FillPC,
		kAction_Preserve
	};

	CEffectVis(CViewPattern *pViewPattern, UINT startRow, UINT endRow, UINT nchn, CModDoc* pModDoc, UINT pat);
	virtual ~CEffectVis();
	//{{AFX_VIRTUAL(CEffectVis)
	//virtual void OnDraw(CDC *);
	//}}AFX_VIRTUAL

// Dialog Data
	enum { IDD = IDD_EFFECTVISUALIZER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//CFastBitmap m_Dib;

	EffectInfo effectInfo;

	CBitmap m_bGrid, m_bNodes, m_bPlayPos;
	CBitmap *m_pbOldGrid, *m_pbOldNodes, *m_pbOldPlayPos;
	CDC m_dcGrid, m_dcNodes, m_dcPlayPos;
	//LPMODPLUGDIB nodeBMP;
	void DrawNodes();
	void DrawGrid();

	void ShowVis(CDC * pDC, CRect rectBorder);
	void ShowVisImage(CDC *pDC);
	BOOL m_boolForceRedraw, m_boolUseBitmaps;
	RECT invalidated;
	
	int m_nLastDrawnRow; // for interpolation
	long m_nLastDrawnY; // for interpolation
	int m_nRowToErase;
	int m_nParamToErase;

	UINT m_nOldPlayPos;
	ModCommand m_templatePCNote;

	CBrush m_brushBlack;

public:
	UINT m_startRow;
    UINT m_endRow;
	UINT m_nRows;
	CHANNELINDEX m_nChan;
	UINT m_nPattern;
	long m_nFillEffect, m_nAction;

    int m_nDragItem;
	UINT m_nBtnMouseOver;
	DWORD m_dwStatus;

	void InvalidateRow(int row);
	float m_pixelsPerRow, m_pixelsPerFXParam, m_pixelsPerPCParam;
	void UpdateSelection(UINT startRow, UINT endRow, UINT nchn, CModDoc* m_pModDoc, UINT pats);
	void Update();
	int RowToScreenX(UINT row);
	int RowToScreenY(UINT row);
	int PCParamToScreenY(uint16 param);
	int FXParamToScreenY(uint16 param);
	uint16 GetParam(UINT row);
	BYTE GetCommand(UINT row);
	void SetParamFromY(UINT row, long y);
	void SetCommand(UINT row, BYTE cmd);
	BYTE ScreenYToFXParam(int y);
	uint16 ScreenYToPCParam(int y);
	UINT ScreenXToRow(int x);
	void SetPlayCursor(UINT nPat, UINT nRow);
	bool IsPcNote(int row);
	void SetPcNote(int row);

	CSoundFile* m_pSndFile;
	CModDoc* m_pModDoc;
	CRect m_rcDraw;
	CRect m_rcFullWin;

	CComboBox m_cmbEffectList, m_cmbActionList;
	CEdit m_edVisStatus;

	virtual VOID OnOK();
	virtual VOID OnCancel();
	BOOL OpenEditor(CWnd *parent);
	VOID DoClose();
	afx_msg void OnClose();
	LONG* GetSplitPosRef() {return NULL;} 	//rewbs.varWindowSize

	CViewPattern *m_pViewPattern;
	

	DECLARE_MESSAGE_MAP()
	BOOL OnInitDialog();
	afx_msg void OnPaint();
	
public: //HACK for first window repos
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEffectChanged();
	afx_msg void OnActionChanged();
	//{{AFX_MSG(CEffectVis)
	afx_msg void OnEditUndo();
	//}}AFX_MSG

private:

	void MakeChange(int currentRow, long newY);
};
