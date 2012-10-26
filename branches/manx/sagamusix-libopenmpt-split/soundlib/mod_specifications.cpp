/*
 * mod_specifications.cpp
 * ----------------------
 * Purpose: Mod specifications characterise the features of every editable module format in OpenMPT, such as the number of supported channels, samples, effects, etc...
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "precompiled.h"
#include "mod_specifications.h"
#include "../common/misc_util.h"


MODTYPE CModSpecifications::ExtensionToType(LPCTSTR pszExt)
//---------------------------------------------------------
{
	if (pszExt == nullptr)
		return MOD_TYPE_NONE;
	if (pszExt[0] == '.')
		pszExt++;
	char szExtA[CountOf(ModSpecs::mod.fileExtension)];
	MemsetZero(szExtA);
	const size_t nLength = _tcslen(pszExt);
	if (nLength >= CountOf(szExtA))
		return MOD_TYPE_NONE;
	for(size_t i = 0; i < nLength; i++)
		szExtA[i] = static_cast<char>(pszExt[i]);

	for(size_t i = 0; i < CountOf(ModSpecs::Collection); i++)
	{
		if (!lstrcmpiA(szExtA, ModSpecs::Collection[i]->fileExtension))
		{
			return ModSpecs::Collection[i]->internalType;
		}
	}
	// Special case: ITP files...
	if(!lstrcmpi(szExtA, _T("itp")))
	{
		return MOD_TYPE_IT;
	}

	return MOD_TYPE_NONE;

}


bool CModSpecifications::HasNote(ModCommand::NOTE note) const
//------------------------------------------------------------
{
	if(note >= noteMin && note <= noteMax)
		return true;
	else if(note >= NOTE_MIN_SPECIAL && note <= NOTE_MAX_SPECIAL)
	{
		if(note == NOTE_NOTECUT)
			return hasNoteCut;
		else if(note == NOTE_KEYOFF)
			return hasNoteOff;
		else if(note == NOTE_FADE)
			return hasNoteFade;
		else
			return (internalType == MOD_TYPE_MPT);
	} else if(note == NOTE_NONE)
		return true;
	return false;
}


bool CModSpecifications::HasVolCommand(ModCommand::VOLCMD volcmd) const
//---------------------------------------------------------------------
{
	if(volcmd >= MAX_VOLCMDS) return false;
	if(volcommands[volcmd] == '?') return false;
	return true;
}


bool CModSpecifications::HasCommand(ModCommand::COMMAND cmd) const
//----------------------------------------------------------------
{
	if(cmd >= MAX_EFFECTS) return false;
	if(commands[cmd] == '?') return false;
	return true;
}


char CModSpecifications::GetVolEffectLetter(ModCommand::VOLCMD volcmd) const
//--------------------------------------------------------------------------
{
	if(volcmd >= MAX_VOLCMDS) return '?';
	return volcommands[volcmd];
}


char CModSpecifications::GetEffectLetter(ModCommand::COMMAND cmd) const
//---------------------------------------------------------------------
{
	if(cmd >= MAX_EFFECTS) return '?';
	return commands[cmd];
}
