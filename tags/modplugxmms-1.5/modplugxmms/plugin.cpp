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

#include"plugin.h"                  //XMMS plugin structs
#include"modplugxmms.h"             //the plugin
#include"gui/main.h"                //gui stuff

static void Init(void);
static int CanPlayFile(char* aFilename)
{
	if(gModplugXMMS.CanPlayFile(aFilename))
		return 1;
	return 0;
}

static void PlayFile(char* aFilename);
static void Stop(void)
{
	gModplugXMMS.Stop();
}
static void Pause(short aPaused)
{
	gModplugXMMS.Pause((bool)aPaused);
}

static void Seek(int aTime)
{
	gModplugXMMS.Seek(float32(aTime));
}
static int GetTime(void)
{
	float32 lTime;
	
	lTime = gModplugXMMS.GetTime();
	if(lTime == -1)
		return -1;
	else
		return (int)(lTime * 1000);
}

static void GetSongInfo(char* aFilename, char** aTitle, int* aLength)
{
	gModplugXMMS.GetSongInfo(aFilename, *aTitle, *aLength);
}

void ShowAboutBox(void)
{
	ShowAboutWindow();
}

void ShowConfigureBox(void)
{
	ShowConfigureWindow(gModplugXMMS.GetModProps());
}

void ShowFileInfoBox(char* aFilename)
{
	ShowInfoWindow(aFilename);
}

extern InputPlugin gModPlug;
InputPlugin gModPlug =
{
	NULL,
	NULL,
	"ModPlug Player",
	Init,
	ShowAboutBox,
	ShowConfigureBox,
	CanPlayFile,
	NULL,
	PlayFile,
	Stop,
	Pause,
	Seek,
	NULL,
	GetTime,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	GetSongInfo,
	ShowFileInfoBox,
	NULL
};

static void Init(void)
{
	gModplugXMMS.SetInputPlugin(gModPlug);
	gModplugXMMS.Init();
}
static void PlayFile(char* aFilename)
{
	gModplugXMMS.SetOutputPlugin(*gModPlug.output);
	gModplugXMMS.PlayFile(aFilename);
}

extern "C"
{
	InputPlugin* get_iplugin_info (void)
	{
		return &gModPlug;
	}
}
