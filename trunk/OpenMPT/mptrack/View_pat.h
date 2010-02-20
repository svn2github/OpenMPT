#ifndef _VIEW_PATTERNS_H_
#define _VIEW_PATTERNS_H_

class CModDoc;
class CEditCommand;
class CEffectVis;	//rewbs.fxvis
class CPatternGotoDialog;
class CPatternRandomizer;
class COpenGLEditor;

// Drag & Drop info
#define DRAGITEM_MASK			0xFF0000
#define DRAGITEM_CHNHEADER		0x010000
#define DRAGITEM_PATTERNHEADER	0x020000
#define DRAGITEM_PLUGNAME		0x040000	//rewbs.patPlugName

#define PATSTATUS_MOUSEDRAGSEL			0x01
#define PATSTATUS_KEYDRAGSEL			0x02
#define PATSTATUS_FOCUS					0x04
#define PATSTATUS_FOLLOWSONG			0x08
#define PATSTATUS_RECORD				0x10
#define PATSTATUS_DRAGHSCROLL			0x20
#define PATSTATUS_DRAGVSCROLL			0x40
#define PATSTATUS_VUMETERS				0x80
#define PATSTATUS_CHORDPLAYING			0x100
#define PATSTATUS_DRAGNDROPEDIT			0x200
#define PATSTATUS_DRAGNDROPPING			0x400
#define PATSTATUS_MIDISPACINGPENDING	0x800
#define PATSTATUS_CTRLDRAGSEL			0x1000
#define PATSTATUS_PLUGNAMESINHEADERS	0x2000 //rewbs.patPlugName
#define PATSTATUS_PATTERNLOOP			0x4000

// Row Spacing
#define MAX_SPACING	64

enum {
	NOTE_COLUMN=0,
    INST_COLUMN,
	VOL_COLUMN,
	EFFECT_COLUMN,
	PARAM_COLUMN,
	LAST_COLUMN = PARAM_COLUMN
};


//Struct for controlling selection clearing. This is used to define which data fields
//should be cleared.
struct RowMask
{
      bool note;
      bool instrument;
      bool volume;
      bool command;
      bool parameter;
};
const RowMask DefaultRowMask = {true, true, true, true, true};

struct ModCommandPos
{
	PATTERNINDEX nPat;
	ROWINDEX nRow;
	CHANNELINDEX nChn;
};


//////////////////////////////////////////////////////////////////
// Pattern editing class


//=======================================
class CViewPattern: public CModScrollView
//=======================================
{
public:
	// Find/Replace
	static MODCOMMAND m_cmdFind, m_cmdReplace, m_cmdOld;
	static DWORD m_dwFindFlags, m_dwReplaceFlags;
	static UINT m_nFindMinChn, m_nFindMaxChn;
	signed char m_cInstrRelChange; // relative instrument change (quick'n'dirty fix, this should be implemented in a less cryptic way)

protected:
	CFastBitmap m_Dib;
	CEditCommand *m_pEditWnd;
	CPatternGotoDialog *m_pGotoWnd;
	SIZE m_szHeader, m_szCell;
	UINT m_nPattern, m_nRow, m_nMidRow, m_nPlayPat, m_nPlayRow, m_nSpacing, m_nAccelChar, m_nLastPlayedRow, m_nLastPlayedOrder;

	int m_nXScroll, m_nYScroll;
	DWORD m_nDragItem, m_nMenuParam, m_nDetailLevel;
	BOOL m_bDragging, m_bInItemRect, m_bRecord, m_bContinueSearch, m_bWholePatternFitsOnScreen;
	RECT m_rcDragItem;
	DWORD m_dwStatus, m_dwCursor;
	DWORD m_dwBeginSel, m_dwEndSel, m_dwStartSel, m_dwDragPos;
	WORD ChnVUMeters[MAX_BASECHANNELS];
	WORD OldVUMeters[MAX_BASECHANNELS];
	CListBox *ChnEffectList[MAX_BASECHANNELS]; //rewbs.patPlugName
	BYTE MultiRecordMask[(MAX_CHANNELS+7)/8];
	UINT m_nFoundInstrument;
	UINT m_nMenuOnChan;
	DWORD m_dwLastNoteEntryTime; //rewbs.customkeys
	UINT m_nLastPlayedChannel; //rewbs.customkeys
	bool m_bLastNoteEntryBlocked;

// -> CODE#0012
// -> DESC="midi keyboard split"
	BYTE activeNoteChannel[NOTE_MAX + 1];
	BYTE splitActiveNoteChannel[NOTE_MAX + 1];
	int oldrow,oldchn,oldsplitchn;
// -! NEW_FEATURE#0012

// -> CODE#0018
// -> DESC="route PC keyboard inputs to midi in mechanism"
	int ignorekey;
// -! BEHAVIOUR_CHANGE#0018
	CPatternRandomizer *m_pRandomizer;	//rewbs.fxVis
public:
	CEffectVis    *m_pEffectVis;	//rewbs.fxVis
	COpenGLEditor *m_pOpenGLEditor;	//rewbs.fxVis


	CViewPattern();
	DECLARE_SERIAL(CViewPattern)

public:

	BOOL UpdateSizes();
	void UpdateScrollSize();
	void UpdateScrollPos();
	void UpdateIndicator();
	void UpdateXInfoText(); //rewbs.xinfo
	void UpdateColors();
	int GetXScrollPos() const { return m_nXScroll; }
	int GetYScrollPos() const { return m_nYScroll; }
	int GetColumnWidth() const { return m_szCell.cx; }
	int GetColumnHeight() const { return m_szCell.cy; }
	UINT GetCurrentPattern() const { return m_nPattern; }
	UINT GetCurrentRow() const { return m_nRow; }
	UINT GetCurrentColumn() const { return m_dwCursor; }
	UINT GetCurrentChannel() const { return (m_dwCursor >> 3); }
	UINT GetColumnOffset(DWORD dwPos) const;
	POINT GetPointFromPosition(DWORD dwPos);
	DWORD GetPositionFromPoint(POINT pt);
	DWORD GetDragItem(CPoint point, LPRECT lpRect);
	void InvalidatePattern(BOOL bHdr=FALSE);
	void InvalidateRow(int n=-1);
	void InvalidateArea(DWORD dwBegin, DWORD dwEnd);
	void InvalidateSelection() { InvalidateArea(m_dwBeginSel, m_dwEndSel); }
	void InvalidateChannelsHeaders();
	void SetCurSel(DWORD dwBegin, DWORD dwEnd);
	BOOL SetCurrentPattern(UINT npat, int nrow=-1);
	BOOL SetCurrentRow(UINT nrow, BOOL bWrap=FALSE, BOOL bUpdateHorizontalScrollbar=TRUE );
	BOOL SetCurrentColumn(UINT ncol);
	// This should be used instead of consecutive calls to SetCurrentRow() then SetCurrentColumn()
	BOOL SetCursorPosition(UINT nrow, UINT ncol, BOOL bWrap=FALSE );
	BOOL DragToSel(DWORD dwPos, BOOL bScroll, BOOL bNoMove=FALSE);
	BOOL SetPlayCursor(UINT nPat, UINT nRow);
	BOOL UpdateScrollbarPositions( BOOL bUpdateHorizontalScrollbar=TRUE );
// -> CODE#0014
// -> DESC="vst wet/dry slider"
//	BOOL EnterNote(UINT nNote, UINT nIns=0, BOOL bCheck=FALSE, int vol=-1, BOOL bMultiCh=FALSE);
	BYTE EnterNote(UINT nNote, UINT nIns=0, BOOL bCheck=FALSE, int vol=-1, BOOL bMultiCh=FALSE);
// -! NEW_FEATURE#0014// -> CODE#0012
	BOOL ShowEditWindow();
	UINT GetCurrentInstrument() const;

	BOOL TransposeSelection(int transp);
	BOOL PrepareUndo(DWORD dwBegin, DWORD dwEnd);
	void DeleteRows(UINT colmin, UINT colmax, UINT nrows);
	void OnDropSelection();
	void ProcessChar(UINT nChar, UINT nFlags);

public:
	void DrawPatternData(HDC, CSoundFile *, UINT, BOOL, BOOL, UINT, UINT, UINT, CRect&, int *);
	void DrawLetter(int x, int y, char letter, int sizex=10, int ofsx=0);
	void DrawNote(int x, int y, UINT note, CTuning* pTuning = NULL);
	void DrawInstrument(int x, int y, UINT instr);
	void DrawVolumeCommand(int x, int y, const MODCOMMAND mc);
	void DrawChannelVUMeter(HDC hdc, int x, int y, UINT nChn);
	void UpdateAllVUMeters(MPTNOTIFICATION *pnotify);
	void DrawDragSel(HDC hdc);
	void OnDrawDragSel();

	//rewbs.customKeys
	BOOL ExecuteCommand(CommandID command);
	void CursorJump(DWORD distance, bool direction, bool snap);
	void TempEnterNote(int n, bool oldStyle = false, int vol = -1);
	void TempStopNote(int note, bool fromMidi=false, const bool bChordMode=false);
	void TempEnterChord(int n);
	void TempStopChord(int note) {TempStopNote(note, false, true);}
	void TempEnterIns(int val);
	void TempEnterOctave(int val);
	void TempEnterVol(int v);
	void TempEnterFX(int v);
	void TempEnterFXparam(int v);
	void SetSpacing(int n);
	void OnClearField(int, bool, bool=false);
	void InsertRows(UINT colmin, UINT colmax);
	//end rewbs.customKeys

	void TogglePluginEditor(int chan); //rewbs.patPlugName

	void ExecutePaste(enmPatternPasteModes pasteMode);

public:
	//{{AFX_VIRTUAL(CViewPattern)
	virtual void OnDraw(CDC *);
	virtual void OnInitialUpdate();
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual void UpdateView(DWORD dwHintMask=0, CObject *pObj=NULL);
	virtual LRESULT OnModViewMsg(WPARAM, LPARAM);
	virtual LRESULT OnPlayerNotify(MPTNOTIFICATION *);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CViewPattern)
	afx_msg BOOL OnEraseBkgnd(CDC *) { return TRUE; }
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT, CPoint);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnLButtonDblClk(UINT, CPoint);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();

	afx_msg void OnEditPaste() {ExecutePaste(pm_overwrite);};
	afx_msg void OnEditMixPaste() {ExecutePaste(pm_mixpaste);};
	afx_msg void OnEditMixPasteITStyle() {ExecutePaste(pm_mixpaste_it);};
	afx_msg void OnEditPasteFlood() {ExecutePaste(pm_pasteflood);};
	afx_msg void OnEditPushForwardPaste() {ExecutePaste(pm_pushforwardpaste);};

	afx_msg void OnClearSelection(bool ITStyle=false, RowMask sb = DefaultRowMask); //rewbs.customKeys
	afx_msg void OnGrowSelection();   //rewbs.customKeys
	afx_msg void OnShrinkSelection(); //rewbs.customKeys
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditSelectColumn();
	afx_msg void OnSelectCurrentColumn();
	afx_msg void OnEditFind();
	afx_msg void OnEditGoto();
	afx_msg void OnEditFindNext();
	afx_msg void OnEditUndo();
	afx_msg void OnChannelReset();
	afx_msg void OnMuteFromClick(); //rewbs.customKeys
	afx_msg void OnSoloFromClick(); //rewbs.customKeys
	afx_msg void OnTogglePendingMuteFromClick(); //rewbs.customKeys
	afx_msg void OnPendingSoloChnFromClick();
	afx_msg void OnPendingUnmuteAllChnFromClick();
	afx_msg void OnSoloChannel(BOOL current); //rewbs.customKeys
	afx_msg void OnMuteChannel(BOOL current); //rewbs.customKeys
	afx_msg void OnUnmuteAll();
	afx_msg void OnRecordSelect();
// -> CODE#0012
// -> DESC="midi keyboard split"
	afx_msg void OnSplitRecordSelect();
// -! NEW_FEATURE#0012
	afx_msg void OnDeleteRows();
	afx_msg void OnDeleteRowsEx();
	afx_msg void OnInsertRows();
	afx_msg void OnPatternStep();
	afx_msg void OnSwitchToOrderList();
	afx_msg void OnPrevOrder();
	afx_msg void OnNextOrder();
	afx_msg void OnPrevInstrument() { PostCtrlMessage(CTRLMSG_PAT_PREVINSTRUMENT); }
	afx_msg void OnNextInstrument() { PostCtrlMessage(CTRLMSG_PAT_NEXTINSTRUMENT); }
//rewbs.customKeys - now implemented at ModDoc level
/*	afx_msg void OnPatternRestart() {}
	afx_msg void OnPatternPlay()	{}
	afx_msg void OnPatternPlayNoLoop()	{} */
//end rewbs.customKeys
	afx_msg void OnPatternRecord()	{ PostCtrlMessage(CTRLMSG_SETRECORD, -1); }
	afx_msg void OnInterpolateVolume();
	afx_msg void OnInterpolateEffect();
	afx_msg void OnInterpolateNote();
	afx_msg void OnVisualizeEffect();		//rewbs.fxvis
	afx_msg void OnOpenRandomizer();		//rewbs.fxvis
	afx_msg void OnTransposeUp();
	afx_msg void OnTransposeDown();
	afx_msg void OnTransposeOctUp();
	afx_msg void OnTransposeOctDown();
	afx_msg void OnSetSelInstrument();
	afx_msg void OnAddChannelFront();
	afx_msg void OnAddChannelAfter();
	afx_msg void OnDuplicateChannel();
	afx_msg void OnRemoveChannel();
	afx_msg void OnRemoveChannelDialog();
	afx_msg void OnPatternProperties();
	afx_msg void OnCursorCopy();
	afx_msg void OnCursorPaste();
	afx_msg void OnPatternAmplify();
	afx_msg void OnUpdateUndo(CCmdUI *pCmdUI);
	afx_msg void OnSelectPlugin(UINT nID);  //rewbs.patPlugName
	afx_msg LRESULT OnUpdatePosition(WPARAM nOrd, LPARAM nRow);
	afx_msg LRESULT OnMidiMsg(WPARAM, LPARAM);
	afx_msg LRESULT OnRecordPlugParamChange(WPARAM, LPARAM);
	afx_msg LRESULT OnCustomKeyMsg(WPARAM, LPARAM); //rewbs.customKeys
	afx_msg void OnClearSelectionFromMenu();
	afx_msg void OnSelectInstrument(UINT nid);
	afx_msg void OnRunScript();
	afx_msg void OnShowTimeAtRow();
	afx_msg void OnRenameChannel();
	afx_msg void OnTogglePCNotePluginEditor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnInitMenu(CMenu* pMenu);
private:

	void SetSplitKeyboardSettings();
	bool HandleSplit(MODCOMMAND* p, int note);
	bool BuildChannelControlCtxMenu(HMENU hMenu);
	bool BuildPluginCtxMenu(HMENU hMenu, UINT nChn, CSoundFile* pSndFile);
	bool BuildRecordCtxMenu(HMENU hMenu, UINT nChn, CModDoc* pModDoc);
	bool BuildSoloMuteCtxMenu(HMENU hMenu, CInputHandler* ih, UINT nChn, CSoundFile* pSndFile);
	bool BuildRowInsDelCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildMiscCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildSelectionCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildGrowShrinkCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildNoteInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile);
	bool BuildVolColInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile);
	bool BuildEffectInterpolationCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile);
	bool BuildEditCtxMenu(HMENU hMenu, CInputHandler* ih,  CModDoc* pModDoc);
	bool BuildVisFXCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildRandomCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildTransposeCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildSetInstCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile);
	bool BuildAmplifyCtxMenu(HMENU hMenu, CInputHandler* ih);
	bool BuildChannelMiscCtxMenu(HMENU hMenu, CSoundFile* pSndFile);
	bool BuildPCNoteCtxMenu(HMENU hMenu, CInputHandler* ih, CSoundFile* pSndFile);

	UINT GetSelectionStartRow();
	UINT GetSelectionEndRow();
	UINT GetSelectionStartChan();
	UINT GetSelectionEndChan();
	UINT ListChansWhereColSelected(UINT colType, CArray<UINT,UINT> &chans);

	static ROWINDEX GetRowFromCursor(DWORD cursor);
	static CHANNELINDEX GetChanFromCursor(DWORD cursor);
	static UINT GetColTypeFromCursor(DWORD cursor);

	bool IsInterpolationPossible(UINT startRow, UINT endRow, UINT chan, UINT colType, CSoundFile* pSndFile);
	void Interpolate(UINT type);

	// Return true if recording live (i.e. editing while following playback).
	// rSndFile must be the CSoundFile object of given rModDoc.
	bool IsLiveRecord(const CModDoc& rModDoc, const CSoundFile& rSndFile) const;
	bool IsLiveRecord(const CMainFrame& rMainFrm, const CModDoc& rModDoc, const CSoundFile& rSndFile) const;

	// If given edit positions are valid, sets them to iRow and iPat.
	// If not valid, set edit cursor position.
	void SetEditPos(const CSoundFile& rSndFile, 
					ROWINDEX& iRow, PATTERNINDEX& iPat,
					const ROWINDEX iRowCandidate, const PATTERNINDEX iPatCandidate) const;

	// Returns edit position.
	ModCommandPos GetEditPos(CSoundFile& rSf, const bool bLiveRecord) const;

	// Returns pointer to modcommand at given position. If the position is not valid, returns pointer
	// to a dummy command.
	MODCOMMAND* GetModCommand(CSoundFile& rSf, const ModCommandPos& pos);

	bool IsEditingEnabled() const {return ((m_dwStatus&PATSTATUS_RECORD) != 0);}

	//Like IsEditingEnabled(), but shows some notification when editing is not enabled.
	bool IsEditingEnabled_bmsg();
	

public:
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
private:

	void TogglePendingMute(UINT nChn);
	void PendingSoloChn(const CHANNELINDEX nChn);
	void PendingUnmuteAllChn();

public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};


inline bool CViewPattern::IsLiveRecord(const CMainFrame& rMainFrm, const CModDoc& rModDoc, const CSoundFile& rSndFile) const
//----------------------------------------------------------------------------
{   //       (following song) && (following in correct document(?))  && (playback is on)
	return ((m_dwStatus & PATSTATUS_FOLLOWSONG) &&	(rMainFrm.GetFollowSong(&rModDoc) == m_hWnd) && !(rSndFile.IsPaused()));
}


inline bool CViewPattern::IsLiveRecord(const CModDoc& rModDoc, const CSoundFile& rSndFile) const
//----------------------------------------------------------------------------
{
	return IsLiveRecord(*CMainFrame::GetMainFrame(), rModDoc, rSndFile);
}

#endif



