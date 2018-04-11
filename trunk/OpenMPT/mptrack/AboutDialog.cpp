#include "stdafx.h"
#include "resource.h"
#include "AboutDialog.h"
#include "PNG.h"
#include "Mptrack.h"
#include "TrackerSettings.h"
#include "BuildVariants.h"
#include "../common/version.h"
#include "../common/mptWine.h"


OPENMPT_NAMESPACE_BEGIN


CAboutDlg *CAboutDlg::instance = nullptr;

BEGIN_MESSAGE_MAP(CRippleBitmap, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


CRippleBitmap::CRippleBitmap()
{
	bitmapSrc = LoadPixelImage(GetResource(MAKEINTRESOURCE(IDB_MPTRACK), _T("PNG")));
	bitmapTarget = mpt::make_unique<RawGDIDIB>(bitmapSrc->Width(), bitmapSrc->Height());
	offset1.assign(bitmapSrc->Pixels().size(), 0);
	offset2.assign(bitmapSrc->Pixels().size(), 0);
	frontBuf = offset2.data();
	backBuf = offset1.data();

	// Pre-fill first and last row of output bitmap, since those won't be touched.
	const RawGDIDIB::Pixel *in1 = bitmapSrc->Pixels().data(), *in2 = bitmapSrc->Pixels().data() + (bitmapSrc->Height() - 1) * bitmapSrc->Width();
	RawGDIDIB::Pixel *out1 = bitmapTarget->Pixels().data(), *out2 = bitmapTarget->Pixels().data() + (bitmapSrc->Height() - 1) * bitmapSrc->Width();
	for(uint32 i = 0; i < bitmapSrc->Width(); i++)
	{
		*(out1++) = *(in1++);
		*(out2++) = *(in2++);
	}

	MemsetZero(bi);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmapSrc->Width();
	bi.biHeight = -(int32)bitmapSrc->Height();
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = bitmapSrc->Width() * bitmapSrc->Height() * 4;

}


CRippleBitmap::~CRippleBitmap()
{
	if(!showMouse)
	{
		ShowCursor(TRUE);
	}
}


void CRippleBitmap::OnMouseMove(UINT nFlags, CPoint point)
{

	// Rate limit in order to avoid too may ripples.
	DWORD now = timeGetTime();
	if(now - lastRipple < UPDATE_INTERVAL)
		return;
	lastRipple = now;

	// Initiate ripples at cursor location
	Limit(point.x, 1, int(bitmapSrc->Width()) - 2);
	Limit(point.y, 2, int(bitmapSrc->Height()) - 3);
	int32 *p = backBuf + point.x + point.y * bitmapSrc->Width();
	p[0] += (nFlags & MK_LBUTTON) ? 50 : 150;
	p[0] += (nFlags & MK_MBUTTON) ? 150 : 0;

	int32 w = bitmapSrc->Width();
	// Make the initial point of this ripple a bit "fatter".
	p[-1] += p[0] / 2;     p[1] += p[0] / 2;
	p[-w] += p[0] / 2;     p[w] += p[0] / 2;
	p[-w - 1] += p[0] / 4; p[-w + 1] += p[0] / 4;
	p[w - 1] += p[0] / 4;  p[w + 1] += p[0] / 4;

	damp = !(nFlags & MK_RBUTTON);
	activity = true;

	// Wine will only ever generate MouseLeave message when the message
	// queue is completely empty and the hover timeout has expired.
	// This results in a hidden mouse cursor long after it had already left the
	// control.
	// Avoid hiding the mouse cursor on Wine. Interferring with the users input
	// methods is an absolute no-go.
	if(mpt::Windows::IsWine())
	{
		return;
	}

	TRACKMOUSEEVENT me;
	me.cbSize = sizeof(TRACKMOUSEEVENT);
	me.hwndTrack = m_hWnd;
	me.dwFlags = TME_LEAVE | TME_HOVER;
	me.dwHoverTime = 1500;

	if(TrackMouseEvent(&me) && showMouse)
	{
		ShowCursor(FALSE);
		showMouse = false;
	}
}


void CRippleBitmap::OnMouseLeave()
{
	if(!showMouse)
	{
		ShowCursor(TRUE);
		showMouse = true;
	}
}


void CRippleBitmap::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(rect);
	StretchDIBits(dc.m_hDC,
		0, 0, rect.Width(), rect.Height(),
		0, 0, bitmapTarget->Width(), bitmapTarget->Height(),
		bitmapTarget->Pixels().data(),
		reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS, SRCCOPY);
}


bool CRippleBitmap::Animate()
{
	// Were there any pixels being moved in the last frame?
	if(!activity)
		return false;

	DWORD now = timeGetTime();
	if(now - lastFrame < UPDATE_INTERVAL)
		return true;
	lastFrame = now;
	activity = false;

	frontBuf = (frame ? offset2 : offset1).data();
	backBuf = (frame ? offset1 : offset2).data();

	// Spread the ripples...
	const int32 w = bitmapSrc->Width(), h = bitmapSrc->Height();
	const int32 numPixels = w * (h - 2);
	const int32 *back = backBuf + w;
	int32 *front = frontBuf + w;
	for(int32 i = numPixels; i != 0; i--, back++, front++)
	{
		(*front) = (back[-1] + back[1] + back[w] + back[-w]) / 2 - (*front);
		if(damp) (*front) -= (*front) >> 5;
	}

	// ...and compute the final picture.
	const int32 *offset = frontBuf + w;
	const RawGDIDIB::Pixel *pixelIn = bitmapSrc->Pixels().data() + w;
	RawGDIDIB::Pixel *pixelOut = bitmapTarget->Pixels().data() + w;
	RawGDIDIB::Pixel *limitMin = bitmapSrc->Pixels().data(), *limitMax = bitmapSrc->Pixels().data() + bitmapSrc->Pixels().size() - 1;
	for(int32 i = numPixels; i != 0; i--, pixelIn++, pixelOut++, offset++)
	{
		// Compute pixel displacement
		const int32 xOff = offset[-1] - offset[1];
		const int32 yOff = offset[-w] - offset[w];

		if(xOff | yOff)
		{
			const RawGDIDIB::Pixel *p = pixelIn + xOff + yOff * w;
			Limit(p, limitMin, limitMax);
			// Add a bit of shading depending on how far we're displacing the pixel...
			pixelOut->r = mpt::saturate_cast<uint8>(p->r + (p->r * xOff) / 32);
			pixelOut->g = mpt::saturate_cast<uint8>(p->g + (p->g * xOff) / 32);
			pixelOut->b = mpt::saturate_cast<uint8>(p->b + (p->b * xOff) / 32);
			// ...and mix it with original picture
			pixelOut->r = (pixelOut->r + pixelIn->r) / 2u;
			pixelOut->g = (pixelOut->g + pixelIn->g) / 2u;
			pixelOut->b = (pixelOut->b + pixelIn->b) / 2u;
			// And now some cheap image smoothing...
			pixelOut[-1].r = (pixelOut->r + pixelOut[-1].r) / 2u;
			pixelOut[-1].g = (pixelOut->g + pixelOut[-1].g) / 2u;
			pixelOut[-1].b = (pixelOut->b + pixelOut[-1].b) / 2u;
			pixelOut[-w].r = (pixelOut->r + pixelOut[-w].r) / 2u;
			pixelOut[-w].g = (pixelOut->g + pixelOut[-w].g) / 2u;
			pixelOut[-w].b = (pixelOut->b + pixelOut[-w].b) / 2u;
			activity = true;	// Also use this to update activity status...
		} else
		{
			*pixelOut = *pixelIn;
		}
	}

	frame = !frame;

	InvalidateRect(NULL, FALSE);

	return true;
}


CAboutDlg::~CAboutDlg()
{
	instance = nullptr;
}


void CAboutDlg::OnOK()
{
	instance = nullptr;
	if(m_TimerID != 0)
	{
		KillTimer(m_TimerID);
		m_TimerID = 0;
	}
	DestroyWindow();
	delete this;
}


void CAboutDlg::OnCancel()
{
	OnOK();
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	mpt::ustring app;
	app += mpt::format(MPT_USTRING("OpenMPT %1 bit"))(sizeof(void*) * 8)
		#if defined(UNICODE)
			+ MPT_USTRING(" Unicode")
		#endif // UNICODE
		+ (!BuildVariants().CurrentBuildIsModern() ? MPT_USTRING(" for older Windows") : MPT_USTRING(""))
		+ MPT_USTRING("\n");
	app += MPT_USTRING("Version ") + Build::GetVersionStringSimple() + MPT_USTRING("\n");
	app += MPT_USTRING("\n");
	app += Build::GetURL(Build::Url::Website) + MPT_USTRING("\n");
	SetDlgItemText(IDC_EDIT3, mpt::ToCString(mpt::String::Replace(app, MPT_USTRING("\n"), MPT_USTRING("\r\n"))));

	m_bmp.SubclassDlgItem(IDC_BITMAP1, this);

	m_Tab.InsertItem(TCIF_TEXT, 0, _T("OpenMPT"), 0, 0, 0, 0);
	m_Tab.InsertItem(TCIF_TEXT, 1, _T("Components"), 0, 0, 0, 0);
	m_Tab.InsertItem(TCIF_TEXT, 2, _T("Credits"), 0, 0, 0, 0);
	m_Tab.InsertItem(TCIF_TEXT, 3, _T("License"), 0, 0, 0, 0);
	m_Tab.InsertItem(TCIF_TEXT, 4, _T("Resources"), 0, 0, 0, 0);
	if(mpt::Windows::IsWine()) m_Tab.InsertItem(TCIF_TEXT, 5, _T("Wine"), 0, 0, 0, 0);
	m_Tab.SetCurSel(0);

	OnTabChange(nullptr, nullptr);

	if(m_TimerID != 0)
	{
		KillTimer(m_TimerID);
		m_TimerID = 0;
	}
	m_TimerID = SetTimer(TIMERID_ABOUT_DEFAULT, CRippleBitmap::UPDATE_INTERVAL, nullptr);

	return TRUE;	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAboutDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_TimerID)
	{
		m_bmp.Animate();
	}
}


void CAboutDlg::OnTabChange(NMHDR * /*pNMHDR*/ , LRESULT * /*pResult*/ )
{
	m_TabEdit.SetWindowText(mpt::ToCString(mpt::String::Replace(GetTabText(m_Tab.GetCurSel()), MPT_USTRING("\n"), MPT_USTRING("\r\n"))));
}


mpt::ustring CAboutDlg::GetTabText(int tab)
{
	const mpt::ustring lf = MPT_USTRING("\n");
	const mpt::ustring yes = MPT_USTRING("yes");
	const mpt::ustring no = MPT_USTRING("no");
	mpt::ustring text;
	switch(tab)
	{
		case 0:
			text += MPT_USTRING("OpenMPT - Open ModPlug Tracker") + lf;
			text += lf;
			text += mpt::format(MPT_USTRING("Version: %1"))(Build::GetVersionStringExtended()) + lf;
			text += mpt::format(MPT_USTRING("Source Code: %1"))(SourceInfo::Current().GetUrlWithRevision() + MPT_ULITERAL(" ") + SourceInfo::Current().GetStateString()) + lf;
			text += mpt::format(MPT_USTRING("Build Date: %1"))(Build::GetBuildDateString()) + lf;
			text += mpt::format(MPT_USTRING("Compiler: %1"))(Build::GetBuildCompilerString()) + lf;
			text += mpt::format(MPT_USTRING("Required Windows Kernel Level: %1"))(mpt::Windows::Version::VersionToString(mpt::Windows::Version::GetMinimumKernelLevel())) + lf;
			text += mpt::format(MPT_USTRING("Required Windows API Level: %1"))(mpt::Windows::Version::VersionToString(mpt::Windows::Version::GetMinimumAPILevel())) + lf;
			{
				text += MPT_USTRING("Required CPU features: ");
				std::vector<mpt::ustring> features;
				#if MPT_COMPILER_MSVC
					#if defined(_M_X64)
						features.push_back(MPT_USTRING("x86-64"));
						if(GetMinimumAVXVersion() >= 1) features.push_back(MPT_USTRING("avx"));
						if(GetMinimumAVXVersion() >= 2) features.push_back(MPT_USTRING("avx2"));
					#elif defined(_M_IX86)
						if(GetMinimumSSEVersion() <= 0 && GetMinimumAVXVersion() <= 0 ) features.push_back(MPT_USTRING("fpu"));
						if(GetMinimumSSEVersion() >= 1) features.push_back(MPT_USTRING("cmov"));
						if(GetMinimumSSEVersion() >= 1) features.push_back(MPT_USTRING("sse"));
						if(GetMinimumSSEVersion() >= 2) features.push_back(MPT_USTRING("sse2"));
						if(GetMinimumAVXVersion() >= 1) features.push_back(MPT_USTRING("avx"));
						if(GetMinimumAVXVersion() >= 2) features.push_back(MPT_USTRING("avx2"));
					#else
						if(GetMinimumSSEVersion() <= 0 && GetMinimumAVXVersion() <= 0 ) features.push_back(MPT_USTRING("fpu"));
						if(GetMinimumSSEVersion() >= 1) features.push_back(MPT_USTRING("cmov"));
						if(GetMinimumSSEVersion() >= 1) features.push_back(MPT_USTRING("sse"));
						if(GetMinimumSSEVersion() >= 2) features.push_back(MPT_USTRING("sse2"));
						if(GetMinimumAVXVersion() >= 1) features.push_back(MPT_USTRING("avx"));
						if(GetMinimumAVXVersion() >= 2) features.push_back(MPT_USTRING("avx2"));
					#endif
				#endif
				text += mpt::String::Combine(features, MPT_USTRING(" "));
				text += lf;
			}
#ifdef ENABLE_ASM
			for(auto &cpuFeatures : { std::make_pair(GetProcSupport(), MPT_USTRING("Optional CPU features used: ")), std::make_pair(GetRealProcSupport(), MPT_USTRING("Available CPU features: ")) })
			{
				text += cpuFeatures.second;
				auto procSupport = cpuFeatures.first;
				std::vector<mpt::ustring> features;
#if MPT_COMPILER_MSVC && defined(ENABLE_ASM)
#if defined(ENABLE_X86)
				features.push_back(MPT_USTRING("x86"));
#endif
#if defined(ENABLE_X64)
				features.push_back(MPT_USTRING("x86-64"));
#endif
				struct ProcFlag
				{
					decltype(procSupport) flag;
					const char *name;
				};
				static constexpr ProcFlag flags[] =
				{
					{ 0, "" },
#if defined(ENABLE_X86)
					{ PROCSUPPORT_CMOV, "cmov" },
#endif
#if defined(ENABLE_MMX)
					{ PROCSUPPORT_MMX, "mmx" },
#endif
#if defined(ENABLE_SSE)
					{ PROCSUPPORT_SSE, "sse" },
#endif
#if defined(ENABLE_SSE2)
					{ PROCSUPPORT_SSE2, "sse2" },
#endif
#if defined(ENABLE_SSE3)
					{ PROCSUPPORT_SSE3, "sse3" },
					{ PROCSUPPORT_SSSE3, "ssse3" },
#endif
#if defined(ENABLE_SSE4)
					{ PROCSUPPORT_SSE4_1, "sse4.1" },
					{ PROCSUPPORT_SSE4_2, "sse4.2" },
#endif
#if defined(ENABLE_X86_AMD)
					{ PROCSUPPORT_AMD_MMXEXT, "mmxext" },
					{ PROCSUPPORT_AMD_3DNOW, "3dnow" },
					{ PROCSUPPORT_AMD_3DNOWEXT, "3dnowext" },
#endif
				};
				for(const auto &f : flags)
				{
					if(procSupport & f.flag) features.push_back(mpt::ToUnicode(mpt::CharsetASCII, f.name));
				}
#endif
				text += mpt::String::Combine(features, MPT_USTRING(" "));
				text += lf;
			}
			text += lf;
			text += mpt::format(MPT_USTRING("CPU: %1, Family %2, Model %3, Stepping %4"))
				( mpt::ToUnicode(mpt::CharsetASCII, (std::strlen(ProcVendorID) > 0) ? std::string(ProcVendorID) : std::string("Generic"))
				, ProcFamily
				, ProcModel
				, ProcStepping
				) + lf;
#endif // ENABLE_ASM
			text += mpt::format(MPT_USTRING("Operating System: %1"))(mpt::Windows::Version::Current().GetName()) + lf;
			text += lf;
			text += mpt::format(MPT_USTRING("OpenMPT Path%2: %1"))(theApp.GetAppDirPath(), theApp.IsPortableMode() ? MPT_USTRING(" (portable)") : MPT_USTRING("")) + lf;
			text += mpt::format(MPT_USTRING("Settings%2: %1"))(theApp.GetConfigFileName(), theApp.IsPortableMode() ? MPT_USTRING(" (portable)") : MPT_USTRING("")) + lf;
			break;
		case 1:
			{
			std::vector<std::string> components = ComponentManager::Instance()->GetRegisteredComponents();
			if(!TrackerSettings::Instance().ComponentsKeepLoaded)
				{
					text += MPT_USTRING("Components are loaded and unloaded as needed.") + lf;
					text += lf;
					for(const auto &component : components)
					{
						ComponentInfo info = ComponentManager::Instance()->GetComponentInfo(component);
						mpt::ustring name = mpt::ToUnicode(mpt::CharsetASCII, (info.name.substr(0, 9) == "Component") ? info.name.substr(9) : info.name);
						if(!info.settingsKey.empty())
						{
							name = mpt::ToUnicode(mpt::CharsetASCII, info.settingsKey);
						}
						text += name + lf;
					}
				} else
				{
					for(int available = 1; available >= 0; --available)
					{
						if(available)
						{
							text += MPT_USTRING("Loaded Components:") + lf;
						} else
						{
							text += lf;
							text += MPT_USTRING("Unloaded Components:") + lf;
						}
						for(const auto &component : components)
						{
							ComponentInfo info = ComponentManager::Instance()->GetComponentInfo(component);
							if(available  && info.state != ComponentStateAvailable) continue;
							if(!available && info.state == ComponentStateAvailable) continue;
							mpt::ustring name = mpt::ToUnicode(mpt::CharsetASCII, (info.name.substr(0, 9) == "Component") ? info.name.substr(9) : info.name);
							if(!info.settingsKey.empty())
							{
								name = mpt::ToUnicode(mpt::CharsetASCII, info.settingsKey);
							}
							text += mpt::format(MPT_USTRING("%1: %2"))
								( name
								, info.state == ComponentStateAvailable ? MPT_USTRING("ok") :
									info.state == ComponentStateUnavailable? MPT_USTRING("missing") :
									info.state == ComponentStateUnintialized ? MPT_USTRING("not loaded") :
									info.state == ComponentStateBlocked ? MPT_USTRING("blocked") :
									info.state == ComponentStateUnregistered ? MPT_USTRING("unregistered") :
									MPT_USTRING("unknown")
								);
							if(info.type != ComponentTypeUnknown)
							{
								text += mpt::format(MPT_USTRING(" (%1)"))
									( info.type == ComponentTypeBuiltin ? MPT_USTRING("builtin") :
										info.type == ComponentTypeSystem ? MPT_USTRING("system") :
										info.type == ComponentTypeSystemInstallable ? MPT_USTRING("system, optional") :
										info.type == ComponentTypeBundled ? MPT_USTRING("bundled") :
										info.type == ComponentTypeForeign ? MPT_USTRING("foreign") :
										MPT_USTRING("unknown")
									);
							}
							text += lf;
						}
					}
				}
			}
			break;
		case 2:
			text += Build::GetFullCreditsString();
			break;
		case 3:
			text += Build::GetLicenseString();
			break;
		case 4:
			text += lf;
			text += MPT_USTRING("Website: ") + lf + Build::GetURL(Build::Url::Website) + lf;
			text += lf;
			text += MPT_USTRING("Forum: ") + lf + Build::GetURL(Build::Url::Forum) + lf;
			text += lf;
			text += MPT_USTRING("Bug Tracker: ") + lf + Build::GetURL(Build::Url::Bugtracker) + lf;
			text += lf;
			text += MPT_USTRING("Updates: ") + lf + Build::GetURL(Build::Url::Updates) + lf;
			text += lf;
			break;
		case 5:
			try
			{
				if(!theApp.GetWine())
				{

					text += MPT_USTRING("Wine integration not available.\n");

				} else
				{

					mpt::Wine::Context & wine = *theApp.GetWine();
					
					text += mpt::format(MPT_USTRING("Windows: %1\n"))
						( mpt::Windows::Version::Current().IsWindows() ? yes : no
						);
					text += mpt::format(MPT_USTRING("Windows version: %1\n"))
						( 
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::Win81) ? MPT_USTRING("Windows 8.1") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::Win8) ? MPT_USTRING("Windows 8") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::Win7) ? MPT_USTRING("Windows 7") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::WinVista) ? MPT_USTRING("Windows Vista") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::WinXP) ? MPT_USTRING("Windows XP") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::Win2000) ? MPT_USTRING("Windows 2000") :
						mpt::Windows::Version::Current().IsAtLeast(mpt::Windows::Version::WinNT4) ? MPT_USTRING("Windows NT4") :
						MPT_USTRING("unknown")
						);
					text += mpt::format(MPT_USTRING("Windows original: %1\n"))
						( mpt::Windows::IsOriginal() ? yes : no
						);

					text += MPT_USTRING("\n");

					text += mpt::format(MPT_USTRING("Wine: %1\n"))
						( mpt::Windows::IsWine() ? yes : no
						);
					text += mpt::format(MPT_USTRING("Wine Version: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.VersionContext().RawVersion())
						);
					text += mpt::format(MPT_USTRING("Wine Build ID: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.VersionContext().RawBuildID())
						);
					text += mpt::format(MPT_USTRING("Wine Host Sys Name: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.VersionContext().RawHostSysName())
						);
					text += mpt::format(MPT_USTRING("Wine Host Release: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.VersionContext().RawHostRelease())
						);

					text += MPT_USTRING("\n");

					text += mpt::format(MPT_USTRING("uname -m: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.Uname_m())
						);
					text += mpt::format(MPT_USTRING("HOME: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.HOME())
						);
					text += mpt::format(MPT_USTRING("XDG_DATA_HOME: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.XDG_DATA_HOME())
						);
					text += mpt::format(MPT_USTRING("XDG_CACHE_HOME: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.XDG_CACHE_HOME())
						);
					text += mpt::format(MPT_USTRING("XDG_CONFIG_HOME: %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.XDG_CONFIG_HOME())
						);

					text += MPT_USTRING("\n");

					text += mpt::format(MPT_USTRING("OpenMPT folder: %1\n"))
						( theApp.GetAppDirPath().ToUnicode()
						);
					text += mpt::format(MPT_USTRING("OpenMPT folder (host): %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.PathToPosix(theApp.GetAppDirPath()))
						);
					text += mpt::format(MPT_USTRING("OpenMPT config folder: %1\n"))
						( theApp.GetConfigPath().ToUnicode()
						);
					text += mpt::format(MPT_USTRING("OpenMPT config folder (host): %1\n"))
						( mpt::ToUnicode(mpt::CharsetUTF8, wine.PathToPosix(theApp.GetConfigPath()))
						);
					text += mpt::format(MPT_USTRING("Host root: %1\n"))
						( wine.PathToWindows("/").ToUnicode()
						);

				}
			} catch(const mpt::Wine::Exception & e)
			{
				text += MPT_USTRING("Exception: ") + mpt::get_exception_text<mpt::ustring>(e) + MPT_USTRING("\n");
			}
			break;
	}
	return text;
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABABOUT, m_Tab);
	DDX_Control(pDX, IDC_EDITABOUT, m_TabEdit);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABABOUT, &CAboutDlg::OnTabChange)
END_MESSAGE_MAP()



OPENMPT_NAMESPACE_END
