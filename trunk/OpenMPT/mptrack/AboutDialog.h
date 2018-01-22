#pragma once

OPENMPT_NAMESPACE_BEGIN

namespace PNG { struct Bitmap; }

class CRippleBitmap: public CWnd
{

public:

	static const DWORD UPDATE_INTERVAL = 15; // milliseconds

protected:

	BITMAPINFOHEADER bi;
	PNG::Bitmap *bitmapSrc, *bitmapTarget;
	std::vector<int32> offset1, offset2;
	int32 *frontBuf, *backBuf;
	DWORD lastFrame = 0;	// Time of last frame
	DWORD lastRipple = 0;	// Time of last added ripple
	bool frame = false;		// Backbuffer toggle
	bool damp = true;		// Ripple damping status
	bool activity = true;	// There are actually some ripples
	bool showMouse = true;

public:

	CRippleBitmap();
	~CRippleBitmap();
	bool Animate();

protected:
	void OnPaint();
	BOOL OnEraseBkgnd(CDC *) { return TRUE; }

	DECLARE_MESSAGE_MAP()
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseHover(UINT nFlags, CPoint point) { OnMouseMove(nFlags, point); }
	void OnMouseLeave();
};


class CAboutDlg: public CDialog
{
protected:
	CRippleBitmap m_bmp;
	CTabCtrl m_Tab;
	CEdit m_TabEdit;
	UINT_PTR m_TimerID = 0;
	static const UINT_PTR TIMERID_ABOUT_DEFAULT = 3;

public:
	static CAboutDlg *instance;

	~CAboutDlg();

	// Implementation
protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	DECLARE_MESSAGE_MAP();
	void DoDataExchange(CDataExchange* pDX) override;
	afx_msg void OnTabChange(NMHDR *pNMHDR, LRESULT *pResult);
	void OnTimer(UINT_PTR nIDEvent);
public:
	static mpt::ustring GetTabText(int tab);
};

OPENMPT_NAMESPACE_END
