/*
 * unlha.h
 * -------
 * Purpose: Header file for .lha loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "archive.h"

typedef struct _LHAInputStream LHAInputStream;
typedef struct _LHAReader LHAReader;
typedef struct _LHAFileHeader LHAFileHeader;

//====================================
class CLhaArchive : public ArchiveBase
//====================================
{
private:
	LHAInputStream *inputstream;
	LHAReader *reader;
	LHAFileHeader *firstfile;
	void OpenArchive();
	void CloseArchive();
public:
	CLhaArchive(FileReader &file);
	virtual ~CLhaArchive();
public:
	virtual bool ExtractFile(std::size_t index);
};
