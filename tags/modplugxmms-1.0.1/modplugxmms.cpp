/* Modplug XMMS Plugin
 * Copyright (C) 1999 Kenton Varda and Olivier Lapicque
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include<pthread.h>
//open()
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
//mmap()
#include<unistd.h>
#include<sys/mman.h>
#include<fstream>

#include"modplugxmms.h"
#include"soundfile/stdafx.h"
#include"soundfile/sndfile.h"
#include"stddefs.h"
#include"modprops.h"

// CModplugXMMS::State definition ==============================

struct CModplugXMMS::State
{
	static InputPlugin*  mInPlug;
	static OutputPlugin* mOutPlug;

	static uchar*  mBuffer;
	static uint32  mBufSize;

	static bool    mPaused;
	static bool    mStopped;

	static ModProperties mModProps;

	static AFormat mFormat;

	static uint32  mBufTime;		//milliseconds

	static CSoundFile  mSoundFile;
	static int         mFileDesc;
	static uchar*      mFileMap;
	static uint32      mFileSize;

	static uint32      mPlayed;

	static pthread_t   mDecodeThread;

	static char        mModName[100];

	static bool        mEqualizer;
};

// Static initializers -------------------------------
InputPlugin*  CModplugXMMS::State::mInPlug         = NULL;
OutputPlugin* CModplugXMMS::State::mOutPlug        = NULL;
uchar*        CModplugXMMS::State::mBuffer          = NULL;
uint32        CModplugXMMS::State::mBufSize        = 0;

bool          CModplugXMMS::State::mPaused         = false;
bool          CModplugXMMS::State::mStopped        = true;

ModProperties CModplugXMMS::State::mModProps;
AFormat       CModplugXMMS::State::mFormat         = FMT_S16_LE;

uint32        CModplugXMMS::State::mBufTime        = 20;

CSoundFile    CModplugXMMS::State::mSoundFile;
int           CModplugXMMS::State::mFileDesc       = 0;
uchar*        CModplugXMMS::State::mFileMap        = NULL;
uint32        CModplugXMMS::State::mFileSize       = 0;

uint32        CModplugXMMS::State::mPlayed         = 0;

pthread_t     CModplugXMMS::State::mDecodeThread;

char          CModplugXMMS::State::mModName[100];

bool          CModplugXMMS::State::mEqualizer      = false;

// CModplugXMMS member functions ===============================

// operations ----------------------------------------
void CModplugXMMS::Init(void)
{
	fstream lConfigFile;
	string lField, lValue;
	string lConfigFilename;
	bool lValueB;
	char junk;

	State::mModProps.mSurround       = true;
	State::mModProps.mOversamp       = true;
	State::mModProps.mReverb         = true;
	State::mModProps.mMegabass       = true;
	State::mModProps.mNoiseReduction = true;
	State::mModProps.mVolumeRamp     = true;
	State::mModProps.mFadeout        = false;    //not stable
	State::mModProps.mFastinfo       = true;

	State::mModProps.mChannels       = 2;
	State::mModProps.mFrequency      = 44100;
	State::mModProps.mBits           = 16;

	State::mModProps.mReverbDepth    = 1;
	State::mModProps.mReverbDelay    = 100;
	State::mModProps.mBassAmount     = 6;
	State::mModProps.mBassRange      = 14;
	State::mModProps.mSurroundDepth  = 12;
	State::mModProps.mSurroundDelay  = 20;
	State::mModProps.mFadeTime       = 500;

	//I chose to use a separate config file to avoid conflicts
	lConfigFilename = g_get_home_dir();
	lConfigFilename += "/.xmms/modplug-xmms.conf";
	lConfigFile.open(lConfigFilename.c_str(), ios::in);

	if(!lConfigFile.is_open())
		return;

	while(!lConfigFile.eof())
	{
		lConfigFile >> lField;
		if(lField[0] == '#')      //allow comments
		{
			do
			{
				lConfigFile.read(&junk, 1);
			}
			while(junk != '\n');
		}
		else
		{
			if(lField == "reverb_depth")
				lConfigFile >> State::mModProps.mReverbDepth;
			else if(lField == "reverb_delay")
				lConfigFile >> State::mModProps.mReverbDelay;
			else if(lField == "megabass_amount")
				lConfigFile >> State::mModProps.mBassAmount;
			else if(lField == "megabass_range")
				lConfigFile >> State::mModProps.mBassRange;
			else if(lField == "surround_depth")
				lConfigFile >> State::mModProps.mSurroundDepth;
			else if(lField == "surround_delay")
				lConfigFile >> State::mModProps.mSurroundDelay;
			else if(lField == "fadeout_time")
				lConfigFile >> State::mModProps.mFadeTime;

      else
			{
				lConfigFile >> lValue;
				if(lValue == "on")
					lValueB = true;
				else
					lValueB = false;

				if(lField == "surround")
					State::mModProps.mSurround         = lValueB;
				else if(lField == "oversampling")
					State::mModProps.mOversamp         = lValueB;
				else if(lField == "reverb")
					State::mModProps.mReverb           = lValueB;
				else if(lField == "megabass")
					State::mModProps.mMegabass         = lValueB;
				else if(lField == "noisereduction")
					State::mModProps.mNoiseReduction   = lValueB;
				else if(lField == "volumeramping")
					State::mModProps.mVolumeRamp       = lValueB;
				else if(lField == "fadeout")
					State::mModProps.mFadeout          = lValueB;
				else if(lField == "fastinfo")
					State::mModProps.mFastinfo         = lValueB;

				else if(lField == "channels")
				{
					if(lValue == "mono")
						State::mModProps.mChannels       = 1;
					else
						State::mModProps.mChannels       = 2;
				}
				else if(lField == "frequency")
				{
						if(lValue == "22050")
					State::mModProps.mFrequency        = 22050;
					else if(lValue == "11025")
						State::mModProps.mFrequency      = 11025;
					else
						State::mModProps.mFrequency      = 44100;
				}
				else if(lField == "bits")
				{
					if(lValue == "8")
						State::mModProps.mBits           = 8;
					else
						State::mModProps.mBits           = 16;
				}
			} //if(numerical value) else
		}   //if(comment) else
	}     //while(!eof)

	lConfigFile.close();
}

bool CModplugXMMS::CanPlayFile(const string& aFilename)
{
  string lExt;
	uint32 lPos;

	lPos = aFilename.find_last_of('.');
	if(lPos < 0)
		return false;
	lExt = aFilename.substr(lPos);
	for(uint32 i = 0; i < lExt.length(); i++)
		lExt[i] = tolower(lExt[i]);

	if (lExt == ".669")
		return true;
	if (lExt == ".amf")
		return true;
	if (lExt == ".ams")
		return true;
	if (lExt == ".dbm")
		return true;
	if (lExt == ".dbf")
		return true;
	if (lExt == ".dsm")
		return true;
	if (lExt == ".far")
		return true;
	if (lExt == ".it")
		return true;
	if (lExt == ".mdl")
		return true;
	if (lExt == ".med")
		return true;
	if (lExt == ".mod")
		return true;
	if (lExt == ".mtm")
		return true;
	if (lExt == ".okt")
		return true;
	if (lExt == ".ptm")
		return true;
	if (lExt == ".s3m")
		return true;
	if (lExt == ".stm")
		return true;
	if (lExt == ".ult")
		return true;
	if (lExt == ".umx")
		return true;
	if (lExt == ".xm")
		return true;

  return false;
}

static void* CModplugXMMS_PlayLoop(void* arg)
{
	uint32 lLength;
	int lOutBufFree;
	//the user might change the number of channels while playing.
	// we don't want this to take effect until we are done!
	uint8 lChannels = CModplugXMMS::State::mModProps.mChannels;

	//Figure out how large the buffer is so we can keep pre-buffering
	// to a minimum.  This way, equalizer changes happen more quickly.
	lOutBufFree = (uint32)CModplugXMMS::State::mOutPlug->buffer_free();
	lOutBufFree -= CModplugXMMS::State::mBufSize * 2;

	while(!CModplugXMMS::State::mStopped)
	{
		if(!CModplugXMMS::State::mPaused)
		{
			if(!(lLength = CModplugXMMS::State::mSoundFile.Read(
					CModplugXMMS::State::mBuffer,
					CModplugXMMS::State::mBufSize)))
			{
				while(CModplugXMMS::State::mOutPlug->buffer_playing())
					usleep(10000);
				break;
			}
			
			while(CModplugXMMS::State::mOutPlug->buffer_free()
			      < lOutBufFree)
//			      < (int)CModplugXMMS::State::mBufSize)
				usleep(10000);

			CModplugXMMS::State::mOutPlug->write_audio
			(
				CModplugXMMS::State::mBuffer,
				CModplugXMMS::State::mBufSize
			);
			CModplugXMMS::State::mInPlug->add_vis_pcm
			(
				CModplugXMMS::State::mPlayed,
				CModplugXMMS::State::mFormat,
				lChannels,
				CModplugXMMS::State::mBufSize,
				CModplugXMMS::State::mBuffer
			);

			CModplugXMMS::State::mPlayed += CModplugXMMS::State::mBufTime;
		}
		else
			usleep(10000);
	}

	CModplugXMMS::State::mOutPlug->flush(0);
	CModplugXMMS::State::mOutPlug->close_audio();

	//Unload the file
	CModplugXMMS::State::mSoundFile.Destroy();
	munmap
	(
		CModplugXMMS::State::mFileMap,
		CModplugXMMS::State::mFileSize
	);
	CModplugXMMS::State::mFileMap = NULL;
	close(CModplugXMMS::State::mFileDesc);
	CModplugXMMS::State::mFileDesc = -1;

	if (CModplugXMMS::State::mBuffer)
	{
		delete [] CModplugXMMS::State::mBuffer;
		CModplugXMMS::State::mBuffer = NULL;
	}

	CModplugXMMS::State::mPaused = false;
	CModplugXMMS::State::mStopped = true;

	pthread_exit(NULL);
}

void CModplugXMMS::PlayFile(const string& aFilename)
{
	struct stat lStat;   //cursed C-style structs

	State::mStopped = true;
	State::mPaused = false;
	
	pthread_join(State::mDecodeThread, NULL);

	if (!State::mBuffer)
	{
		//find buftime to get approx. 512 samples/block
		State::mBufTime = 512000 / State::mModProps.mFrequency + 1;

		State::mBufSize = State::mBufTime;
		State::mBufSize *= State::mModProps.mFrequency;
		State::mBufSize /= 1000;    //milliseconds
		State::mBufSize *= State::mModProps.mChannels;
		State::mBufSize *= State::mModProps.mBits / 8;

		State::mBuffer = new uchar[State::mBufSize];
		if(!State::mBuffer)
			return;             //out of memory!
	}

	CSoundFile::SetWaveConfig
	(
		State::mModProps.mFrequency,
		State::mModProps.mBits,
		State::mModProps.mChannels
	);
	CSoundFile::SetWaveConfigEx
	(
		State::mModProps.mSurround,
		!State::mModProps.mOversamp,
		State::mModProps.mReverb,
		State::mModProps.mMegabass,
		State::mModProps.mNoiseReduction,
		State::mModProps.mVolumeRamp
	);
	// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
	if(State::mModProps.mReverb)
	{
		CSoundFile::SetReverbParameters
		(
			State::mModProps.mReverbDepth,
			State::mModProps.mReverbDelay
		);
	}
	// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 10-100]
	if(State::mModProps.mMegabass)
	{
		CSoundFile::SetXBassParameters
		(
			State::mModProps.mBassAmount,
			State::mModProps.mBassRange
		);
	}
	// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-40ms]
	if(State::mModProps.mSurround)
	{
		CSoundFile::SetSurroundParameters
		(
			State::mModProps.mSurroundDepth,
			State::mModProps.mSurroundDelay
		);
	}
	
	State::mPaused = false;
	State::mStopped = false;

	//open and mmap the file
	State::mFileDesc = open(aFilename.c_str(), O_RDONLY);
	if(State::mFileDesc == -1)
		return;
	fstat(State::mFileDesc, &lStat);
	State::mFileMap =
		(uchar*)mmap(0, lStat.st_size, PROT_READ,
		MAP_PRIVATE, State::mFileDesc, 0);
	if(!State::mFileMap)
	{
		close(State::mFileDesc);
		return;
	}
	State::mFileSize = lStat.st_size;
	State::mSoundFile.Create
	(
		State::mFileMap,
		State::mFileSize
	);
	State::mPlayed = 0;

	//XMMS wants a non-const char*, but modplug returns a const...
	strncpy(State::mModName, State::mSoundFile.GetTitle(), 100);
	State::mInPlug->set_info
	(
		State::mModName,
		State::mSoundFile.GetSongTime() * 1000,
		State::mSoundFile.GetNumChannels(),
		State::mModProps.mFrequency / 1000,
		State::mModProps.mChannels
	);

	State::mStopped = State::mPaused = false;

	if(State::mModProps.mBits == 16)
		State::mFormat = FMT_S16_LE;
	else
		State::mFormat = FMT_U8;

	State::mOutPlug->open_audio
	(
		State::mFormat,
		State::mModProps.mFrequency,
		State::mModProps.mChannels
	);

	pthread_create
	(
		&State::mDecodeThread,
		NULL,
		CModplugXMMS_PlayLoop,
		NULL
	);
}

void CModplugXMMS::Stop(void)
{
	if(State::mStopped)
		return;

	if(State::mModProps.mFadeout)
		State::mSoundFile.GlobalFadeSong(State::mModProps.mFadeTime);
	else
	{
		State::mStopped = true;
		State::mPaused = false;

		pthread_join(State::mDecodeThread, NULL);
	}
}

void CModplugXMMS::Pause(bool aPaused)
{
	if(aPaused)
		State::mPaused = true;
	else
		State::mPaused = false;
}

void CModplugXMMS::Seek(float32 aTime)
{
	uint32  lMax;
	uint32  lMaxtime;
	float32 lPostime;
	
	if(aTime > (lMaxtime = State::mSoundFile.GetSongTime()))
		aTime = lMaxtime;
	lMax = State::mSoundFile.GetMaxPosition();
	lPostime = float(lMax) / lMaxtime;

	State::mSoundFile.SetCurrentPos(int(aTime * lPostime));

	State::mOutPlug->flush(int(aTime * 1000));
	State::mPlayed = uint32(aTime * 1000);
}

float32 CModplugXMMS::GetTime(void)
{
	if(State::mStopped)
		return -1;
	return (float32)State::mOutPlug->output_time();
}

void CModplugXMMS::GetSongInfo(const string& aFilename, char*& aTitle, int32& aLength)
{
	aLength = -1;

	if(State::mModProps.mFastinfo)
	{
		fstream lModFile;
	  string lExt;
		uint32 lPos;

		lModFile.open(aFilename.c_str(), ios::in | ios::nocreate);

		lPos = aFilename.find_last_of('.');
		if(lPos < 0)
			return;
		lExt = aFilename.substr(lPos);

		if (lExt == ".mod")
		{
			lModFile.read(State::mModName, 20);
			State::mModName[20] = 0;
		}
		else if (lExt == ".s3m")
		{
			lModFile.read(State::mModName, 28);
			State::mModName[28] = 0;
		}
		else if (lExt == ".xm")
		{
			lModFile.seekg(17);
			lModFile.read(State::mModName, 20);
			State::mModName[20] = 0;
		}
		else if (lExt == ".it")
		{
			lModFile.seekg(4);
			lModFile.read(State::mModName, 28);
			State::mModName[28] = 0;
		}
		else
			return;

		lModFile.close();

		aTitle = new char[strlen(State::mModName) + 1];
		strcpy(aTitle, State::mModName);
	}
	else
	{
		int lFileDesc;
		struct stat lStat;   //silly C syntax
		uchar* lFileMap;
		uint32 lFileSize;
		CSoundFile* lSoundFile;
		const char* lTitle;

		//open and mmap the file
		lFileDesc = open(aFilename.c_str(), O_RDONLY);
		if(lFileDesc == -1)
			return;
		fstat(lFileDesc, &lStat);
		lFileMap =
			(uchar*)mmap(0, lStat.st_size, PROT_READ,
			MAP_PRIVATE, lFileDesc, 0);
		if(!lFileMap)
		{
			close(lFileDesc);
			return;
		}
		lFileSize = lStat.st_size;
		lSoundFile = new CSoundFile;
		lSoundFile->Create(lFileMap, lFileSize);

		lTitle = lSoundFile->GetTitle();
		aTitle = new char[strlen(lTitle) + 1];

		strcpy(aTitle, lTitle);
		aLength = lSoundFile->GetSongTime() * 1000;                   //It wants milliseconds!?!

		//unload the file
		lSoundFile->Destroy();
		delete lSoundFile;
		munmap(lFileMap, lFileSize);
		close(lFileDesc);
	}
}

void CModplugXMMS::SetInputPlugin(InputPlugin& aInPlugin)
{
	State::mInPlug = &aInPlugin;
}
void CModplugXMMS::SetOutputPlugin(OutputPlugin& aOutPlugin)
{
	State::mOutPlug = &aOutPlugin;
}

//Only preamp is supported as of now.
//I tried two different bandpass filter algorithms.  They both generated
// intense static. :(  MP3's get free equalizers due to their compression
// format.  Mods don't. :(
void CModplugXMMS::SetEquilizer(bool aOn, float32 aPreamp, float32 *aBands)
{
	uint32 lPreamp;

	State::mEqualizer = aOn;

	if(!aOn)
	{
		State::mSoundFile.SetMasterVolume(128);
	}
	else
	{
		lPreamp = (uint32)((aPreamp / 40 + 0.5) * 256);
		State::mSoundFile.SetMasterVolume(lPreamp);
	}
}

const ModProperties& CModplugXMMS::GetModProps()
{
	return State::mModProps;
}

static const char* CModplugXMMS_Bool2OnOff(bool aValue)
{
	if(aValue)
		return "on";
	else
		return "off";
}

void CModplugXMMS::SetModProps(const ModProperties& aModProps)
{
	fstream lConfigFile;
	string lConfigFilename;

	State::mModProps.mSurround       = aModProps.mSurround;
	State::mModProps.mOversamp       = aModProps.mOversamp;
	State::mModProps.mMegabass       = aModProps.mMegabass;
	State::mModProps.mNoiseReduction = aModProps.mNoiseReduction;
	State::mModProps.mVolumeRamp     = aModProps.mVolumeRamp;
	State::mModProps.mReverb         = aModProps.mReverb;
	State::mModProps.mFadeout        = aModProps.mFadeout;
	State::mModProps.mFastinfo       = aModProps.mFastinfo;

	State::mModProps.mChannels       = aModProps.mChannels;
	State::mModProps.mBits           = aModProps.mBits;
	State::mModProps.mFrequency      = aModProps.mFrequency;

	State::mModProps.mReverbDepth    = aModProps.mReverbDepth;
	State::mModProps.mReverbDelay    = aModProps.mReverbDelay;
	State::mModProps.mBassAmount     = aModProps.mBassAmount;
	State::mModProps.mBassRange      = aModProps.mBassRange;
	State::mModProps.mSurroundDepth  = aModProps.mSurroundDepth;
	State::mModProps.mSurroundDelay  = aModProps.mSurroundDelay;
	State::mModProps.mFadeTime       = aModProps.mFadeTime;

	// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
	if(State::mModProps.mReverb)
	{
		CSoundFile::SetReverbParameters
		(
			State::mModProps.mReverbDepth,
			State::mModProps.mReverbDelay
		);
	}
	// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 10-100]
	if(State::mModProps.mMegabass)
	{
		CSoundFile::SetXBassParameters
		(
			State::mModProps.mBassAmount,
			State::mModProps.mBassRange
		);
	}
	else //modplug seems to ignore the SetWaveConfigEx() setting for bass boost
	{
		CSoundFile::SetXBassParameters
		(
			0,
			0
		);
	}
	// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-40ms]
	if(State::mModProps.mSurround)
	{
		CSoundFile::SetSurroundParameters
		(
			State::mModProps.mSurroundDepth,
			State::mModProps.mSurroundDelay
		);
	}
	CSoundFile::SetWaveConfigEx
	(
		State::mModProps.mSurround,
		!State::mModProps.mOversamp,
		State::mModProps.mReverb,
		State::mModProps.mMegabass,
		State::mModProps.mNoiseReduction,
		State::mModProps.mVolumeRamp
	);

	lConfigFilename = g_get_home_dir();
	lConfigFilename += "/.xmms/modplug-xmms.conf";
	lConfigFile.open(lConfigFilename.c_str(), ios::out);

	lConfigFile << "# Modplug XMMS plugin config file\n"
	            << "# Modplug (C) 1999 Olivier Lapicque\n"
	            << "# XMMS port (C) 1999 Kenton Varda\n" << endl;

	lConfigFile << "# ---Effects---"  << endl;
	lConfigFile << "reverb          " << CModplugXMMS_Bool2OnOff(State::mModProps.mReverb)         << endl;
	lConfigFile << "reverb_depth    " << State::mModProps.mReverbDepth   << endl;
	lConfigFile << "reverb_delay    " << State::mModProps.mReverbDelay   << endl;
	lConfigFile << endl;
	lConfigFile << "surround        " << CModplugXMMS_Bool2OnOff(State::mModProps.mSurround)       << endl;
	lConfigFile << "surround_depth  " << State::mModProps.mSurroundDepth << endl;
	lConfigFile << "surround_delay  " << State::mModProps.mSurroundDelay << endl;
	lConfigFile << endl;
	lConfigFile << "megabass        " << CModplugXMMS_Bool2OnOff(State::mModProps.mMegabass)       << endl;
	lConfigFile << "megabass_amount " << State::mModProps.mBassAmount    << endl;
	lConfigFile << "megabass_range  " << State::mModProps.mBassRange     << endl;
	lConfigFile << endl;
	lConfigFile << "oversampling    " << CModplugXMMS_Bool2OnOff(State::mModProps.mOversamp)       << endl;
	lConfigFile << "noisereduction  " << CModplugXMMS_Bool2OnOff(State::mModProps.mNoiseReduction) << endl;
	lConfigFile << "volumeramping   " << CModplugXMMS_Bool2OnOff(State::mModProps.mVolumeRamp)     << endl;
	lConfigFile << "fastinfo        " << CModplugXMMS_Bool2OnOff(State::mModProps.mFastinfo)       << endl;
	lConfigFile << endl;
	lConfigFile << "fadeout         " << CModplugXMMS_Bool2OnOff(State::mModProps.mFadeout)        << endl;
	lConfigFile << "fadeout_time    " << State::mModProps.mFadeTime      << endl;
	lConfigFile << endl;

	lConfigFile << "# ---Quality---" << endl;
	lConfigFile << "channels        ";
	if(State::mModProps.mChannels == 1)
		lConfigFile << "mono" << endl;
	else
		lConfigFile << "stereo" << endl;
	lConfigFile << "bits            "      << (int)State::mModProps.mBits << endl;
	lConfigFile << "frequency       " << State::mModProps.mFrequency << endl;

	lConfigFile.close();
}
