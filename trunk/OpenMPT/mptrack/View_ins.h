/*
 * view_ins.h
 * ----------
 * Purpose: Instrument tab, lower panel.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

OPENMPT_NAMESPACE_BEGIN

#define INSSTATUS_DRAGGING		0x01
#define INSSTATUS_NCLBTNDOWN	0x02
#define INSSTATUS_SPLITCURSOR	0x04

// Non-Client toolbar buttons
#define ENV_LEFTBAR_BUTTONS		19

//==========================================
class CViewInstrument: public CModScrollView
//==========================================
{
protected:
	CImageList m_bmpEnvBar;
	POINT m_ptMenu;
	CRect m_rcClient;

	CBitmap m_bmpGrid;
	CBitmap m_bmpMemMain;
	CBitmap *m_pbmpOldGrid;
	CBitmap *oldBitmap;

	enmEnvelopeTypes m_nEnv;
	UINT m_nDragItem, m_nBtnMouseOver;
	DWORD m_dwStatus;
	DWORD m_NcButtonState[ENV_LEFTBAR_BUTTONS];

	INSTRUMENTINDEX m_nInstrument;

	CDC m_dcMemMain;
	CDC m_dcGrid;
	int m_GridScrollPos;
	int m_GridSpeed;

	float m_fZoom;

	bool m_bGrid;
	bool m_bGridForceRedraw;

	std::bitset<128> m_baPlayingNote;
	uint32 m_dwNotifyPos[MAX_CHANNELS];

public:
	CViewInstrument();
	DECLARE_SERIAL(CViewInstrument)

protected:
	////////////////////////
	// Envelope get stuff

	// Flags
	bool EnvGetFlag(const EnvelopeFlags dwFlag) const;
	bool EnvGetLoop() const { return EnvGetFlag(ENV_LOOP); };
	bool EnvGetSustain() const { return EnvGetFlag(ENV_SUSTAIN); };
	bool EnvGetCarry() const { return EnvGetFlag(ENV_CARRY); };

	// Misc.
	UINT EnvGetTick(int nPoint) const;
	UINT EnvGetValue(int nPoint) const;
	UINT EnvGetLastPoint() const;
	UINT EnvGetNumPoints() const;

	// Get loop points
	UINT EnvGetLoopStart() const;
	UINT EnvGetLoopEnd() const;
	UINT EnvGetSustainStart() const;
	UINT EnvGetSustainEnd() const;

	// Get envelope status
	bool EnvGetVolEnv() const;
	bool EnvGetPanEnv() const;
	bool EnvGetPitchEnv() const;
	bool EnvGetFilterEnv() const;

	////////////////////////
	// Envelope set stuff

	// Flags
	bool EnvSetFlag(EnvelopeFlags flag, bool enable) const;
	bool EnvSetLoop(bool enable) const {return EnvSetFlag(ENV_LOOP, enable);};
	bool EnvSetSustain(bool enable) const {return EnvSetFlag(ENV_SUSTAIN, enable);};
	bool EnvSetCarry(bool enable) const {return EnvSetFlag(ENV_CARRY, enable);};

	// Misc.
	bool EnvSetValue(int nPoint, int nTick = -1, int nValue = -1, bool moveTail = false);
	bool CanMovePoint(UINT envPoint, int step);

	// Set loop points
	bool EnvSetLoopStart(int nPoint);
	bool EnvSetLoopEnd(int nPoint);
	bool EnvSetSustainStart(int nPoint);
	bool EnvSetSustainEnd(int nPoint);
	bool EnvToggleReleaseNode(int nPoint);

	// Set envelope status
	bool EnvToggleEnv(enmEnvelopeTypes envelope, CSoundFile &sndFile, ModInstrument &ins, bool enable, BYTE defaultValue, EnvelopeFlags extraFlags = EnvelopeFlags(0));
	bool EnvSetVolEnv(bool bEnable);
	bool EnvSetPanEnv(bool bEnable);
	bool EnvSetPitchEnv(bool bEnable);
	bool EnvSetFilterEnv(bool bEnable);

	// Keyboard envelope control
	void EnvKbdSelectPrevPoint();
	void EnvKbdSelectNextPoint();
	void EnvKbdMovePointLeft();
	void EnvKbdMovePointRight();
	void EnvKbdMovePointUp(BYTE stepsize = 1);
	void EnvKbdMovePointDown(BYTE stepsize = 1);
	void EnvKbdInsertPoint();
	void EnvKbdRemovePoint();
	void EnvKbdSetLoopStart();
	void EnvKbdSetLoopEnd();
	void EnvKbdSetSustainStart();
	void EnvKbdSetSustainEnd();
	void EnvKbdToggleReleaseNode();

	bool IsDragItemEnvPoint() const { return !(m_nDragItem < 1 || m_nDragItem > EnvGetLastPoint() + 1); }

	////////////////////////
	// Misc stuff
	void UpdateScrollSize();
	void SetModified(HintType mask, bool updateAll);
	BOOL SetCurrentInstrument(INSTRUMENTINDEX nIns, enmEnvelopeTypes m_nEnv = ENV_VOLUME);
	ModInstrument *GetInstrumentPtr() const;
	InstrumentEnvelope *GetEnvelopePtr() const;
	UINT EnvInsertPoint(int nTick, int nValue);
	bool EnvRemovePoint(UINT nPoint);
	int TickToScreen(int nTick) const;
	int PointToScreen(int nPoint) const;
	int ScreenToTick(int x) const;
	int ScreenToPoint(int x, int y) const;
	int ValueToScreen(int val) const { return m_rcClient.bottom - 1 - (val * (m_rcClient.bottom - 1)) / 64; }
	int ScreenToValue(int y) const;
	void InvalidateEnvelope() { InvalidateRect(NULL, FALSE); }
	void DrawPositionMarks();
	void DrawNcButton(CDC *pDC, UINT nBtn);
	BOOL GetNcButtonRect(UINT nBtn, LPRECT lpRect);
	void UpdateNcButtonState();
	void PlayNote(ModCommand::NOTE note);
	void DrawGrid(CDC *memDC, UINT speed);

	void OnEnvZoomIn() { EnvSetZoom(m_fZoom + 1); };
	void OnEnvZoomOut() { EnvSetZoom(m_fZoom - 1); };
	void EnvSetZoom(float fNewZoom);

public:
	//{{AFX_VIRTUAL(CViewInstrument)
	virtual void OnDraw(CDC *);
	virtual void OnInitialUpdate();
	virtual void UpdateView(UpdateHint hint = HINT_NONE, CObject *pObj = nullptr);
	virtual LRESULT OnModViewMsg(WPARAM, LPARAM);
	virtual BOOL OnDragonDrop(BOOL, const DRAGONDROP *);
	virtual LRESULT OnPlayerNotify(Notification *);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CViewInstrument)
	afx_msg BOOL OnEraseBkgnd(CDC *) { return TRUE; }
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
#if _MFC_VER > 0x0710
	afx_msg LRESULT OnNcHitTest(CPoint point);
#else
	afx_msg UINT OnNcHitTest(CPoint point);
#endif 
	afx_msg void OnNcPaint();
	afx_msg void OnPrevInstrument();
	afx_msg void OnNextInstrument();
	afx_msg void OnMouseMove(UINT, CPoint);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnMButtonDown(UINT, CPoint);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT, CPoint);
	afx_msg void OnNcLButtonUp(UINT, CPoint);
	afx_msg void OnNcLButtonDblClk(UINT, CPoint);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnvLoopChanged();
	afx_msg void OnEnvSustainChanged();
	afx_msg void OnEnvCarryChanged();
	afx_msg void OnEnvToggleReleasNode();
	afx_msg void OnEnvInsertPoint();
	afx_msg void OnEnvRemovePoint();
	afx_msg void OnSelectVolumeEnv();
	afx_msg void OnSelectPanningEnv();
	afx_msg void OnSelectPitchEnv();
	afx_msg void OnEnvVolChanged();
	afx_msg void OnEnvPanChanged();
	afx_msg void OnEnvPitchChanged();
	afx_msg void OnEnvFilterChanged();
	afx_msg void OnEnvToggleGrid(); //rewbs.envRowGrid
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditSampleMap();
	afx_msg void OnEnvelopeScalepoints();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LRESULT OnCustomKeyMsg(WPARAM, LPARAM); //rewbs.customKeys
	afx_msg LRESULT OnMidiMsg(WPARAM, LPARAM);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual BOOL PreTranslateMessage(MSG *pMsg); //rewbs.customKeys
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	uint8 EnvGetReleaseNode();
	uint16 EnvGetReleaseNodeValue();
	uint16 EnvGetReleaseNodeTick();
};


OPENMPT_NAMESPACE_END
