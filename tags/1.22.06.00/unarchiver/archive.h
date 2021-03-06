/*
 * archive.h
 * ---------
 * Purpose: archive loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "../soundlib/FileReader.h"
#include <string>
#include <vector>

enum ArchiveFileType
{
	ArchiveFileInvalid,
	ArchiveFileNormal,
	ArchiveFileSpecial,
};

struct ArchiveFileInfo
{
	std::string name;
	ArchiveFileType type;
	uint64 size;
	std::string comment;
	uint64 cookie1;
	uint64 cookie2;
	ArchiveFileInfo()
		: name(std::string())
		, type(ArchiveFileInvalid)
		, size(0)
		, comment(std::string())
		, cookie1(0)
		, cookie2(0)
	{
		return;
	}
};

//============
class IArchive
//============
{
public:
	typedef std::vector<ArchiveFileInfo>::const_iterator const_iterator;
protected:
	IArchive() {}
public:
	virtual ~IArchive() {};
public:
	virtual bool IsArchive() const = 0;
	virtual std::string GetComment() const = 0;
	virtual bool ExtractFile(std::size_t index) = 0;
	virtual FileReader GetOutputFile() const = 0;
	virtual std::size_t size() const = 0;
	virtual IArchive::const_iterator begin() const = 0;
	virtual IArchive::const_iterator end() const = 0;
	virtual const ArchiveFileInfo & at(std::size_t index) const = 0;
	virtual const ArchiveFileInfo & operator [] (std::size_t index) const = 0;
};

//=================================
class ArchiveBase : public IArchive
//=================================
{
protected:
	FileReader inFile;
	std::string comment;
	std::vector<ArchiveFileInfo> contents;
	std::vector<char> data;
public:
	ArchiveBase(const FileReader &inFile)
		: inFile(inFile)
	{
		return;
	}
	virtual ~ArchiveBase()
	{
		return;
	}
	virtual bool ExtractFile(std::size_t index) { MPT_UNREFERENCED_PARAMETER(index); return false; } // overwrite this
public:
	virtual bool IsArchive() const
	{
		return !contents.empty();
	}
	virtual std::string GetComment() const
	{
		return comment;
	}
	virtual FileReader GetOutputFile() const
	{
		return FileReader(&data[0], data.size());
	}
	virtual std::size_t size() const { return contents.size(); }
	virtual IArchive::const_iterator begin() const { return contents.begin(); }
	virtual IArchive::const_iterator end() const { return contents.end(); }
	virtual const ArchiveFileInfo & at(std::size_t index) const { return contents.at(index); }
	virtual const ArchiveFileInfo & operator [] (std::size_t index) const { return contents[index]; }
};
