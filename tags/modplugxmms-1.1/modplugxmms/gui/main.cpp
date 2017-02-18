/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "main.h"

#include<strstream>
//open()
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
//mmap()
#include<unistd.h>
#include<sys/mman.h>
#include<fstream>

#include"modplugxmms/stddefs.h"
#include"modplugxmms/soundfile/stdafx.h"
#include"modplugxmms/soundfile/sndfile.h"

GtkWidget *AboutWin    = NULL;
GtkWidget *ConfigWin   = NULL;
GtkWidget *InfoWin     = NULL;

void ShowAboutWindow()
{
	if(!AboutWin)
		AboutWin = create_About();
	gtk_widget_show(AboutWin);
}

void ShowConfigureWindow(const ModProperties& aProps)
{
	if(!ConfigWin)
		ConfigWin = create_Config();
	
	if(aProps.mBits == 8)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "bit8"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "bit16"), TRUE);
	
	if(aProps.mFrequency == 11025)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "samp11"), TRUE);
	else if (aProps.mFrequency == 22050)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "samp22"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "samp44"), TRUE);
	
	if(aProps.mChannels == 1)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "mono"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "stereo"), TRUE);
	
	if(aProps.mOversamp)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxOversamp"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxOversamp"), FALSE);
	
	if(aProps.mVolumeRamp)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxVolRamp"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxVolRamp"), FALSE);
	
	if(aProps.mNoiseReduction)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxNR"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxNR"), FALSE);
	
	if(aProps.mFastinfo)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxFastInfo"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxFastInfo"), FALSE);
	
	if(aProps.mReverb)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxReverb"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxReverb"), FALSE);
	
	if(aProps.mMegabass)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxBassBoost"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxBassBoost"), FALSE);
	
	if(aProps.mSurround)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxSurround"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxSurround"), FALSE);
	
	if(aProps.mFadeout)
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxFadeOut"), TRUE);
	else
		gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(ConfigWin, "fxFadeOut"), FALSE);
	
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxReverbDepth")),   aProps.mReverbDepth);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxReverbDelay")),   aProps.mReverbDelay);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxBassAmount")),    aProps.mBassAmount);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxBassRange")),     aProps.mBassRange);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxSurroundDepth")), aProps.mSurroundDepth);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxSurroundDelay")), aProps.mSurroundDelay);
	gtk_adjustment_set_value(gtk_range_get_adjustment((GtkRange*)lookup_widget(ConfigWin, "fxFadeTime")),      aProps.mFadeTime);
	
	gtk_widget_show(ConfigWin);
}

void ShowInfoWindow(const string& aFilename)
{
	if(!InfoWin)
		InfoWin = create_Info();

	uint32 lSongTime, lNumSamples, lNumInstruments, i;
	string lInfo;
	char lBuffer[33];
	strstream lStrStream(lBuffer, 33);   //C++ replacement for sprintf()

	int lFileDesc;
	uchar* lFileMap;
	uint32 lFileSize;
	struct stat lStat;
	CSoundFile* lSoundFile;

	string lShortFN;
	uint32 lPos;

	lPos = aFilename.find_last_of('/') + 1;
	lShortFN = aFilename.substr(lPos);

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

	lInfo = lShortFN;
	lInfo += '\n';
	lInfo += lSoundFile->GetTitle();
	lInfo += '\n';

	switch(lSoundFile->GetType())
	{
	case MOD_TYPE_MOD:
		lInfo+= "ProTracker";
		break;
	case MOD_TYPE_S3M:
		lInfo+= "Scream Tracker 3";
		break;
	case MOD_TYPE_XM:
		lInfo+= "Fast Tracker 2";
		break;
	case MOD_TYPE_IT:
		lInfo+= "Impulse Tracker";
		break;
	default:
		lInfo+= "Oddball";  //one of them wierdo types...
		break;
	}
	lInfo += '\n';

	lSongTime = lSoundFile->GetSongTime();
	lStrStream.seekp(0);
	lStrStream << lSongTime / 60 << '\0';
	lInfo += lBuffer;
	lInfo += ':';
	lStrStream.seekp(0);
	lStrStream << lSongTime % 60 << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)lSoundFile->GetMusicSpeed() << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)lSoundFile->GetMusicTempo() << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)(lNumSamples = lSoundFile->GetNumSamples() + 1) << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)(lNumInstruments = lSoundFile->GetNumInstruments() + 1) << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)(lSoundFile->GetNumPatterns() + 1) << '\0';
	lInfo += lBuffer;
	lInfo += '\n';

	lStrStream.seekp(0);
	lStrStream << (int)lSoundFile->GetNumChannels() << '\0';
	lInfo += lBuffer;

	gtk_label_set_text((GtkLabel*)lookup_widget(InfoWin, "info_general"), lInfo.c_str());

	lInfo = "";
	for(i = 0; i < lNumSamples; i++)
	{
		lSoundFile->GetSampleName(i, lBuffer);
		lInfo += lBuffer;
		lInfo += '\n';
	}
	gtk_label_set_text((GtkLabel*)lookup_widget(InfoWin, "info_samples"), lInfo.c_str());

	lInfo = "";
	for(i = 0; i < lNumInstruments; i++)
	{
		lSoundFile->GetInstrumentName(i, lBuffer);
		lInfo += lBuffer;
		lInfo += '\n';
	}
	gtk_label_set_text((GtkLabel*)lookup_widget(InfoWin, "info_instruments"), lInfo.c_str());

	//unload the file
	lSoundFile->Destroy();
	delete lSoundFile;
	munmap(lFileMap, lFileSize);
	close(lFileDesc);

	gtk_widget_show(InfoWin);
}