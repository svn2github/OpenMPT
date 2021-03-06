/*
 * view_tre.h
 * ----------
 * Purpose: Tree view for managing open songs, sound files, file browser, ...
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

class CModDoc;
class CModTree;

#include <vector>
#include <bitset>

using std::vector;
using std::bitset;

#define TREESTATUS_RDRAG			0x01
#define TREESTATUS_LDRAG			0x02
#define TREESTATUS_SINGLEEXPAND		0x04
#define TREESTATUS_DRAGGING			(TREESTATUS_RDRAG|TREESTATUS_LDRAG)

struct ModTreeDocInfo
{
	CModDoc *pModDoc;
	// Tree state variables
	vector<vector<HTREEITEM> > tiOrders;
	vector<HTREEITEM> tiSequences, tiPatterns;
	HTREEITEM hSong, hPatterns, hSamples, hInstruments, hComments, hOrders, hEffects;
	HTREEITEM tiSamples[MAX_SAMPLES];
	HTREEITEM tiInstruments[MAX_INSTRUMENTS];
	HTREEITEM tiEffects[MAX_MIXPLUGINS];

	// Module information
	ORDERINDEX nOrdSel;
	SEQUENCEINDEX nSeqSel;

	bitset<MAX_SAMPLES> samplesPlaying;
	bitset<MAX_INSTRUMENTS> instrumentsPlaying;
	
	ModTreeDocInfo(const CSoundFile &sndFile)
	{
		pModDoc = sndFile.GetpModDoc();
		nSeqSel = SEQUENCEINDEX_INVALID;
		nOrdSel = ORDERINDEX_INVALID;
		hSong = hPatterns = hSamples = hInstruments = hComments = hOrders = hEffects = nullptr;
		tiPatterns.resize(sndFile.Patterns.Size(), nullptr);
		tiOrders.resize(sndFile.Order.GetNumSequences());
		tiSequences.resize(sndFile.Order.GetNumSequences(), nullptr);
		MemsetZero(tiSamples);
		MemsetZero(tiInstruments);
		MemsetZero(tiEffects);
		samplesPlaying.reset();
		instrumentsPlaying.reset();
	}
};


//=============================================
class CModTreeDropTarget: public COleDropTarget
//=============================================
{
protected:
	CModTree *m_pModTree;

public:
	CModTreeDropTarget() { m_pModTree = NULL; }
	BOOL Register(CModTree *pWnd);

public:
	virtual DROPEFFECT OnDragEnter(CWnd *pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd *pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd *pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};


//==============================
class CModTree: public CTreeCtrl
//==============================
{
protected:

	enum ModItemType
	{
		MODITEM_NULL = 0,
		MODITEM_ORDER,
		MODITEM_PATTERN,
		MODITEM_SAMPLE,
		MODITEM_INSTRUMENT,
		MODITEM_COMMENTS,
		MODITEM_EFFECT,
		MODITEM_HDR_SONG,
		MODITEM_HDR_ORDERS,
		MODITEM_HDR_PATTERNS,
		MODITEM_HDR_SAMPLES,
		MODITEM_HDR_INSTRUMENTS,
		MODITEM_HDR_INSTRUMENTLIB,
		MODITEM_HDR_MIDILIB,
		MODITEM_HDR_MIDIGROUP,
		MODITEM_MIDIINSTRUMENT,
		MODITEM_MIDIPERCUSSION,
		MODITEM_INSLIB_FOLDER,
		MODITEM_INSLIB_SAMPLE,
		MODITEM_INSLIB_INSTRUMENT,
		MODITEM_INSLIB_SONG,
		MODITEM_DLSBANK_FOLDER,
		MODITEM_DLSBANK_INSTRUMENT,
		MODITEM_SEQUENCE,
	};

	static CSoundFile *m_SongFile;	// For browsing samples and instruments inside modules on disk
	CModTreeDropTarget m_DropTarget;
	CModTree *m_pDataTree;	// Pointer to instrument browser (lower part of tree view) - if it's a nullptr, this object is the instrument browser itself.
	DWORD m_dwStatus;
	HWND m_hDropWnd;
	uint64 m_qwItemDrag;
	UINT m_nDocNdx, m_nDragDocNdx;
	HTREEITEM m_hItemDrag, m_hItemDrop;
	HTREEITEM m_hInsLib, m_hMidiLib;
	HTREEITEM m_tiMidiGrp[17];
	HTREEITEM m_tiMidi[128];
	HTREEITEM m_tiPerc[128];
	vector<HTREEITEM> m_tiDLS;
	vector<ModTreeDocInfo *> DocInfo;
	// Instrument library
	bool m_bShowAllFiles, doLabelEdit;
	CHAR m_szInstrLibPath[_MAX_PATH], m_szOldPath[_MAX_PATH], m_szSongName[_MAX_PATH];

public:
	CModTree(CModTree *pDataTree);
	virtual ~CModTree();

// Attributes
public:
	void Init();
	void InsLibSetFullPath(LPCSTR pszLibPath, LPCSTR pszSongFolder);
	void InsLibGetFullPath(HTREEITEM hItem, LPSTR pszFullPath) const;
	void RefreshMidiLibrary();
	void RefreshDlsBanks();
	void RefreshInstrumentLibrary();
	void EmptyInstrumentLibrary();
	void FillInstrumentLibrary();
	uint64 GetModItem(HTREEITEM hItem);
	ModItemType GetModItemType(const uint64 modItem) {return static_cast<ModItemType>(modItem & 0xFFFF);};	// return "item type" part of mod item variable ( & 0xFFFF )
	uint32 GetModItemID(const uint64 modItem) {return static_cast<uint32>(modItem >> 16);};		// return "item ID" part of mod item variable ( >> 16 )
	BOOL SetMidiInstrument(UINT nIns, LPCTSTR lpszFileName);
	BOOL SetMidiPercussion(UINT nPerc, LPCTSTR lpszFileName);
	BOOL ExecuteItem(HTREEITEM hItem);
	BOOL DeleteTreeItem(HTREEITEM hItem);
	BOOL PlayItem(HTREEITEM hItem, ModCommand::NOTE nParam);
	BOOL OpenTreeItem(HTREEITEM hItem);
	BOOL OpenMidiInstrument(DWORD dwItem);
	BOOL InstrumentLibraryChDir(LPCSTR lpszDir);
	BOOL GetDropInfo(LPDRAGONDROP pdropinfo, LPSTR lpszPath);
	void OnOptionsChanged();
	void AddDocument(CModDoc *pModDoc);
	void RemoveDocument(CModDoc *pModDoc);
	void UpdateView(ModTreeDocInfo *pInfo, DWORD dwHint);
	void OnUpdate(CModDoc *pModDoc, DWORD dwHint, CObject *pHint);
	bool CanDrop(HTREEITEM hItem, bool bDoDrop);
	void UpdatePlayPos(CModDoc *pModDoc, Notification *pNotify);
	bool IsItemExpanded(HTREEITEM hItem);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModTree)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Drag & Drop operations
public:
	DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

protected:
	static int CALLBACK ModTreeInsLibCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK ModTreeDrumCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void ModTreeBuildTVIParam(TV_INSERTSTRUCT &tvis, LPCSTR lpszName, int iImage);
	CModDoc *GetDocumentFromItem(HTREEITEM hItem);
	ModTreeDocInfo *GetDocumentInfoFromModDoc(CModDoc *pModDoc);

// Generated message map functions
protected:
	//{{AFX_MSG(CModTree)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(HTREEITEM, bool bLeft, LRESULT *pResult);
	afx_msg void OnBeginLDrag(LPNMHDR, LRESULT *pResult);
	afx_msg void OnBeginRDrag(LPNMHDR, LRESULT *pResult);
	afx_msg void OnEndDrag(DWORD dwMask);
	afx_msg void OnItemDblClk(LPNMHDR phdr, LRESULT *pResult);
	afx_msg void OnItemReturn(LPNMHDR, LRESULT *pResult);
	afx_msg void OnItemLeftClick(LPNMHDR pNMHDR, LRESULT *pResult);
	afx_msg void OnItemRightClick(LPNMHDR, LRESULT *pResult);
	afx_msg void OnItemExpanded(LPNMHDR pnmhdr, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRefreshTree();
	afx_msg void OnExecuteItem();
	afx_msg void OnPlayTreeItem();
	afx_msg void OnDeleteTreeItem();
	afx_msg void OnOpenTreeItem();
	afx_msg void OnMuteTreeItem();
	afx_msg void OnSoloTreeItem();
	afx_msg void OnUnmuteAllTreeItem();
	afx_msg void OnDuplicateTreeItem();
	afx_msg void OnInsertTreeItem();
	afx_msg void OnSwitchToTreeItem();	// hack for sequence items to avoid double-click action
	afx_msg void OnCloseItem();
	afx_msg void OnBeginLabelEdit(NMHDR *nmhdr, LRESULT *result);
	afx_msg void OnEndLabelEdit(NMHDR *nmhdr, LRESULT *result);

// -> CODE#0023
// -> DESC="IT project files (.itp)"
	afx_msg void OnSetItemPath();
	afx_msg void OnSaveItem();
// -! NEW_FEATURE#0023

	afx_msg void OnAddDlsBank();
	afx_msg void OnImportMidiLib();
	afx_msg void OnExportMidiLib();
	afx_msg void OnSoundBankProperties();
	afx_msg void OnRefreshInstrLib();
	afx_msg void OnShowAllFiles();
	afx_msg void OnShowSoundFiles();
	afx_msg LRESULT OnCustomKeyMsg(WPARAM, LPARAM);	//rewbs.customKeys
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);	//rewbs.customKeys
	afx_msg void OnSetFocus(CWnd* pOldWnd);		//rewbs.customKeys
};
