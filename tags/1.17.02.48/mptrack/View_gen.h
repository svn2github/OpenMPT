#ifndef _VIEW_GLOBALS_H_
#define _VIEW_GLOBALS_H_

//==================================
class CViewGlobals: public CFormView
//==================================
{
protected:
	CRect m_rcClient;
	CTabCtrl m_TabCtrl;
	CComboBox m_CbnEffects[4];
	CComboBox m_CbnPlugin, m_CbnParam, m_CbnOutput;

	CSliderCtrl m_sbVolume[4], m_sbPan[4], m_sbValue, m_sbDryRatio;

// -> CODE#0002
// -> DESC="VST plugins presets"
	CComboBox m_CbnPreset;
// -! NEW_FEATURE#0002
//	CSliderCtrl m_sbVolume[4], m_sbPan[4], m_sbValue;
// -> CODE#0014
// -> DESC="vst wet/dry slider"
	CSliderCtrl m_sbWetDry;
// -! NEW_FEATURE#0014
	CSpinButtonCtrl m_spinVolume[4], m_spinPan[4];
	CButton m_BtnSelect, m_BtnEdit;
	int m_nActiveTab, m_nLockCount;
	UINT m_nCurrentPlugin, m_nCurrentParam;
// -> CODE#0002
// -> DESC="VST plugins presets"
	UINT m_nCurrentPreset;
// -! NEW_FEATURE#0002

// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
	CComboBox m_CbnSpecialMixProcessing;
	CSpinButtonCtrl m_SpinMixGain;			// update#02
// -! NEW_FEATURE#0028

protected:
	CViewGlobals():CFormView(IDD_VIEW_GLOBALS) { m_nLockCount = 1; }
	DECLARE_SERIAL(CViewGlobals)

public:
	virtual ~CViewGlobals() {}

public:
	CModDoc* GetDocument() const { return (CModDoc *)m_pDocument; }
	void RecalcLayout();
	void LockControls() { m_nLockCount++; }
	void UnlockControls() { PostMessage(WM_MOD_UNLOCKCONTROLS); }
	BOOL IsLocked() const { return (m_nLockCount > 0); }
	int GetDlgItemIntEx(UINT nID);
	void BuildEmptySlotList(CArray<UINT, UINT> &emptySlots);
	bool MovePlug(UINT src, UINT dest);

public:
	//{{AFX_VIRTUAL(CViewGlobals)
	virtual void OnInitialUpdate();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void UpdateView(DWORD dwHintMask=0, CObject *pObj=NULL);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT OnModViewMsg(WPARAM, LPARAM);
// -> CODE#0015
// -> DESC="channels management dlg"
	virtual void OnDraw(CDC* pDC);
// -! NEW_FEATURE#0015
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CViewGlobals)
	afx_msg void OnMute1();
	afx_msg void OnMute2();
	afx_msg void OnMute3();
	afx_msg void OnMute4();
	afx_msg void OnSurround1();
	afx_msg void OnSurround2();
	afx_msg void OnSurround3();
	afx_msg void OnSurround4();
	afx_msg void OnEditVol1();
	afx_msg void OnEditVol2();
	afx_msg void OnEditVol3();
	afx_msg void OnEditVol4();
	afx_msg void OnEditPan1();
	afx_msg void OnEditPan2();
	afx_msg void OnEditPan3();
	afx_msg void OnEditPan4();
	afx_msg void OnEditName1();
	afx_msg void OnEditName2();
	afx_msg void OnEditName3();
	afx_msg void OnEditName4();
	afx_msg void OnFx1Changed();
	afx_msg void OnFx2Changed();
	afx_msg void OnFx3Changed();
	afx_msg void OnFx4Changed();
	afx_msg void OnPluginChanged();
	afx_msg void OnPluginNameChanged();
	afx_msg void OnParamChanged();
// -> CODE#0002
// -> DESC="VST plugins presets"
	afx_msg void OnProgramChanged();
	afx_msg void OnLoadParam();
	afx_msg void OnSaveParam();
// -! NEW_FEATURE#0002
	afx_msg void OnSelectPlugin();
	afx_msg void OnSetParameter();
// -> CODE#0014
// -> DESC="vst wet/dry slider"
	afx_msg void OnSetWetDry();
//	afx_msg void OnWetDryChanged();
// -! NEW_FEATURE#0014
	afx_msg void OnEditPlugin();
	afx_msg void OnMixModeChanged();
	afx_msg void OnBypassChanged();
	afx_msg void OnDryMixChanged();
	afx_msg void OnMovePlugToSlot();
	afx_msg void OnInsertSlot();
	afx_msg void OnClonePlug();

// -> CODE#0028
// -> DESC="effect plugin mixing mode combo"
	afx_msg void OnWetDryExpandChanged();
	afx_msg void OnSpecialMixProcessingChanged();
// -! NEW_FEATURE#0028

	afx_msg void OnOutputRoutingChanged();
	afx_msg void OnPrevPlugin();
	afx_msg void OnNextPlugin();
	afx_msg void OnDestroy();
	afx_msg void OnFxCommands(UINT id);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTabSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUnlockControls(WPARAM, LPARAM) { if (m_nLockCount > 0) m_nLockCount--; return 0; }
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif