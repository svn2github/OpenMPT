/*
 * Miscellaneous (unit) tests.
 * 
 */

#include "stdafx.h"
#include "test.h"
#include "../mptrack.h"
#include "../moddoc.h"
#include "../MainFrm.h"
#include "../version.h"
#include "../misc_util.h"
#include "../serialization_utils.h"
#include <limits>
#include <fstream>
#include <strstream>


#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

#ifdef ENABLE_TESTS

namespace MptTest
{

//Verify that given parameters are 'equal'(show error message if not).
//The exact meaning of equality is not specified; for now using operator!=.
//The macro is active in both 'debug' and 'release' build.
#define VERIFY_EQUAL(x,y)	\
	if((x) != (y))		\
	{					\
		CString str;	\
		str.Format("File: " STRINGIZE(__FILE__) "\nLine: " STRINGIZE(__LINE__) "\n\nVERIFY_EQUAL failed when comparing\n" #x "\nand\n" #y); \
		MessageBox(0, str, "VERIFY_EQUAL failed", MB_ICONERROR);	\
	}

// Like VERIFY_EQUAL, but throws exception if comparison fails.
#define VERIFY_EQUAL_NONCONT(x,y) \
	if((x) != (y))		\
	{					\
		CString str;	\
		str.Format("File: " STRINGIZE(__FILE__) "\nLine: " STRINGIZE(__LINE__) "\n\nVERIFY_EQUAL failed when comparing\n" #x "\nand\n" #y); \
		std::string stdstr = str; \
		throw std::runtime_error(stdstr); \
	}


#define DO_TEST(func)			\
try								\
{								\
	func();						\
}								\
catch(const std::exception& e)	\
{								\
	MessageBox(0, CString("Test \"" STRINGIZE(func) "\" threw an exception, message: ") + e.what(), "Test \"" STRINGIZE(func) "\": Exception was thrown", MB_ICONERROR); \
}	\
catch(...)  \
{			\
	MessageBox(0, CString("Test \"" STRINGIZE(func) "\" threw an unknown exception."), "Test \"" STRINGIZE(func) "\": Exception was thrown", MB_ICONERROR); \
}
	

void TestVersion();
void TestTypes();
void TestLoadSaveFile();
void TestPCnoteSerialization();
void TestMisc();




void DoTests()
//------------
{
	DO_TEST(TestVersion);
	DO_TEST(TestTypes);
	//DO_TEST(TestPCnoteSerialization);
	DO_TEST(TestMisc);
	DO_TEST(TestLoadSaveFile)

	Log(TEXT("Tests were run\n"));
}


// Test if functions related to program version data work
void TestVersion()
//----------------
{
	//Verify that macros and functions work.
	{
		VERIFY_EQUAL( MPT_VERSION_NUMERIC, MptVersion::num );
		VERIFY_EQUAL( CString(MPT_VERSION_STR), CString(MptVersion::str) );
		VERIFY_EQUAL( MptVersion::ToNum(MptVersion::ToStr(MptVersion::num)), MptVersion::num );
		VERIFY_EQUAL( MptVersion::ToStr(MptVersion::ToNum(MptVersion::str)), MptVersion::str );
		VERIFY_EQUAL( MptVersion::ToStr(18285096), "1.17.02.28" );
		VERIFY_EQUAL( MptVersion::ToNum("1.17.02.28"), 18285096 );
		VERIFY_EQUAL( MptVersion::ToNum(MptVersion::str), MptVersion::num );
		VERIFY_EQUAL( MptVersion::ToStr(MptVersion::num), MptVersion::str );
		VERIFY_EQUAL( MptVersion::RemoveBuildNumber(MAKE_VERSION_NUMERIC(1,19,02,00)), MAKE_VERSION_NUMERIC(1,19,02,00));
		VERIFY_EQUAL( MptVersion::RemoveBuildNumber(MAKE_VERSION_NUMERIC(1,18,03,20)), MAKE_VERSION_NUMERIC(1,18,03,00));
		VERIFY_EQUAL( MptVersion::IsTestBuild(MAKE_VERSION_NUMERIC(1,18,01,13)), true);
		VERIFY_EQUAL( MptVersion::IsTestBuild(MAKE_VERSION_NUMERIC(1,19,01,00)), false);
		VERIFY_EQUAL( MptVersion::IsTestBuild(MAKE_VERSION_NUMERIC(1,17,02,54)), false);
		STATIC_ASSERT( MAKE_VERSION_NUMERIC(1,17,2,28) == 18285096 );
		STATIC_ASSERT( MAKE_VERSION_NUMERIC(1,17,02,48) == 18285128 );
		STATIC_ASSERT( MAKE_VERSION_NUMERIC(01,17,02,52) == 18285138 );
		// Ensure that bit-shifting works (used in some mod loaders for example)
		STATIC_ASSERT( MAKE_VERSION_NUMERIC(01,17,00,00) == 0x0117 << 16 );
		STATIC_ASSERT( MAKE_VERSION_NUMERIC(01,17,03,00) >> 8 == 0x011703 );
	}

	//Verify that the version obtained from the executable file is the same as 
	//defined in MptVersion.
	{
		char  szFullPath[MAX_PATH];
		DWORD dwVerHnd;
		DWORD dwVerInfoSize;

		// Get version information from the application
		::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
		dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
		if (!dwVerInfoSize)
			throw std::runtime_error("!dwVerInfoSize is true");

		char* pVersionInfo = new char[dwVerInfoSize];
		if (!pVersionInfo)
			throw std::runtime_error("!pVersionInfo is true");

		char* szVer = NULL;
		UINT uVerLength;
		if (!(::GetFileVersionInfo((LPTSTR)szFullPath, (DWORD)dwVerHnd, 
								   (DWORD)dwVerInfoSize, (LPVOID)pVersionInfo))) {
			delete[] pVersionInfo;
			throw std::runtime_error("GetFileVersionInfo() returned false");
		}   
		if (!(::VerQueryValue(pVersionInfo, TEXT("\\StringFileInfo\\040904b0\\FileVersion"), 
							  (LPVOID*)&szVer, &uVerLength))) {
			delete[] pVersionInfo;
			throw std::runtime_error("VerQueryValue() returned false");
		}

		CString version = szVer;	
		delete[] pVersionInfo;

		//version string should be like: 1,17,2,38  Change ',' to '.' to get format 1.17.2.38
		version.Replace(',', '.');

		VERIFY_EQUAL( version, MptVersion::str );
		VERIFY_EQUAL( MptVersion::ToNum(version), MptVersion::num );
	}
}


// Test if data types are interpreted correctly
void TestTypes()
//--------------
{
	VERIFY_EQUAL(int8_min, (std::numeric_limits<int8>::min)());
	VERIFY_EQUAL(int8_max, (std::numeric_limits<int8>::max)());
	VERIFY_EQUAL(uint8_max, (std::numeric_limits<uint8>::max)());

	VERIFY_EQUAL(int16_min, (std::numeric_limits<int16>::min)());
	VERIFY_EQUAL(int16_max, (std::numeric_limits<int16>::max)());
	VERIFY_EQUAL(uint16_max, (std::numeric_limits<uint16>::max)());

	VERIFY_EQUAL(int32_min, (std::numeric_limits<int32>::min)());
	VERIFY_EQUAL(int32_max, (std::numeric_limits<int32>::max)());
	VERIFY_EQUAL(uint32_max, (std::numeric_limits<uint32>::max)());

	VERIFY_EQUAL(int64_min, (std::numeric_limits<int64>::min)());
	VERIFY_EQUAL(int64_max, (std::numeric_limits<int64>::max)());
	VERIFY_EQUAL(uint64_max, (std::numeric_limits<uint64>::max)());

	VERIFY_EQUAL(ROWINDEX_MAX, (std::numeric_limits<ROWINDEX>::max)());
	VERIFY_EQUAL(ORDERINDEX_MAX, (std::numeric_limits<ORDERINDEX>::max)());
	VERIFY_EQUAL(PATTERNINDEX_MAX, (std::numeric_limits<PATTERNINDEX>::max)());
	VERIFY_EQUAL(SAMPLEINDEX_MAX, (std::numeric_limits<SAMPLEINDEX>::max)());
	VERIFY_EQUAL(INSTRUMENTINDEX_MAX, (std::numeric_limits<INSTRUMENTINDEX>::max)());
}


void TestMisc()
//-------------
{
	VERIFY_EQUAL(ConvertStrTo<uint32>("586"), 586);
	VERIFY_EQUAL(ConvertStrTo<uint32>("2147483647"), int32_max);
	VERIFY_EQUAL(ConvertStrTo<uint32>("4294967295"), uint32_max);

	VERIFY_EQUAL(ConvertStrTo<int64>("-9223372036854775808"), int64_min);
	VERIFY_EQUAL(ConvertStrTo<int64>("-159"), -159);
	VERIFY_EQUAL(ConvertStrTo<int64>("9223372036854775807"), int64_max);

	VERIFY_EQUAL(ConvertStrTo<uint64>("85059"), 85059);
	VERIFY_EQUAL(ConvertStrTo<uint64>("9223372036854775807"), int64_max);
	VERIFY_EQUAL(ConvertStrTo<uint64>("18446744073709551615"), uint64_max);

	VERIFY_EQUAL(ConvertStrTo<float>("-87.0"), -87.0);
	VERIFY_EQUAL(ConvertStrTo<double>("-0.5e-6"), -0.5e-6);
	VERIFY_EQUAL(ConvertStrTo<double>("58.65403492763"), 58.65403492763);

	VERIFY_EQUAL(MODCOMMAND::IsPcNote(NOTE_MAX), false);
	VERIFY_EQUAL(MODCOMMAND::IsPcNote(NOTE_PC), true);
	VERIFY_EQUAL(MODCOMMAND::IsPcNote(NOTE_PCS), true);
}


// Check if our test file was loaded correctly.
void TestLoadFile(const CModDoc *pModDoc)
//---------------------------------------
{
	const CSoundFile *pSndFile = pModDoc->GetSoundFile();

	// Global Variables
	VERIFY_EQUAL_NONCONT(strcmp(pSndFile->m_szNames[0], "Test Module"), 0);
	VERIFY_EQUAL_NONCONT(pSndFile->m_lpszSongComments[0], 'O');
	VERIFY_EQUAL_NONCONT(pSndFile->m_nDefaultTempo, 139);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nDefaultSpeed, 5);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nGlobalVolume, 128);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nVSTiVolume, 42);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nSamplePreAmp, 23);
	VERIFY_EQUAL_NONCONT((pSndFile->m_dwSongFlags & SONG_FILE_FLAGS), SONG_EMBEDMIDICFG | SONG_LINEARSLIDES | SONG_EXFILTERRANGE | SONG_ITCOMPATGXX | SONG_ITOLDEFFECTS);
	VERIFY_EQUAL_NONCONT(pSndFile->GetModFlag(MSF_COMPATIBLE_PLAY), true);
	VERIFY_EQUAL_NONCONT(pSndFile->GetModFlag(MSF_MIDICC_BUGEMULATION), false);
	VERIFY_EQUAL_NONCONT(pSndFile->GetModFlag(MSF_OLDVOLSWING), false);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nMixLevels, mixLevels_compatible);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nTempoMode, tempo_mode_modern);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nDefaultRowsPerBeat, 6);
	VERIFY_EQUAL_NONCONT(pSndFile->m_nDefaultRowsPerMeasure, 12);
	VERIFY_EQUAL_NONCONT(pSndFile->m_dwCreatedWithVersion, MAKE_VERSION_NUMERIC(1, 19, 02, 05));
	VERIFY_EQUAL_NONCONT(pSndFile->m_nRestartPos, 1);
	
	// Edit history
	VERIFY_EQUAL_NONCONT(pModDoc->GetFileHistory()->size() > 0, true);
	const FileHistory &fh = pModDoc->GetFileHistory()->at(0);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_year, 111);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_mon, 5);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_mday, 14);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_hour, 21);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_min, 8);
	VERIFY_EQUAL_NONCONT(fh.loadDate.tm_sec, 32);
	VERIFY_EQUAL_NONCONT((uint32)((double)fh.openTime / HISTORY_TIMER_PRECISION), 31);

	// Macros
	VERIFY_EQUAL_NONCONT(pModDoc->GetMacroType(pSndFile->m_MidiCfg.szMidiSFXExt[0]), sfx_reso);
	VERIFY_EQUAL_NONCONT(pModDoc->GetMacroType(pSndFile->m_MidiCfg.szMidiSFXExt[1]), sfx_drywet);
	VERIFY_EQUAL_NONCONT(pModDoc->GetZxxType(pSndFile->m_MidiCfg.szMidiZXXExt), zxx_resomode);

	// Channels
	VERIFY_EQUAL_NONCONT(pSndFile->GetNumChannels(), 2);
	VERIFY_EQUAL_NONCONT(strcmp(pSndFile->ChnSettings[0].szName, "First Channel"), 0);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[0].nPan, 32);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[0].nVolume, 32);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[0].dwFlags, CHN_MUTE);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[0].nMixPlugin, 0);

	VERIFY_EQUAL_NONCONT(strcmp(pSndFile->ChnSettings[1].szName, "Second Channel"), 0);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[1].nPan, 128);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[1].nVolume, 16);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[1].dwFlags, CHN_SURROUND);
	VERIFY_EQUAL_NONCONT(pSndFile->ChnSettings[1].nMixPlugin, 1);

	// Samples
	VERIFY_EQUAL_NONCONT(pSndFile->GetNumSamples(), 1);
	const MODSAMPLE *pSmp = &pSndFile->Samples[1];
	VERIFY_EQUAL_NONCONT(pSmp->GetBytesPerSample(), 1);
	VERIFY_EQUAL_NONCONT(pSmp->GetNumChannels(), 1);
	VERIFY_EQUAL_NONCONT(pSmp->GetElementarySampleSize(), 1);
	VERIFY_EQUAL_NONCONT(pSmp->GetSampleSizeInBytes(), 16);
	VERIFY_EQUAL_NONCONT(pSmp->GetSampleRate(MOD_TYPE_MPT), 9001);
	VERIFY_EQUAL_NONCONT(pSmp->uFlags, CHN_PANNING | CHN_LOOP | CHN_SUSTAINLOOP | CHN_PINGPONGSUSTAIN);

	VERIFY_EQUAL_NONCONT(pSmp->nLoopStart, 1);
	VERIFY_EQUAL_NONCONT(pSmp->nLoopEnd, 8);
	VERIFY_EQUAL_NONCONT(pSmp->nSustainStart, 1);
	VERIFY_EQUAL_NONCONT(pSmp->nSustainEnd, 7);

	VERIFY_EQUAL_NONCONT(pSmp->nVibType, VIB_SQUARE);
	VERIFY_EQUAL_NONCONT(pSmp->nVibSweep, 3);
	VERIFY_EQUAL_NONCONT(pSmp->nVibRate, 4);
	VERIFY_EQUAL_NONCONT(pSmp->nVibDepth, 5);

	// Instruments
	VERIFY_EQUAL_NONCONT(pSndFile->GetNumInstruments(), 1);
	const MODINSTRUMENT *pIns = pSndFile->Instruments[1];
	VERIFY_EQUAL_NONCONT(pIns->nGlobalVol, 32);
	VERIFY_EQUAL_NONCONT(pIns->nFadeOut, 1024);
	VERIFY_EQUAL_NONCONT(pIns->nPan, 64);
	VERIFY_EQUAL_NONCONT(pIns->dwFlags, INS_SETPANNING);

	VERIFY_EQUAL_NONCONT(pIns->nPPS, 16);
	VERIFY_EQUAL_NONCONT(pIns->nPPC, (NOTE_MIDDLEC - 1) + 6);	// F#5

	VERIFY_EQUAL_NONCONT(pIns->nVolRamp, 1200);
	VERIFY_EQUAL_NONCONT(pIns->nResampling, SRCMODE_POLYPHASE);

	VERIFY_EQUAL_NONCONT(pIns->nIFC, 0x80 | 0x32);
	VERIFY_EQUAL_NONCONT(pIns->nIFR, 0x80 | 0x64);
	VERIFY_EQUAL_NONCONT(pIns->nFilterMode, FLTMODE_HIGHPASS);

	VERIFY_EQUAL_NONCONT(pIns->nVolSwing, 0x30);
	VERIFY_EQUAL_NONCONT(pIns->nPanSwing, 0x18);
	VERIFY_EQUAL_NONCONT(pIns->nCutSwing, 0x0C);
	VERIFY_EQUAL_NONCONT(pIns->nResSwing, 0x3C);

	VERIFY_EQUAL_NONCONT(pIns->nNNA, NNA_CONTINUE);
	VERIFY_EQUAL_NONCONT(pIns->nDCT, DCT_NOTE);
	VERIFY_EQUAL_NONCONT(pIns->nDNA, DNA_NOTEFADE);

	VERIFY_EQUAL_NONCONT(pIns->nMixPlug, 1);
	VERIFY_EQUAL_NONCONT(pIns->nMidiChannel, 16);
	VERIFY_EQUAL_NONCONT(pIns->nMidiProgram, 64);
	VERIFY_EQUAL_NONCONT(pIns->wMidiBank, 2);

	VERIFY_EQUAL_NONCONT(pIns->pTuning, pIns->s_DefaultTuning)

	VERIFY_EQUAL_NONCONT(pIns->wPitchToTempoLock, 130);

	VERIFY_EQUAL_NONCONT(pIns->nPluginVelocityHandling, PLUGIN_VELOCITYHANDLING_VOLUME);
	VERIFY_EQUAL_NONCONT(pIns->nPluginVolumeHandling, PLUGIN_VOLUMEHANDLING_MIDI);

	for(size_t i = 0; i < NOTE_MAX; i++)
	{
		if(i == NOTE_MIDDLEC - 1)
		{
			VERIFY_EQUAL_NONCONT(pIns->Keyboard[i], 99);
			VERIFY_EQUAL_NONCONT(pIns->NoteMap[i], i + 13);
		}
		else
		{
			VERIFY_EQUAL_NONCONT(pIns->Keyboard[i], 1);
			VERIFY_EQUAL_NONCONT(pIns->NoteMap[i], i + 1);
		}
	}

	VERIFY_EQUAL_NONCONT(pIns->VolEnv.dwFlags, ENV_ENABLED | ENV_CARRY);
	VERIFY_EQUAL_NONCONT(pIns->VolEnv.nNodes, 3);
	VERIFY_EQUAL_NONCONT(pIns->VolEnv.nReleaseNode, 1);
	VERIFY_EQUAL_NONCONT(pIns->VolEnv.Ticks[2], 96);
	VERIFY_EQUAL_NONCONT(pIns->VolEnv.Values[2], 0);

	VERIFY_EQUAL_NONCONT(pIns->PanEnv.dwFlags, ENV_LOOP);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.nNodes, 76);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.nLoopStart, 22);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.nLoopEnd, 29);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.nReleaseNode, ENV_RELEASE_NODE_UNSET);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.Ticks[75], 427);
	VERIFY_EQUAL_NONCONT(pIns->PanEnv.Values[75], 27);

	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.dwFlags, ENV_ENABLED | ENV_CARRY | ENV_SUSTAIN | ENV_FILTER);
	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.nNodes, 3);
	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.nSustainStart, 1);
	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.nSustainEnd, 2);
	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.Ticks[1], 96);
	VERIFY_EQUAL_NONCONT(pIns->PitchEnv.Values[1], 64);

	// Sequences
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetNumSequences(), 2);

	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(0).GetLengthTailTrimmed(), 3);
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(0).m_sName, "First Sequence");
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(0)[0], pSndFile->Order.GetIgnoreIndex());
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(0)[1], 0);
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(0)[2], pSndFile->Order.GetIgnoreIndex());

	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(1).GetLengthTailTrimmed(), 2);
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(1).m_sName, "Second Sequence");
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(1)[0], 1);
	VERIFY_EQUAL_NONCONT(pSndFile->Order.GetSequence(1)[1], 2);

	// Patterns
	VERIFY_EQUAL_NONCONT(pSndFile->GetNumPatterns(), 2);

	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetName(), "First Pattern");
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetNumRows(), 70);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetNumChannels(), 2);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetOverrideSignature(), true);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetRowsPerBeat(), 5);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[0].GetRowsPerMeasure(), 10);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns.IsPatternEmpty(0), true);

	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetName(), "Second Pattern");
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetNumRows(), 32);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetNumChannels(), 2);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetOverrideSignature(), false);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetRowsPerBeat(), 0);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetRowsPerMeasure(), 0);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns.IsPatternEmpty(1), false);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetpModCommand(0, 0)->IsPcNote(), true);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetpModCommand(0, 0)->GetValueVolCol(), 1);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetpModCommand(0, 0)->GetValueEffectCol(), 200);
	VERIFY_EQUAL_NONCONT(pSndFile->Patterns[1].GetpModCommand(31, 0)->IsEmpty(), true);

	// Plugins
	const SNDMIXPLUGIN &plug = pSndFile->m_MixPlugins[0];
	VERIFY_EQUAL_NONCONT(strcmp(plug.GetName(), "First Plugin"), 0);
	VERIFY_EQUAL_NONCONT(plug.fDryRatio, 0.26f);
	VERIFY_EQUAL_NONCONT((plug.Info.dwInputRouting & MIXPLUG_INPUTF_MASTEREFFECT), MIXPLUG_INPUTF_MASTEREFFECT);
	VERIFY_EQUAL_NONCONT((plug.Info.dwInputRouting >> 16), 11);

	// MIDI Mapping
	VERIFY_EQUAL_NONCONT(pSndFile->GetMIDIMapper().GetCount(), 1);
	const CMIDIMappingDirective &mapping = pSndFile->GetMIDIMapper().GetDirective(0);
	VERIFY_EQUAL_NONCONT(mapping.GetAllowPatternEdit(), true);
	VERIFY_EQUAL_NONCONT(mapping.GetCaptureMIDI(), false);
	VERIFY_EQUAL_NONCONT(mapping.IsActive(), true);
	VERIFY_EQUAL_NONCONT(mapping.GetAnyChannel(), false);
	VERIFY_EQUAL_NONCONT(mapping.GetChannel(), 5);
	VERIFY_EQUAL_NONCONT(mapping.GetPlugIndex(), 1);
	VERIFY_EQUAL_NONCONT(mapping.GetParamIndex(), 0);
	VERIFY_EQUAL_NONCONT(mapping.GetEvent(), MIDIEVENT_CONTROLLERCHANGE);
	VERIFY_EQUAL_NONCONT(mapping.GetController(), MIDICC_ModulationWheel_Coarse);

}


// Test file loading and saving
void TestLoadSaveFile()
//---------------------
{
	CString theFile = theApp.GetAppDirPath();
	// Only run the tests when we're in the project directory structure.
	if(theFile.Mid(theFile.GetLength() - 6, 5) != "Debug")
		return;
	theFile.Delete(theFile.GetLength() - 6, 6);
	theFile.Append("test/test.");

	// Test file loading
	CModDoc *pModDoc = (CModDoc *)theApp.OpenDocumentFile(theFile + "mptm");
	TestLoadFile(pModDoc);

	// Test file saving
 	pModDoc->DoSave(theFile + "saved.mptm");
	pModDoc->OnCloseDocument();
	
	// Reload the saved file and test if everything is still working correctly.
	pModDoc = (CModDoc *)theApp.OpenDocumentFile(theFile + "saved.mptm");
	TestLoadFile(pModDoc);
	pModDoc->OnCloseDocument();
}


template<class T>
T Round(double a) {return static_cast<T>(floor(a + 0.5));}

double Rand01() {return rand() / double(RAND_MAX);}

template <class T>
T Rand(const T& min, const T& max) {return Round<T>(min + Rand01() * (max - min));}



void GenerateCommands(CPattern& pat, const double dProbPcs, const double dProbPc)
//-------------------------------------------------------------------------------
{
	const double dPcxProb = dProbPcs + dProbPc;
	const CPattern::const_iterator end = pat.End();
	for(CPattern::iterator i = pat.Begin(); i != end; i++)
	{
		const double rand = Rand01();
		if(rand < dPcxProb)
		{
			if(rand < dProbPcs)
				i->note = NOTE_PCS;
			else
				i->note = NOTE_PC;

			i->instr = Rand<BYTE>(0, MAX_MIXPLUGINS);
			i->SetValueVolCol(Rand<uint16>(0, MODCOMMAND::maxColumnValue));
			i->SetValueEffectCol(Rand<uint16>(0, MODCOMMAND::maxColumnValue));
		}
		else
			i->Clear();
	}
}


// Test PC note serialization
void TestPCnoteSerialization()
//----------------------------
{
	theApp.OnFileNewMPT();
	CMainFrame* pMainFrm = CMainFrame::GetMainFrame();
	if(pMainFrm == nullptr)
		throw(std::runtime_error("pMainFrm is nullptr"));
	CModDoc* pModDoc = pMainFrm->GetActiveDoc();
	if(pModDoc == nullptr)
		throw(std::runtime_error("pModdoc is nullptr"));

	CSoundFile* pSndFile = pModDoc->GetSoundFile();
	if(pSndFile == nullptr)
		throw(std::runtime_error("pSndFile is nullptr"));

	// Set maximum number of channels.
	pModDoc->ReArrangeChannels(std::vector<CHANNELINDEX>(ModSpecs::mptm.channelsMax , 0));

	pSndFile->Patterns.Remove(0);
	pSndFile->Patterns.Insert(0, ModSpecs::mptm.patternRowsMin);
	pSndFile->Patterns.Insert(1, 64);
	GenerateCommands(pSndFile->Patterns[1], 0.3, 0.3);
	pSndFile->Patterns.Insert(2, ModSpecs::mptm.patternRowsMax);
	GenerateCommands(pSndFile->Patterns[2], 0.5, 0.5);

	//
	vector<MODCOMMAND> pat[3];
	const size_t numCommands[] = {	pSndFile->GetNumChannels() * pSndFile->Patterns[0].GetNumRows(),
									pSndFile->GetNumChannels() * pSndFile->Patterns[1].GetNumRows(),
									pSndFile->GetNumChannels() * pSndFile->Patterns[2].GetNumRows()
								 };
	pat[0].resize(numCommands[0]);
	pat[1].resize(numCommands[1]);
	pat[2].resize(numCommands[2]);

	for(size_t i = 0; i<3; i++) // Copy pattern data for comparison.
	{
		CPattern::const_iterator iter = pSndFile->Patterns[i].Begin();
		for(size_t j = 0; j < numCommands[i]; j++, iter++) pat[i][j] = *iter;
	}

	std::strstream mem;
	WriteModPatterns(mem, pSndFile->Patterns);

	VERIFY_EQUAL_NONCONT( mem.good(), true );

	// Clear patterns.
	pSndFile->Patterns[0].ClearCommands();
	pSndFile->Patterns[1].ClearCommands();
	pSndFile->Patterns[2].ClearCommands();

	// Read data back.
	ReadModPatterns(mem, pSndFile->Patterns);

	// Compare.
	VERIFY_EQUAL_NONCONT( pSndFile->Patterns[0].GetNumRows(), ModSpecs::mptm.patternRowsMin);
	VERIFY_EQUAL_NONCONT( pSndFile->Patterns[1].GetNumRows(), 64);
	VERIFY_EQUAL_NONCONT( pSndFile->Patterns[2].GetNumRows(), ModSpecs::mptm.patternRowsMax);
	for(size_t i = 0; i < 3; i++)
	{
		bool bPatternDataMatch = true;
		CPattern::const_iterator iter = pSndFile->Patterns[i].Begin();
		for(size_t j = 0; j < numCommands[i]; j++, iter++)
		{
			if(pat[i][j] != *iter)
			{
				bPatternDataMatch = false;
				break;
			}
		}
		VERIFY_EQUAL( bPatternDataMatch, true);
	}
		
	pModDoc->OnCloseDocument();
}


}; //Namespace MptTest

#else //Case: ENABLE_TESTS is not defined.

namespace MptTest
{
	void DoTests() {}
};

#endif

