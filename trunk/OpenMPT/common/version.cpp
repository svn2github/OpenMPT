/*
 * version.cpp
 * -----------
 * Purpose: OpenMPT version handling.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "version.h"

#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

#include "versionNumber.h"
#include "svn_version.h"

OPENMPT_NAMESPACE_BEGIN

namespace MptVersion {

static_assert((MPT_VERSION_NUMERIC & 0xffff) != 0x0000, "Version numbers ending in .00.00 shall never exist again, as they make interpreting the version number ambiguous for file formats which can only store the two major parts of the version number (e.g. IT and S3M).");

const VersionNum num = MPT_VERSION_NUMERIC;

const char * const str = MPT_VERSION_STR;

std::string GetOpenMPTVersionStr()
{
	return std::string("OpenMPT " MPT_VERSION_STR);
}

struct version
{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
	version() : a(0),b(0),c(0),d(0) {}
	VersionNum Get() const
	{
		return (((a&0xff) << 24) | ((b&0xff) << 16) | ((c&0xff) << 8) | (d&0xff));
	}
};

VersionNum ToNum(const std::string &s_)
{
	std::istringstream s(s_);
	s.imbue(std::locale::classic());
	version v;
	char dot = '\0';
	s >> std::hex >> v.a;
	s >> dot; if(dot != '.') return v.Get();
	s >> std::hex >> v.b;
	s >> dot; if(dot != '.') return v.Get();
	s >> std::hex >> v.c;
	s >> dot; if(dot != '.') return v.Get();
	s >> std::hex >> v.d;
	s >> dot; if(dot != '.') return v.Get();
	return v.Get();
}

std::string ToStr(const VersionNum v)
{
	if(v == 0)
	{
		// Unknown version
		return "Unknown";
	} else if((v & 0xFFFF) == 0)
	{
		// Only parts of the version number are known (e.g. when reading the version from the IT or S3M file header)
		return mpt::String::Print("%1.%2", mpt::fmt::HEX((v >> 24) & 0xFF), mpt::fmt::HEX0<2>((v >> 16) & 0xFF));
	} else
	{
		// Full version info available
		return mpt::String::Print("%1.%2.%3.%4", mpt::fmt::HEX((v >> 24) & 0xFF), mpt::fmt::HEX0<2>((v >> 16) & 0xFF), mpt::fmt::HEX0<2>((v >> 8) & 0xFF), mpt::fmt::HEX0<2>((v) & 0xFF));
	}
}

VersionNum RemoveBuildNumber(const VersionNum num_)
{
	return (num_ & 0xFFFFFF00);
}

bool IsTestBuild(const VersionNum num_)
{
	return (
			// Legacy
			(num_ > MAKE_VERSION_NUMERIC(1,17,02,54) && num_ < MAKE_VERSION_NUMERIC(1,18,02,00) && num_ != MAKE_VERSION_NUMERIC(1,18,00,00))
		||
			// Test builds have non-zero VER_MINORMINOR
			(num_ > MAKE_VERSION_NUMERIC(1,18,02,00) && RemoveBuildNumber(num_) != num_)
		);
}

bool IsDebugBuild()
{
	#ifdef _DEBUG
		return true;
	#else
		return false;
	#endif
}

std::string GetUrl()
{
	#ifdef OPENMPT_VERSION_URL
		return OPENMPT_VERSION_URL;
	#else
		return "";
	#endif
}

int GetRevision()
{
	#if defined(OPENMPT_VERSION_REVISION)
		return OPENMPT_VERSION_REVISION;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return 0;
		}
		if(svnversion.find(":") != std::string::npos)
		{
			svnversion = svnversion.substr(svnversion.find(":") + 1);
		}
		if(svnversion.find("-") != std::string::npos)
		{
			svnversion = svnversion.substr(svnversion.find("-") + 1);
		}
		if(svnversion.find("M") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("M"));
		}
		if(svnversion.find("S") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("S"));
		}
		if(svnversion.find("P") != std::string::npos)
		{
			svnversion = svnversion.substr(0, svnversion.find("P"));
		}
		return ConvertStrTo<int>(svnversion);
	#else
		#if MPT_COMPILER_MSVC
			#pragma message("SVN revision unknown. Please check your build system.")
		#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
			#warning "SVN revision unknown. Please check your build system."
		#else
			// There is no portable way to display a warning.
			// Try to provoke a warning with an unused variable.
			int SVN_revision_unknown__Please_check_your_build_system;
		#endif
		return 0;
	#endif
}

bool IsDirty()
{
	#if defined(OPENMPT_VERSION_DIRTY)
		return OPENMPT_VERSION_DIRTY != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find("M") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

bool HasMixedRevisions()
{
	#if defined(OPENMPT_VERSION_MIXEDREVISIONS)
		return OPENMPT_VERSION_MIXEDREVISIONS != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find(":") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("-") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("S") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("P") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

bool IsPackage()
{
	#if defined(OPENMPT_VERSION_IS_PACKAGE)
		return OPENMPT_VERSION_IS_PACKAGE != 0;
	#else
		return false;
	#endif
}

std::string GetStateString()
{
	std::string retval;
	if(HasMixedRevisions())
	{
		retval += "+mixed";
	}
	if(IsDirty())
	{
		retval += "+dirty";
	}
	if(IsPackage())
	{
		retval += "-pkg";
	}
	return retval;
}

std::string GetBuildDateString()
{
	#if defined(OPENMPT_VERSION_DATE)
		return OPENMPT_VERSION_DATE;
	#else
		return __DATE__ " " __TIME__ ;
	#endif
}

std::string GetBuildFlagsString()
{
	std::string retval;
	#ifdef MODPLUG_TRACKER
		if(IsTestBuild())
		{
			retval += " TEST";
		}
	#endif // MODPLUG_TRACKER
	if(IsDebugBuild())
	{
		retval += " DEBUG";
	}
	return retval;
}

std::string GetBuildFeaturesString()
{
	std::string retval;
	#ifdef LIBOPENMPT_BUILD
		#if MPT_COMPILER_GENERIC
			retval += "*C++11";
		#elif MPT_COMPILER_MSVC
			retval += mpt::String::Print("*MSVC-%1.%2", MPT_COMPILER_MSVC_VERSION / 100, MPT_COMPILER_MSVC_VERSION % 100);
		#elif MPT_COMPILER_GCC
			retval += mpt::String::Print("*GCC-%1.%2.%3", MPT_COMPILER_GCC_VERSION / 10000, (MPT_COMPILER_GCC_VERSION / 100) % 100, MPT_COMPILER_GCC_VERSION % 100);
		#elif MPT_COMPILER_CLANG
			retval += mpt::String::Print("*Clang-%1.%2.%3", MPT_COMPILER_CLANG_VERSION / 10000, (MPT_COMPILER_CLANG_VERSION / 100) % 100, MPT_COMPILER_CLANG_VERSION % 100);
		#else
			retval += "*unknown";
		#endif
		#if defined(MPT_CHARSET_WIN32)
			retval += " +WINAPI";
		#elif defined(MPT_CHARSET_ICONV)
			retval += " +ICONV";
		#elif defined(MPT_CHARSET_CODECVTUTF8)
			retval += " +CODECVTUTF8";
		#elif defined(MPT_CHARSET_INTERNAL)
			retval += " +INTERNALCHARSETS";
		#endif
		#if !defined(NO_ZLIB)
			retval += " +ZLIB";
		#elif !defined(NO_MINIZ)
			retval += " +MINIZ";
		#else
			retval += " -INFLATE";
		#endif
		#if !defined(NO_MO3)
			retval += " +UNMO3";
		#else
			retval += " -UNMO3";
		#endif
	#endif
	#ifdef MODPLUG_TRACKER
		if(IsForOlderWindows())
		{
			retval += " OLDWIN";
		}
		#ifdef NO_VST
			retval += " NO_VST";
		#endif
		#ifdef NO_ASIO
			retval += " NO_ASIO";
		#endif
		#ifdef NO_DSOUND
			retval += " NO_DSOUND";
		#endif
	#endif
	return retval;
}

std::string GetBuildCompilerString()
{
	std::string retval;
	#if MPT_COMPILER_GENERIC
		retval += "*Generic C++11 Compiler";
	#elif MPT_COMPILER_MSVC
		#if defined(_MSC_FULL_VER) && defined(_MSC_BUILD) && (_MSC_BUILD > 0)
			retval += MPT_FORMAT("Microsoft Compiler %1.%2.%3.%4"
				, _MSC_FULL_VER / 10000000
				, mpt::fmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::fmt::dec0<5>(_MSC_FULL_VER % 100000)
				, mpt::fmt::dec0<2>(_MSC_BUILD)
				);
		#elif defined(_MSC_FULL_VER)
			retval += MPT_FORMAT("Microsoft Compiler %1.%2.%3"
				, _MSC_FULL_VER / 10000000
				, mpt::fmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::fmt::dec0<5>(_MSC_FULL_VER % 100000)
				);
		#else
			retval += MPT_FORMAT("Microsoft Compiler %1.%2", MPT_COMPILER_MSVC_VERSION / 100, MPT_COMPILER_MSVC_VERSION % 100);
		#endif
	#elif MPT_COMPILER_GCC
		retval += MPT_FORMAT("GNU Compiler Collection %1.%2.%3", MPT_COMPILER_GCC_VERSION / 10000, (MPT_COMPILER_GCC_VERSION / 100) % 100, MPT_COMPILER_GCC_VERSION % 100);
	#elif MPT_COMPILER_CLANG
		retval += MPT_FORMAT("Clang %1.%2.%3", MPT_COMPILER_CLANG_VERSION / 10000, (MPT_COMPILER_CLANG_VERSION / 100) % 100, MPT_COMPILER_CLANG_VERSION % 100);
	#else
		retval += "*unknown";
	#endif
	return retval;
}

std::string GetRevisionString()
{
	std::string result;
	if(GetRevision() == 0)
	{
		return result;
	}
	result = std::string("-r") + mpt::ToString(GetRevision());
	if(HasMixedRevisions())
	{
		result += "!";
	}
	if(IsDirty())
	{
		result += "+";
	}
	if(IsPackage())
	{
		result += "p";
	}
	return result;
}

bool IsForOlderWindows()
{
	#ifdef MODPLUG_TRACKER
		return true
			&& (GetMinimumSSEVersion() <= 0)
			&& (GetMinimumAVXVersion() <= 0)
			&& (mpt::Windows::Version::GetMinimumKernelLevel() < mpt::Windows::Version::WinXP)
			&& (mpt::Windows::Version::GetMinimumAPILevel() < mpt::Windows::Version::WinXP)
			;
	#else
		return false;
	#endif
}

mpt::ustring GetDownloadURL()
{
	#ifdef MODPLUG_TRACKER
		return (MptVersion::IsDebugBuild() || MptVersion::IsTestBuild() || MptVersion::IsDirty() || MptVersion::HasMixedRevisions())
			?
				MPT_USTRING("http://buildbot.openmpt.org/builds/")
			:
				MPT_USTRING("http://openmpt.org/download")
			;
	#else
		return MPT_USTRING("http://lib.openmpt.org/");
	#endif
}

static std::string GetVersionString(bool verbose)
{
	#if !defined(MODPLUG_TRACKER)
		MPT_UNREFERENCED_PARAMETER(verbose);
	#endif
	std::string retval = MPT_VERSION_STR;
	if(IsDebugBuild() || IsTestBuild() || IsDirty() || HasMixedRevisions())
	{
		retval += GetRevisionString();
	}
	#ifdef MODPLUG_TRACKER
		if(verbose)
		{
			retval += MPT_FORMAT(" %1 bit", sizeof(void*) * 8);
		}
	#endif
	if(IsDebugBuild() || IsTestBuild() || IsDirty() || HasMixedRevisions())
	{
		retval += GetBuildFlagsString();
	}
	#ifdef MODPLUG_TRACKER
		if(verbose)
		{
			retval += GetBuildFeaturesString();
		}
	#endif
	return retval;
}

std::string GetVersionStringSimple()
{
	return GetVersionString(false);
}

std::string GetVersionStringExtended()
{
	return GetVersionString(true);
}

std::string GetVersionUrlString()
{
	if(GetRevision() == 0)
	{
		return "";
	}
	#if defined(OPENMPT_VERSION_URL)
		std::string url = OPENMPT_VERSION_URL;
	#else
		std::string url = "";
	#endif
	std::string baseurl = "https://source.openmpt.org/svn/openmpt/";
	if(url.substr(0, baseurl.length()) == baseurl)
	{
		url = url.substr(baseurl.length());
	}
	return url + "@" + mpt::ToString(GetRevision()) + GetStateString();
}

mpt::ustring GetContactString()
{
	return MPT_UTF8(
		"Contact / Discussion:\n"
		"http://forum.openmpt.org/\n"
		"\n"
		"Updates:\n"
		"http://openmpt.org/download\n"
		);
}

mpt::ustring GetFullCreditsString()
{
	return mpt::ToUnicode(mpt::CharsetUTF8,
#ifdef MODPLUG_TRACKER
		"OpenMPT / ModPlug Tracker\n"
#else
		"libopenmpt (based on OpenMPT / ModPlug Tracker)\n"
#endif
		"\n"
		"Copyright \xC2\xA9 2004-2015 Contributors\n"
		"Copyright \xC2\xA9 1997-2003 Olivier Lapicque\n"
		"\n"
		"Contributors:\n"
		"Johannes Schultz (2008-2015)\n"
		"J\xC3\xB6rn Heusipp (2012-2015)\n"
		"Ahti Lepp\xC3\xA4nen (2005-2011)\n"
		"Robin Fernandes (2004-2007)\n"
		"Sergiy Pylypenko (2007)\n"
		"Eric Chavanon (2004-2005)\n"
		"Trevor Nunes (2004)\n"
		"Olivier Lapicque (1997-2003)\n"
		"\n"
		"Additional patch submitters:\n"
		"coda (http://coda.s3m.us/)\n"
		"kode54 (https://kode54.net/)\n"
		"xaimus (http://xaimus.com/)\n"
		"\n"
		"Thanks to:\n"
		"\n"
		"Konstanty for the XMMS-ModPlug resampling implementation\n"
		"http://modplug-xmms.sourceforge.net/\n"
		"\n"
#ifdef MODPLUG_TRACKER
		"Stephan M. Bernsee for pitch shifting source code\n"
		"http://www.dspdimension.com/\n"
		"\n"
		"Aleksey Vaneev of Voxengo for r8brain sample rate converter\n"
		"https://github.com/avaneev/r8brain-free-src\n"
		"\n"
		"Olli Parviainen for SoundTouch Library (time stretching)\n"
		"http://www.surina.net/soundtouch/\n"
		"\n"
#endif
#ifndef NO_VST
		"Hermann Seib for his example VST Host implementation\n"
		"http://www.hermannseib.com/english/vsthost.htm\n"
		"\n"
#endif
#ifndef NO_MO3
		"Ian Luck for UNMO3\n"
		"http://www.un4seen.com/mo3.html\n"
		"\n"
#endif
		"Ben \"GreaseMonkey\" Russell for IT sample compression code\n"
		"https://github.com/iamgreaser/it2everything/\n"
		"\n"
		"Alexander Chemeris for msinttypes\n"
		"https://github.com/chemeris/msinttypes\n"
		"\n"
#ifndef NO_ZLIB
		"Jean-loup Gailly and Mark Adler for zlib\n"
		"http://zlib.net/\n"
		"\n"
#endif
#ifndef NO_MINIZ
		"Rich Geldreich for miniz\n"
		"https://github.com/richgel999/miniz\n"
		"\n"
#endif
#ifndef NO_ARCHIVE_SUPPORT
		"Simon Howard for lhasa\n"
		"http://fragglet.github.io/lhasa/\n"
		"\n"
		"Alexander L. Roshal for UnRAR\n"
		"http://rarlab.com/\n"
		"\n"
#endif
#ifndef NO_PORTAUDIO
		"PortAudio contributors\n"
		"http://www.portaudio.com/\n"
		"\n"
#endif
#ifndef NO_FLAC
		"Josh Coalson for libFLAC\n"
		"http://flac.sourceforge.net/\n"
		"\n"
#endif
#ifndef NO_MP3_SAMPLES
		"The mpg123 project for libmpg123\n"
		"http://mpg123.de/\n"
		"\n"
#endif
#ifndef NO_OGG
		"Xiph.Org Foundation for libogg\n"
		"https://xiph.org/ogg/\n"
		"\n"
#endif
		"Storlek for all the IT compatibility hints and testcases\n"
		"as well as the IMF, OKT and ULT loaders\n"
		"http://schismtracker.org/\n"
		"\n"
#ifdef MODPLUG_TRACKER
		"Alexander Uckun for decimal input field\n"
		"http://www.codeproject.com/Articles/21257/_\n"
		"\n"
		"Nobuyuki for application and file icon\n"
		"http://twitter.com/nobuyukinyuu\n"
		"\n"
#endif
		"The people at ModPlug forums for crucial contribution\n"
		"in the form of ideas, testing and support;\n"
		"thanks particularly to:\n"
		"33, 8bitbubsy, Anboi, BooT-SectoR-ViruZ, Bvanoudtshoorn\n"
		"christofori, Diamond, Ganja, Georg, Goor00, jmkz,\n"
		"KrazyKatz, LPChip, Nofold, Rakib, Sam Zen\n"
		"Skaven, Skilletaudio, Snu, Squirrel Havoc, Waxhead\n"
		"\n"
#ifndef NO_VST
		"VST PlugIn Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
#ifndef NO_ASIO
		"ASIO Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
		);
}

mpt::ustring GetLicenseString()
{
	return MPT_UTF8(
		"The OpenMPT code is licensed under the BSD license." "\n"
		"" "\n"
		"Copyright (c) 2004-2015, OpenMPT contributors" "\n"
		"Copyright (c) 1997-2003, Olivier Lapicque" "\n"
		"All rights reserved." "\n"
		"" "\n"
		"Redistribution and use in source and binary forms, with or without" "\n"
		"modification, are permitted provided that the following conditions are met:" "\n"
		"    * Redistributions of source code must retain the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer." "\n"
		"    * Redistributions in binary form must reproduce the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer in the" "\n"
		"      documentation and/or other materials provided with the distribution." "\n"
		"    * Neither the name of the OpenMPT project nor the" "\n"
		"      names of its contributors may be used to endorse or promote products" "\n"
		"      derived from this software without specific prior written permission." "\n"
		"" "\n"
		"THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY" "\n"
		"EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" "\n"
		"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE" "\n"
		"DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY" "\n"
		"DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES" "\n"
		"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;" "\n"
		"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND" "\n"
		"ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT" "\n"
		"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS" "\n"
		"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." "\n"
		);
}

} // namespace MptVersion

OPENMPT_NAMESPACE_END
