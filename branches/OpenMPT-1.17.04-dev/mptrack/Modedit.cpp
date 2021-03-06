// modedit.cpp : CModDoc operations
//

#include "stdafx.h"
#include "mptrack.h"
#include "mainfrm.h"
#include "moddoc.h"
#include "dlg_misc.h"
#include "dlsbank.h"
#include "modsmp_ctrl.h"
#include "misc_util.h"

#pragma warning(disable:4244) //"conversion from 'type1' to 'type2', possible loss of data"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define str_mptm_conversion_warning		GetStrI18N(_TEXT("Conversion from mptm to any other moduletype may makes certain features unavailable and is not guaranteed to work properly. Do the conversion anyway?"))

const size_t Pow10Table[10] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

// Return D'th digit(character) of given value.
// GetDigit<0>(123) == '3'
// GetDigit<1>(123) == '2'
// GetDigit<2>(123) == '1'
template<BYTE D>
inline TCHAR GetDigit(const size_t val)
{
	return (D > 9) ? '0' : 48 + ((val / Pow10Table[D]) % 10);
}


//////////////////////////////////////////////////////////////////////
// Module type conversion

BOOL CModDoc::ChangeModType(UINT nNewType)
//----------------------------------------
{
	CHAR s[256];
	UINT b64 = 0;

	const MODTYPE oldtype = m_SndFile.GetType();
	
	if (nNewType == oldtype && nNewType == MOD_TYPE_IT){
		// Even if m_nType doesn't change, we might need to change extension in itp<->it case.
		// This is because ITP is a HACK and doesn't genuinely change m_nType,
		// but uses flages instead.
		ChangeFileExtension(nNewType);
		return TRUE;
	}

	if(nNewType == oldtype) return TRUE;

	const bool oldTypeIsMOD = (oldtype == MOD_TYPE_MOD), oldTypeIsXM = (oldtype == MOD_TYPE_XM),
				oldTypeIsS3M = (oldtype == MOD_TYPE_S3M), oldTypeIsIT = (oldtype == MOD_TYPE_IT),
				oldTypeIsMPT = (oldtype == MOD_TYPE_MPT), oldTypeIsMOD_XM = (oldTypeIsMOD || oldTypeIsXM),
                oldTypeIsS3M_IT_MPT = (oldTypeIsS3M || oldTypeIsIT || oldTypeIsMPT),
                oldTypeIsIT_MPT = (oldTypeIsIT || oldTypeIsMPT);

	const bool newTypeIsMOD = (nNewType == MOD_TYPE_MOD), newTypeIsXM =  (nNewType == MOD_TYPE_XM), 
				newTypeIsS3M = (nNewType == MOD_TYPE_S3M), newTypeIsIT = (nNewType == MOD_TYPE_IT),
				newTypeIsMPT = (nNewType == MOD_TYPE_MPT), newTypeIsMOD_XM = (newTypeIsMOD || newTypeIsXM), 
				newTypeIsS3M_IT_MPT = (newTypeIsS3M || newTypeIsIT || newTypeIsMPT), 
				newTypeIsXM_IT_MPT = (newTypeIsXM || newTypeIsIT || newTypeIsMPT),
				newTypeIsIT_MPT = (newTypeIsIT || newTypeIsMPT);

	if(oldTypeIsMPT)
	{
		if(::MessageBox(NULL, str_mptm_conversion_warning, 0, MB_YESNO) != IDYES)
			return FALSE;

		/*
		Incomplete list of MPTm-only features and extensions in the old formats:

		Features only available for MPTm:
		-User definable tunings.
		-Extended pattern range
		-Extended sequence

		Extended features in IT/XM/S3M/MOD(not all listed below are available in all of those formats):
		-plugs
		-Extended ranges for
			-sample count
			-instrument count
			-pattern count
			-sequence size
			-Row count
			-channel count
			-tempo limits
		-Extended sample/instrument properties.
		-MIDI mapping directives
		-Versioninfo
		-channel names
		-pattern names
		-Alternative tempomodes
		-For more info, see e.g. SaveExtendedSongProperties(), SaveExtendedInstrumentProperties()
		*/
		
	}

	// Check if conversion to 64 rows is necessary
	for (UINT ipat=0; ipat<m_SndFile.Patterns.Size(); ipat++)
	{
		if ((m_SndFile.Patterns[ipat]) && (m_SndFile.PatternSize[ipat] != 64)) b64++;
	}
	if (((m_SndFile.m_nInstruments) || (b64)) && (nNewType & (MOD_TYPE_MOD|MOD_TYPE_S3M)))
	{
		if (::MessageBox(NULL,
				"This operation will convert all instruments to samples,\n"
				"and resize all patterns to 64 rows.\n"
				"Do you want to continue?", "Warning", MB_YESNO | MB_ICONQUESTION) != IDYES) return FALSE;
		BeginWaitCursor();
		BEGIN_CRITICAL();
		// Converting instruments to samples
		if (m_SndFile.m_nInstruments)
		{
			ConvertInstrumentsToSamples();
			AddToLog("WARNING: All instruments have been converted to samples.\n");
		}
		// Resizing all patterns to 64 rows
		UINT nPatCvt = 0;
		UINT i = 0;
		for (i=0; i<m_SndFile.Patterns.Size(); i++) if ((m_SndFile.Patterns[i]) && (m_SndFile.PatternSize[i] != 64))
		{
			m_SndFile.Patterns[i].Resize(64);
			if (b64 < 5)
			{
				wsprintf(s, "WARNING: Pattern %d resized to 64 rows\n", i);
				AddToLog(s);
			}
			nPatCvt++;
		}
		if (nPatCvt >= 5)
		{
			wsprintf(s, "WARNING: %d patterns have been resized to 64 rows\n", i);
			AddToLog(s);
		} 
		// Removing all instrument headers
		for (UINT i1=0; i1<MAX_CHANNELS; i1++)
		{
			m_SndFile.Chn[i1].pModInstrument = nullptr;
		}
		for (UINT i2=0; i2<m_SndFile.m_nInstruments; i2++) if (m_SndFile.Instruments[i2])
		{
			delete m_SndFile.Instruments[i2];
			m_SndFile.Instruments[i2] = NULL;
		}
		m_SndFile.m_nInstruments = 0;
		END_CRITICAL();
		EndWaitCursor();
	} //End if (((m_SndFile.m_nInstruments) || (b64)) && (nNewType & (MOD_TYPE_MOD|MOD_TYPE_S3M)))
	BeginWaitCursor();


	/////////////////////////////
	// Converting pattern data

	for (UINT nPat = 0; nPat < m_SndFile.Patterns.Size(); nPat++) if (m_SndFile.Patterns[nPat])
	{
		MODCOMMAND *m = m_SndFile.Patterns[nPat];

		// This is used for -> MOD/XM conversion
		BYTE cEffectMemory[MAX_BASECHANNELS][MAX_EFFECTS];
		memset(&cEffectMemory, 0, sizeof(BYTE) * MAX_BASECHANNELS * MAX_EFFECTS);
		UINT nChannel = m_SndFile.m_nChannels - 1;

		for (UINT len = m_SndFile.PatternSize[nPat] * m_SndFile.m_nChannels; len; m++, len--)
		{
			nChannel = (nChannel + 1) % m_SndFile.m_nChannels; // 0...Channels - 1

			//////////////////////////
			// Convert 8-bit Panning
			if(m->command == CMD_PANNING8)
			{
				if(newTypeIsS3M)
				{
					m->param = (m->param + 1) >> 1;
				}
				else if(oldTypeIsS3M)
				{
					if(m->param == 0xA4)
					{
						// surround remap
						m->command = (newTypeIsIT_MPT) ? CMD_S3MCMDEX : CMD_XFINEPORTAUPDOWN;
						m->param = 0x91;
					}
					else
					{
						m->param = min(m->param << 1, 0xFF);
					}
				}
			} // End if(m->command == CMD_PANNING8)

			//////////////////////////
			// Convert param control
			if(oldTypeIsMPT)
			{
				if(m->note == NOTE_PC || m->note == NOTE_PCS)
				{
					m->param = min(MODCOMMAND::maxColumnValue, m->GetValueEffectCol()) * 0x7F / MODCOMMAND::maxColumnValue;
					m->command = (m->note == NOTE_PC) ? CMD_MIDI : CMD_SMOOTHMIDI;
					m->volcmd = VOLCMD_NONE;
					m->note = NOTE_NONE;
				}
			} // End if(oldTypeIsMPT)

			/////////////////////////////////////////
			// Convert MOD / XM to S3M / IT / MPTM
			if(oldTypeIsMOD_XM && newTypeIsS3M_IT_MPT)
			{
				switch(m->command)
				{
				case CMD_MODCMDEX:
					m->command = CMD_S3MCMDEX;
					switch(m->param & 0xF0)
					{
					case 0x10:	m->command = CMD_PORTAMENTOUP; m->param |= 0xF0; break;
					case 0x20:	m->command = CMD_PORTAMENTODOWN; m->param |= 0xF0; break;
					case 0x30:	m->param = (m->param & 0x0F) | 0x10; break;
					case 0x40:	m->param = (m->param & 0x0F) | 0x30; break;
					case 0x50:	m->param = (m->param & 0x0F) | 0x20; break;
					case 0x60:	m->param = (m->param & 0x0F) | 0xB0; break;
					case 0x70:	m->param = (m->param & 0x0F) | 0x40; break;
					case 0x90:	m->command = CMD_RETRIG; m->param = 0x80 | (m->param & 0x0F); break;
					case 0xA0:	if (m->param & 0x0F) { m->command = CMD_VOLUMESLIDE; m->param = (m->param << 4) | 0x0F; } else m->command = 0; break;
					case 0xB0:	if (m->param & 0x0F) { m->command = CMD_VOLUMESLIDE; m->param |= 0xF0; } else m->command = 0; break;
					}
					break;
				case CMD_VOLUME:
					if (!m->volcmd)
					{
						m->volcmd = VOLCMD_VOLUME;
						m->vol = m->param;
						if (m->vol > 0x40) m->vol = 0x40;
						m->command = m->param = 0;
					}
					break;
				case CMD_PORTAMENTOUP:
					if (m->param > 0xDF) m->param = 0xDF;
					break;
				case CMD_PORTAMENTODOWN:
					if (m->param > 0xDF) m->param = 0xDF;
					break;
				case CMD_XFINEPORTAUPDOWN:
					switch(m->param & 0xF0)
					{
					case 0x10:	m->command = CMD_PORTAMENTOUP; m->param = (m->param & 0x0F) | 0xE0; break;
					case 0x20:	m->command = CMD_PORTAMENTODOWN; m->param = (m->param & 0x0F) | 0xE0; break;
					case 0x50:
					case 0x60:
					case 0x70:
					case 0x90:
					case 0xA0:
						m->command = CMD_S3MCMDEX;
						// surround remap (this is the "official" command)
						if(newTypeIsS3M && m->param == 0x91)
						{
							m->command = CMD_PANNING8;
							m->param = 0xA4;
						}
						break;
					}
					break;
				case CMD_KEYOFF:
					if(m->note == 0)
					{
						m->note = (newTypeIsS3M) ? NOTE_NOTECUT : NOTE_KEYOFF;
						m->command = CMD_S3MCMDEX;
						if(m->param == 0)
							m->instr = 0;
						m->param = 0xD0 | (m->param & 0x0F);
					}
					break;
				case CMD_PANNINGSLIDE:
					// swap L/R
					m->param = ((m->param & 0x0F) << 4) | (m->param >> 4);
				default:
					break;
				}
			} // End if(oldTypeIsMOD_XM && newTypeIsS3M_IT_MPT)


			/////////////////////////////////////////
			// Convert S3M / IT / MPTM to MOD / XM
			else if (oldTypeIsS3M_IT_MPT && newTypeIsMOD_XM)
			{
				if(m->note == NOTE_NOTECUT || m->note == NOTE_FADE)
					m->note = NOTE_KEYOFF;
				
				switch(m->command)
				{
				case CMD_ARPEGGIO:
					// No effect memory in XM / MOD
					if(m->param == 0)
						m->param = cEffectMemory[nChannel][CMD_ARPEGGIO];
					else
						cEffectMemory[nChannel][CMD_ARPEGGIO] = m->param;
					break;
				case CMD_S3MCMDEX:
					m->command = CMD_MODCMDEX;
					switch(m->param & 0xF0)
					{
					case 0x10:	m->param = (m->param & 0x0F) | 0x30; break;
					case 0x20:	m->param = (m->param & 0x0F) | 0x50; break;
					case 0x30:	m->param = (m->param & 0x0F) | 0x40; break;
					case 0x40:	m->param = (m->param & 0x0F) | 0x70; break;
					case 0x50:	
					case 0x60:	
					case 0x70:
					case 0x90:
					case 0xA0:	m->command = CMD_XFINEPORTAUPDOWN; break;
					case 0xB0:	m->param = (m->param & 0x0F) | 0x60; break;
					}
					break;
				case CMD_VOLUMESLIDE:
					if ((m->param & 0xF0) && ((m->param & 0x0F) == 0x0F))
					{
						m->command = CMD_MODCMDEX;
						m->param = (m->param >> 4) | 0xA0;
					} else
					if ((m->param & 0x0F) && ((m->param & 0xF0) == 0xF0))
					{
						m->command = CMD_MODCMDEX;
						m->param = (m->param & 0x0F) | 0xB0;
					}
					break;
				case CMD_PORTAMENTOUP:
					if (m->param >= 0xF0)
					{
						m->command = CMD_MODCMDEX;
						m->param = (m->param & 0x0F) | 0x10;
					} else
					if (m->param >= 0xE0)
					{
						m->command = CMD_MODCMDEX;
						m->param = (((m->param & 0x0F)+3) >> 2) | 0x10;
					} else m->command = CMD_PORTAMENTOUP;
					break;
				case CMD_PORTAMENTODOWN:
					if (m->param >= 0xF0)
					{
						m->command = CMD_MODCMDEX;
						m->param = (m->param & 0x0F) | 0x20;
					} else
					if (m->param >= 0xE0)
					{
						m->command = CMD_MODCMDEX;
						m->param = (((m->param & 0x0F)+3) >> 2) | 0x20;
					} else m->command = CMD_PORTAMENTODOWN;
					break;
				case CMD_SPEED:
					{
						UINT spdmax = (nNewType == MOD_TYPE_XM) ? 0x1F : 0x20;
						if (m->param > spdmax) m->param = spdmax;
					}
					break;
				case CMD_PANNINGSLIDE:
					// swap L/R
					m->param = ((m->param & 0x0F) << 4) | (m->param >> 4);
					// remove fine slides
					if((m->param > 0xF0) || ((m->param & 0x0F) == 0x0F && m->param != 0x0F))
						m->command = CMD_NONE;
				default:
					break;
				}
			} // End if (oldTypeIsS3M_IT_MPT && newTypeIsMOD_XM)


			///////////////////////
			// Convert IT to S3M
			else if (oldTypeIsIT_MPT && newTypeIsS3M)
			{
				if(m->note == NOTE_KEYOFF || m->note == NOTE_FADE)
					m->note = NOTE_NOTECUT;

				switch(m->command)
				{
				case CMD_S3MCMDEX:
					if(m->param == 0x91)
					{
						// surround remap (this is the "official" command)
						m->command = CMD_PANNING8;
						m->param = 0xA4;
					}
					break;
				case CMD_SMOOTHMIDI:
					m->command = CMD_MIDI;
					break;
				default:
					break;
				}
			} // End if (oldTypeIsIT_MPT && newTypeIsS3M)



			///////////////////////////////////////////////////
			// Convert MOD to anything - adjust effect memory
			if (oldTypeIsMOD)
			{
				switch(m->command)
				{
				case CMD_TONEPORTAVOL: // lacks memory -> 500 is the same as 300
					if(m->param == 0x00) m->command = CMD_TONEPORTAMENTO;
					break;
				case CMD_VIBRATOVOL: // lacks memory -> 600 is the same as 400
					if(m->param == 0x00) m->command = CMD_VIBRATO;
					break;
				}
			} // End if (oldTypeIsMOD && newTypeIsXM)

			/////////////////////////////////////////////////////////////////////////////////
			// Convert anything to MOD - remove volume column, adjust retrig, effect memory
			if (newTypeIsMOD)
			{
				if(m->command) switch(m->command)
				{
				case CMD_RETRIG:
					m->command = CMD_MODCMDEX;
					m->param = 0x90 | (m->param & 0x0F);
					break;
				case CMD_PORTAMENTOUP:
				case CMD_PORTAMENTODOWN:
				case CMD_TONEPORTAVOL:
				case CMD_VIBRATOVOL:
				case CMD_VOLUMESLIDE:
					// ProTracker doesn't have effect memory for these commands, so let's try to fix them
					if(m->param == 0)
						m->param = cEffectMemory[nChannel][m->command];
					else
						cEffectMemory[nChannel][m->command] = m->param;
					break;				
				}

				else switch(m->volcmd)
				{
				case VOLCMD_VOLUME:
					m->command = CMD_VOLUME;
					m->param = m->vol;
					break;
				case VOLCMD_PANNING:
					m->command = CMD_PANNING8;
					m->param = CLAMP(m->vol << 2, 0, 0xFF);
					break;
				case VOLCMD_VOLSLIDEDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol;
					break;
				case VOLCMD_VOLSLIDEUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol << 4;
					break;
				case VOLCMD_FINEVOLDOWN:
					m->command = CMD_MODCMDEX;
					m->param = 0xB0 | m->vol;
					break;
				case VOLCMD_FINEVOLUP:
					m->command = CMD_MODCMDEX;
					m->param = 0xA0 | m->vol;
					break;
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					break;
				case VOLCMD_TONEPORTAMENTO:
					m->command = CMD_TONEPORTAMENTO;
					m->param = m->vol << 2;
					break;
				case VOLCMD_VIBRATODEPTH:
					m->command = CMD_VIBRATO;
					m->param = m->vol;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					break;
				// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					break;
				case VOLCMD_VELOCITY:
					m->command = CMD_VOLUME;
					m->param = m->vol * 7;
					break;
				default:
					break;
				}
				m->volcmd = CMD_NONE;
			} // End if (newTypeIsMOD)

			///////////////////////////////////////////////////
			// Convert anything to S3M - adjust volume column
			if (newTypeIsS3M)
			{
				if(!m->command) switch(m->volcmd)
				{
				case VOLCMD_VOLSLIDEDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VOLSLIDEUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_FINEVOLDOWN:
					m->command = CMD_VOLUMESLIDE;
					m->param = 0xF0 | m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_FINEVOLUP:
					m->command = CMD_VOLUMESLIDE;
					m->param = (m->vol << 4) | 0x0F;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_TONEPORTAMENTO:
					m->command = CMD_TONEPORTAMENTO;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATODEPTH:
					m->command = CMD_VIBRATO;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDELEFT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDERIGHT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VELOCITY:
					m->volcmd = CMD_VOLUME;
					m->vol *= 7;
					break;
				default:
					break;
				}
			} // End if (newTypeIsS3M)

			//////////////////////////////////////////////////
			// Convert anything to XM - adjust volume column
			if (newTypeIsXM)
			{
				if(!m->command) switch(m->volcmd)
				{
				case VOLCMD_PORTADOWN:
					m->command = CMD_PORTAMENTODOWN;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PORTAUP:
					m->command = CMD_PORTAMENTOUP;
					m->param = m->vol << 2;
					m->volcmd = CMD_NONE;
					break;
				// OpenMPT-specific commands
				case VOLCMD_OFFSET:
					m->command = CMD_OFFSET;
					m->param = m->vol << 3;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VELOCITY:
					m->volcmd = CMD_VOLUME;
					m->vol *= 7;
					break;
				default:
					break;
				}
			} // End if (newTypeIsXM)

			///////////////////////////////////////////////////
			// Convert anything to IT - adjust volume column
			if (newTypeIsIT_MPT)
			{
				if(!m->command) switch(m->volcmd)
				{
				case VOLCMD_VOLSLIDEDOWN:
				case VOLCMD_VOLSLIDEUP:
				case VOLCMD_FINEVOLDOWN:
				case VOLCMD_FINEVOLUP:
				case VOLCMD_PORTADOWN:
				case VOLCMD_PORTAUP:
				case VOLCMD_TONEPORTAMENTO:
				case VOLCMD_VIBRATODEPTH:
				// OpenMPT-specific commands
				case VOLCMD_OFFSET:
				case VOLCMD_VELOCITY:
					m->vol = min(m->vol, 9);
					break;
				case VOLCMD_PANSLIDELEFT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_PANSLIDERIGHT:
					m->command = CMD_PANNINGSLIDE;
					m->param = m->vol;
					m->volcmd = CMD_NONE;
					break;
				case VOLCMD_VIBRATOSPEED:
					m->command = CMD_VIBRATO;
					m->param = m->vol << 4;
					m->volcmd = CMD_NONE;
					break;
				default:
					break;
				}
			} // End if (newTypeIsIT)

			if(!m_SndFile.GetModSpecifications().HasNote(m->note))
				m->note = NOTE_NONE;
		}
	}

	////////////////////////////////////////////////
	// Converting instrument / sample / etc. data


	// Convert MOD/XM to S3M/IT/MPT
	if (oldTypeIsMOD_XM && newTypeIsS3M_IT_MPT)
	{
		for (SAMPLEINDEX i=1; i<=m_SndFile.m_nSamples; i++)
		{
			m_SndFile.Samples[i].nC5Speed = CSoundFile::TransposeToFrequency(m_SndFile.Samples[i].RelativeTone, m_SndFile.Samples[i].nFineTune);
			m_SndFile.Samples[i].RelativeTone = 0;
			m_SndFile.Samples[i].nFineTune = 0;
		}
		if (oldTypeIsXM && newTypeIsIT_MPT) m_SndFile.m_dwSongFlags |= SONG_ITCOMPATMODE;
	} else

	// Convert S3M/IT/MPT to XM
	if (oldTypeIsS3M_IT_MPT && newTypeIsXM)
	{
		for (SAMPLEINDEX i=1; i<=m_SndFile.m_nSamples; i++)
		{
			CSoundFile::FrequencyToTranspose(&m_SndFile.Samples[i]);
			if (!(m_SndFile.Samples[i].uFlags & CHN_PANNING)) m_SndFile.Samples[i].nPan = 128;
		}
		bool bBrokenNoteMap = false, bBrokenSustainLoop = false;
		for (INSTRUMENTINDEX j = 1; j <= m_SndFile.m_nInstruments; j++)
		{
			MODINSTRUMENT *pIns = m_SndFile.Instruments[j];
			if (pIns)
			{
				for (UINT k = 0; k < NOTE_MAX; k++)
				{
					if ((pIns->NoteMap[k]) && (pIns->NoteMap[k] != (BYTE)(k+1)))
					{
						bBrokenNoteMap = true;
						break;
					}
				}
				// Convert sustain loops to sustain "points"
				if(pIns->VolEnv.nSustainStart != pIns->VolEnv.nSustainEnd)
				{
					pIns->VolEnv.nSustainEnd = pIns->VolEnv.nSustainStart;
					bBrokenSustainLoop = true;
				}
				if(pIns->PanEnv.nSustainStart != pIns->PanEnv.nSustainEnd)
				{
					pIns->PanEnv.nSustainEnd = pIns->PanEnv.nSustainStart;
					bBrokenSustainLoop = true;
				}
				pIns->dwFlags &= ~(ENV_SETPANNING|ENV_VOLCARRY|ENV_PANCARRY|ENV_PITCHCARRY|ENV_FILTER|ENV_PITCH);
				pIns->nIFC &= 0x7F;
				pIns->nIFR &= 0x7F;
			}
		}
		if (bBrokenNoteMap) AddToLog("WARNING: Note Mapping will be lost when saving as XM.\n");
		if (bBrokenSustainLoop) AddToLog("WARNING: Sustain loops were converted to sustain points.\n");
	}

	if(newTypeIsMOD)
	{
		// Not supported in MOD format
		m_SndFile.m_nDefaultSpeed = 6;
		m_SndFile.m_nDefaultTempo = 125;
		m_SndFile.m_nDefaultGlobalVolume = 256;
		m_SndFile.m_nSamplePreAmp = 48;
		m_SndFile.m_nVSTiVolume = 48;
		AddToLog("WARNING: Default speed, tempo and global volume will be lost.\n");
	}

	// Too many samples?
	if (newTypeIsMOD && (m_SndFile.m_nSamples > 31))
	{
		AddToLog("WARNING: Samples above 31 will be lost when saving as MOD!\n");
	}
	BEGIN_CRITICAL();
	m_SndFile.ChangeModTypeTo(nNewType);
	if (!newTypeIsXM_IT_MPT && (m_SndFile.m_dwSongFlags & SONG_LINEARSLIDES))
	{
		AddToLog("WARNING: Linear Frequency Slides not supported by the new format.\n");
		m_SndFile.m_dwSongFlags &= ~SONG_LINEARSLIDES;
	}
	if (!newTypeIsIT_MPT) m_SndFile.m_dwSongFlags &= ~(SONG_ITOLDEFFECTS|SONG_ITCOMPATMODE);
	if (!newTypeIsS3M) m_SndFile.m_dwSongFlags &= ~SONG_FASTVOLSLIDES;
	END_CRITICAL();
	ChangeFileExtension(nNewType);

	//rewbs.cutomKeys: update effect key commands
	CInputHandler *ih = CMainFrame::GetMainFrame()->GetInputHandler();
	if	(newTypeIsMOD_XM) {
		ih->SetXMEffects();
	} else {
		ih->SetITEffects();
	}
	//end rewbs.cutomKeys

	// Check mod specifications
	const CModSpecifications& specs = m_SndFile.GetModSpecifications();
	m_SndFile.m_nDefaultTempo = CLAMP(m_SndFile.m_nDefaultTempo, specs.tempoMin, specs.tempoMax);
	m_SndFile.m_nDefaultSpeed = CLAMP(m_SndFile.m_nDefaultSpeed, specs.speedMin, specs.speedMax);

	bool bTrimmedEnvelopes = false;
	for(INSTRUMENTINDEX i = 1; i <= m_SndFile.m_nInstruments; i++)
	{
		bTrimmedEnvelopes |= UpdateEnvelopes(&m_SndFile.Instruments[i]->VolEnv);
		bTrimmedEnvelopes |= UpdateEnvelopes(&m_SndFile.Instruments[i]->PanEnv);
		bTrimmedEnvelopes |= UpdateEnvelopes(&m_SndFile.Instruments[i]->PitchEnv);
	}
	if(bTrimmedEnvelopes == true)
		AddToLog("WARNING: Instrument envelopes have been shortened.\n");

	SetModified();
	ClearUndo();
	UpdateAllViews(NULL, HINT_MODTYPE | HINT_MODGENERAL);
	EndWaitCursor();
	return TRUE;
}

bool CModDoc::UpdateEnvelopes(INSTRUMENTENVELOPE *mptEnv)
//-------------------------------------------------------
{
	// shorten instrument envelope if necessary (for mod conversion)
	const UINT iEnvMax = m_SndFile.GetModSpecifications().envelopePointsMax;
	bool bResult = false;

	#define TRIMENV(i) if(i > iEnvMax) {i = iEnvMax; bResult = true;}

	TRIMENV(mptEnv->nNodes);
	TRIMENV(mptEnv->nLoopStart);
	TRIMENV(mptEnv->nLoopEnd);
	TRIMENV(mptEnv->nSustainStart);
	TRIMENV(mptEnv->nSustainEnd);
	if(mptEnv->nReleaseNode != ENV_RELEASE_NODE_UNSET) TRIMENV(mptEnv->nReleaseNode);

	#undef TRIMENV

	return bResult;
}






// Change the number of channels
BOOL CModDoc::ChangeNumChannels(UINT nNewChannels, const bool showCancelInRemoveDlg)
//------------------------------------------------
{
	const CHANNELINDEX maxChans = m_SndFile.GetModSpecifications().channelsMax;

	if (nNewChannels > maxChans) {
		CString error;
		error.Format("Error: Max number of channels for this type is %d", maxChans);
		::AfxMessageBox(error, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (nNewChannels == m_SndFile.m_nChannels) return FALSE;
	if (nNewChannels < m_SndFile.m_nChannels)
	{
		UINT nChnToRemove = 0;
		CHANNELINDEX nFound = 0;

		//nNewChannels = 0 means user can choose how many channels to remove
		if(nNewChannels > 0) {
			nChnToRemove = m_SndFile.m_nChannels - nNewChannels;
			nFound = nChnToRemove;
		} else {
			nChnToRemove = 0;
			nFound = m_SndFile.m_nChannels;
		}
		
		CRemoveChannelsDlg rem(&m_SndFile, nChnToRemove, showCancelInRemoveDlg);
		CheckUnusedChannels(rem.m_bChnMask, nFound);
		if (rem.DoModal() != IDOK) return FALSE;

		// Removing selected channels
		RemoveChannels(rem.m_bChnMask);
	} else
	{
		BeginWaitCursor();
		// Increasing number of channels
		BEGIN_CRITICAL();
		for (UINT i=0; i<m_SndFile.Patterns.Size(); i++) if (m_SndFile.Patterns[i])
		{
			MODCOMMAND *p = m_SndFile.Patterns[i];
			MODCOMMAND *newp = CSoundFile::AllocatePattern(m_SndFile.PatternSize[i], nNewChannels);
			if (!newp)
			{
				END_CRITICAL();
				AddToLog("ERROR: Not enough memory to create new channels!\nPattern Data is corrupted!\n");
				return FALSE;
			}
			for (UINT j=0; j<m_SndFile.PatternSize[i]; j++)
			{
				memcpy(&newp[j*nNewChannels], &p[j*m_SndFile.m_nChannels], m_SndFile.m_nChannels*sizeof(MODCOMMAND));
			}
			m_SndFile.Patterns[i] = newp;
			CSoundFile::FreePattern(p);
		}

		//if channel was removed before and is added again, mute status has to be unset! (bug 1814)
		for (UINT i=m_SndFile.m_nChannels; i<nNewChannels; i++)
		{
			m_SndFile.InitChannel(i);
		}
	
		m_SndFile.m_nChannels = nNewChannels;
		END_CRITICAL();
		EndWaitCursor();
	}
	SetModified();
	ClearUndo();
	UpdateAllViews(NULL, HINT_MODTYPE);
	return TRUE;
}


BOOL CModDoc::RemoveChannels(BOOL m_bChnMask[MAX_CHANNELS])
//--------------------------------------------------------
//To remove all channels whose index corresponds to true value at m_bChnMask[] array. Code is almost non-modified copy of
//the code which was in CModDoc::ChangeNumChannels(UINT nNewChannels) - the only differences are the lines before 
//BeginWaitCursor(), few lines in the end and that nNewChannels is renamed to nRemaningChannels.
{
		UINT nRemainingChannels = 0;
		//First calculating how many channels are to be left
		UINT i = 0;
		for(i = 0; i<m_SndFile.m_nChannels; i++)
		{
			if(!m_bChnMask[i]) nRemainingChannels++;
		}
		if(nRemainingChannels == m_SndFile.m_nChannels || nRemainingChannels < m_SndFile.GetModSpecifications().channelsMin)
		{
			CString str;	
			if(nRemainingChannels == m_SndFile.m_nChannels) str.Format("No channels chosen to be removed.");
			else str.Format("No removal done - channel number is already at minimum.");
			CMainFrame::GetMainFrame()->MessageBox(str , "Remove channel", MB_OK | MB_ICONINFORMATION);
			return FALSE;
		}

		BeginWaitCursor();
		BEGIN_CRITICAL();
		for (i=0; i<m_SndFile.Patterns.Size(); i++) if (m_SndFile.Patterns[i])
		{
			MODCOMMAND *p = m_SndFile.Patterns[i];
			MODCOMMAND *newp = CSoundFile::AllocatePattern(m_SndFile.PatternSize[i], nRemainingChannels);
			if (!newp)
			{
				END_CRITICAL();
				AddToLog("ERROR: Not enough memory to resize patterns!\nPattern Data is corrupted!");
				return TRUE;
			}
			MODCOMMAND *tmpsrc = p, *tmpdest = newp;
			for (UINT j=0; j<m_SndFile.PatternSize[i]; j++)
			{
				for (UINT k=0; k<m_SndFile.m_nChannels; k++, tmpsrc++)
				{
					if (!m_bChnMask[k]) *tmpdest++ = *tmpsrc;
				}
			}
			m_SndFile.Patterns[i] = newp;
			CSoundFile::FreePattern(p);
		}
		UINT tmpchn = 0;
		for (i=0; i<m_SndFile.m_nChannels; i++)
		{
			if (!m_bChnMask[i])
			{
				if (tmpchn != i)
				{
					m_SndFile.ChnSettings[tmpchn] = m_SndFile.ChnSettings[i];
					m_SndFile.Chn[tmpchn] = m_SndFile.Chn[i];
				}
				tmpchn++;
				if (i >= nRemainingChannels)
				{
					m_SndFile.Chn[i].dwFlags |= CHN_MUTE;
					m_SndFile.InitChannel(i);
				}
			}
		}
		m_SndFile.m_nChannels = nRemainingChannels;
		END_CRITICAL();
		EndWaitCursor();
		SetModified();
		ClearUndo();
		UpdateAllViews(NULL, HINT_MODTYPE);
		return FALSE;
}


BOOL CModDoc::RemoveUnusedPatterns(BOOL bRemove)
//----------------------------------------------
{
	if (GetSoundFile()->GetType() == MOD_TYPE_MPT && GetSoundFile()->Order.GetNumSequences() > 1)
	{   // Multiple sequences are not taken into account in the code below. For now just make
		// removing unused patterns disabled in this case.
		AfxMessageBox(IDS_PATTERN_CLEANUP_UNAVAILABLE, MB_ICONINFORMATION);
		return FALSE;
	}
	const UINT maxPatIndex = m_SndFile.Patterns.Size();
	const UINT maxOrdIndex = m_SndFile.Order.size();
	vector<UINT> nPatMap(maxPatIndex, 0);
	vector<UINT> nPatRows(maxPatIndex, 0);
	vector<MODCOMMAND*> pPatterns(maxPatIndex, NULL);
	vector<BOOL> bPatUsed(maxPatIndex, false);

	const ORDERINDEX nLengthSub0 = m_SndFile.Order.GetLengthFirstEmpty();
	const ORDERINDEX nLengthUsed = m_SndFile.Order.GetLengthTailTrimmed();

	// Flag to tell whether keeping sequence items which are after the first empty('---') order.
	bool bKeepSubSequences = false;

	if(nLengthUsed != nLengthSub0)
	{   // There are used sequence items after first '---'; ask user whether to remove those.
		if (CMainFrame::GetMainFrame()->MessageBox(
			_TEXT("Do you want to remove sequence items which are after the first '---' item?"),
			_TEXT("Sequence Cleanup"), MB_YESNO) != IDYES
			)
			bKeepSubSequences = true;
	}
	
	CHAR s[512];
	BOOL bEnd = FALSE, bReordered = FALSE;
	UINT nPatRemoved = 0, nMinToRemove, nPats;
	
	BeginWaitCursor();
	UINT maxpat = 0;
	for (UINT iord=0; iord<maxOrdIndex; iord++)
	{
		UINT n = m_SndFile.Order[iord];
		if (n < maxPatIndex)
		{
			if (n >= maxpat) maxpat = n+1;
			if (!bEnd || bKeepSubSequences) bPatUsed[n] = TRUE;
		} else if (n == m_SndFile.Order.GetInvalidPatIndex()) bEnd = TRUE;
	}
	nMinToRemove = 0;
	if (!bRemove)
	{
		UINT imax = maxPatIndex;
		while (imax > 0)
		{
			imax--;
			if ((m_SndFile.Patterns[imax]) && (bPatUsed[imax])) break;
		}
		nMinToRemove = imax+1;
	}
	for (UINT ipat=maxpat; ipat<maxPatIndex; ipat++) if ((m_SndFile.Patterns[ipat]) && (ipat >= nMinToRemove))
	{
		MODCOMMAND *m = m_SndFile.Patterns[ipat];
		UINT ncmd = m_SndFile.m_nChannels * m_SndFile.PatternSize[ipat];
		for (UINT i=0; i<ncmd; i++)
		{
			if ((m[i].note) || (m[i].instr) || (m[i].volcmd) || (m[i].command)) goto NotEmpty;
		}
		m_SndFile.Patterns.Remove(ipat);
		nPatRemoved++;
	NotEmpty:
		;
	}
	UINT bWaste = 0;
	for (UINT ichk=0; ichk < maxPatIndex; ichk++)
	{
		if ((m_SndFile.Patterns[ichk]) && (!bPatUsed[ichk])) bWaste++;
	}
	if ((bRemove) && (bWaste))
	{
		EndWaitCursor();
		wsprintf(s, "%d pattern(s) present in file, but not used in the song\nDo you want to reorder the sequence list and remove these patterns?", bWaste);
		if (CMainFrame::GetMainFrame()->MessageBox(s, "Pattern Cleanup", MB_YESNO) != IDYES) return TRUE;
		BeginWaitCursor();
	}
	for (UINT irst=0; irst<maxPatIndex; irst++) nPatMap[irst] = 0xFFFF;
	nPats = 0;
	UINT imap = 0;
	for (imap=0; imap<maxOrdIndex; imap++)
	{
		UINT n = m_SndFile.Order[imap];
		if (n < maxPatIndex)
		{
			if (nPatMap[n] > maxPatIndex) nPatMap[n] = nPats++;
			m_SndFile.Order[imap] = nPatMap[n];
		} else if (n == m_SndFile.Order.GetInvalidPatIndex() && (bKeepSubSequences == false)) break;
	}
	// Add unused patterns at the end
	if ((!bRemove) || (!bWaste))
	{
		for (UINT iadd=0; iadd<maxPatIndex; iadd++)
		{
			if ((m_SndFile.Patterns[iadd]) && (nPatMap[iadd] >= maxPatIndex))
			{
				nPatMap[iadd] = nPats++;
			}
		}
	}
	while (imap < maxOrdIndex)
	{
		m_SndFile.Order[imap++] = m_SndFile.Order.GetInvalidPatIndex();
	}
	BEGIN_CRITICAL();
	// Reorder patterns & Delete unused patterns
	{
		UINT npatnames = m_SndFile.m_nPatternNames;
		LPSTR lpszpatnames = m_SndFile.m_lpszPatternNames;
		m_SndFile.m_nPatternNames = 0;
		m_SndFile.m_lpszPatternNames = NULL;
		for (UINT i=0; i<maxPatIndex; i++)
		{
			UINT k = nPatMap[i];
			if (k < maxPatIndex)
			{
				if (i != k) bReordered = TRUE;
				// Remap pattern names
				if (i < npatnames)
				{
					UINT noldpatnames = m_SndFile.m_nPatternNames;
					LPSTR lpszoldpatnames = m_SndFile.m_lpszPatternNames;
					m_SndFile.m_nPatternNames = npatnames;
					m_SndFile.m_lpszPatternNames = lpszpatnames;
					m_SndFile.GetPatternName(i, s);
					m_SndFile.m_nPatternNames = noldpatnames;
					m_SndFile.m_lpszPatternNames = lpszoldpatnames;
					if (s[0]) m_SndFile.SetPatternName(k, s);
				}
				nPatRows[k] = m_SndFile.PatternSize[i];
				pPatterns[k] = m_SndFile.Patterns[i];
			} else
			if (m_SndFile.Patterns[i])
			{
				m_SndFile.Patterns.Remove(i);
				nPatRemoved++;
			}
		}
		for (UINT j=0; j<maxPatIndex;j++)
		{
			m_SndFile.Patterns[j].SetData(pPatterns[j], nPatRows[j]);
		}
	}
	END_CRITICAL();
	EndWaitCursor();
	if ((nPatRemoved) || (bReordered))
	{
		ClearUndo();
		SetModified();
		if (nPatRemoved)
		{
			wsprintf(s, "%d pattern(s) removed.\n", nPatRemoved);
			AddToLog(s);
		}
		return TRUE;
	}
	return FALSE;
}




void CModDoc::RearrangeSamples()
//------------------------------
{
	if(m_SndFile.m_nSamples < 2)
		return;

	UINT nRemap = 0; // remap count
	UINT nSampleMap[MAX_SAMPLES + 1]; // map old => new
	for(UINT i = 0; i <= MAX_SAMPLES; i++)
		nSampleMap[i] = i;

	// First, find out which sample slots are unused and create the new sample map
	for(SAMPLEINDEX i = 1 ; i <= m_SndFile.m_nSamples; i++) {
		if(!m_SndFile.Samples[i].pSample)
		{
			// Move all following samples
			nRemap++;
			nSampleMap[i] = 0;
			for(UINT j = i + 1; j <= m_SndFile.m_nSamples; j++)
				nSampleMap[j]--;
		}
	}

	if(!nRemap)
		return;

	BEGIN_CRITICAL();

	// Now, move everything around
	for(SAMPLEINDEX i = 1; i <= m_SndFile.m_nSamples; i++)
	{
		if(nSampleMap[i] != i)
		{
			// This gotta be moved
			m_SndFile.MoveSample(i, nSampleMap[i]);
			m_SndFile.Samples[i].pSample = nullptr;
			if(nSampleMap[i] > 0) strcpy(m_SndFile.m_szNames[nSampleMap[i]], m_SndFile.m_szNames[i]);
			memset(m_SndFile.m_szNames[i], 0, sizeof(m_SndFile.m_szNames[i]));

			// Also update instrument mapping (if module is in instrument mode)
			for(INSTRUMENTINDEX iInstr = 1; iInstr <= m_SndFile.m_nInstruments; iInstr++){
				if(m_SndFile.Instruments[iInstr]){
					MODINSTRUMENT *p = m_SndFile.Instruments[iInstr];
					for(WORD iNote = 0; iNote < 128; iNote++)
						if(p->Keyboard[iNote] == i) p->Keyboard[iNote] = nSampleMap[i];
				}
			}
		}
	}

	// Go through the patterns and remap samples (if module is in sample mode)
	if(!m_SndFile.m_nInstruments)
	{
		for (PATTERNINDEX nPat = 0; nPat < m_SndFile.Patterns.Size(); nPat++) if (m_SndFile.Patterns[nPat])
		{
			MODCOMMAND *m = m_SndFile.Patterns[nPat];
			for(UINT len = m_SndFile.PatternSize[nPat] * m_SndFile.m_nChannels; len; m++, len--)
			{
				if(m->instr <= m_SndFile.m_nSamples) m->instr = nSampleMap[m->instr];
			}
		}
	}

	m_SndFile.m_nSamples -= nRemap;

	END_CRITICAL();

	SetModified();
	UpdateAllViews(NULL, HINT_MODTYPE);

}


BOOL CModDoc::ConvertInstrumentsToSamples()
//-----------------------------------------
{
	if (!m_SndFile.m_nInstruments) return FALSE;
	for (UINT i=0; i<m_SndFile.Patterns.Size(); i++) if (m_SndFile.Patterns[i])
	{
		MODCOMMAND *p = m_SndFile.Patterns[i];
		for (UINT j=m_SndFile.m_nChannels*m_SndFile.PatternSize[i]; j; j--, p++) if (p->instr)
		{
			UINT instr = p->instr;
			UINT note = p->note;
			UINT newins = 0;
			if ((note) && (note < 128)) note--; else note = 5*12;
			if ((instr < MAX_INSTRUMENTS) && (m_SndFile.Instruments[instr]))
			{
				MODINSTRUMENT *pIns = m_SndFile.Instruments[instr];
				newins = pIns->Keyboard[note];
				if (newins >= MAX_SAMPLES) newins = 0;
			}
			p->instr = newins;
		}
	}
	return TRUE;
}


BOOL CModDoc::RemoveUnusedSamples()
//---------------------------------
{
	CHAR s[512];
	BOOL bIns[MAX_SAMPLES];
	UINT nExt = 0, nLoopOpt = 0;
	UINT nRemoved = 0;
	

	BeginWaitCursor();
	for (UINT i=m_SndFile.m_nSamples; i>=1; i--) if (m_SndFile.Samples[i].pSample)
	{
		if (!m_SndFile.IsSampleUsed(i))
		{
			BEGIN_CRITICAL();
			m_SndFile.DestroySample(i);
			if ((i == m_SndFile.m_nSamples) && (i > 1)) m_SndFile.m_nSamples--;
			END_CRITICAL();
			nRemoved++;
		}
	}
	if (m_SndFile.m_nInstruments)
	{
		memset(bIns, 0, sizeof(bIns));
		for (UINT ipat=0; ipat<m_SndFile.Patterns.Size(); ipat++)
		{
			MODCOMMAND *p = m_SndFile.Patterns[ipat];
			if (p)
			{
				UINT jmax = m_SndFile.PatternSize[ipat] * m_SndFile.m_nChannels;
				for (UINT j=0; j<jmax; j++, p++)
				{
					if ((p->note) && (p->note <= NOTE_MAX))
					{
						if ((p->instr) && (p->instr < MAX_INSTRUMENTS))
						{
							MODINSTRUMENT *pIns = m_SndFile.Instruments[p->instr];
							if (pIns)
							{
								UINT n = pIns->Keyboard[p->note-1];
								if (n < MAX_SAMPLES) bIns[n] = TRUE;
							}
						} else
						{
							for (UINT k=1; k<=m_SndFile.m_nInstruments; k++)
							{
								MODINSTRUMENT *pIns = m_SndFile.Instruments[k];
								if (pIns)
								{
									UINT n = pIns->Keyboard[p->note-1];
									if (n < MAX_SAMPLES) bIns[n] = TRUE;
								}
							}
						}
					}
				}
			}
		}
		for (UINT ichk=1; ichk<MAX_SAMPLES; ichk++)
		{
			if ((!bIns[ichk]) && (m_SndFile.Samples[ichk].pSample)) nExt++;
		}
	}
	EndWaitCursor();
	if (nExt &&  !((m_SndFile.m_nType & MOD_TYPE_IT) && (m_SndFile.m_dwSongFlags&SONG_ITPROJECT)))
	{	//We don't remove an instrument's unused samples in an ITP.
		wsprintf(s, "OpenMPT detected %d sample(s) referenced by an instrument,\n"
					"but not used in the song. Do you want to remove them ?", nExt);
		if (::MessageBox(NULL, s, "Sample Cleanup", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			for (UINT j=1; j<MAX_SAMPLES; j++)
			{
				if ((!bIns[j]) && (m_SndFile.Samples[j].pSample))
				{
					BEGIN_CRITICAL();
					m_SndFile.DestroySample(j);
					if ((j == m_SndFile.m_nSamples) && (j > 1)) m_SndFile.m_nSamples--;
					END_CRITICAL();
					nRemoved++;
				}
			}
		}
	}
	for (UINT ilo=1; ilo<=m_SndFile.m_nSamples; ilo++) if (m_SndFile.Samples[ilo].pSample)
	{
		if ((m_SndFile.Samples[ilo].uFlags & CHN_LOOP)
		 && (m_SndFile.Samples[ilo].nLength > m_SndFile.Samples[ilo].nLoopEnd + 2)) nLoopOpt++;
	}
	if (nLoopOpt)
	{
		wsprintf(s, "OpenMPT detected %d sample(s) with unused data after the loop end point,\n"
					"Do you want to optimize it, and remove this unused data ?", nLoopOpt);
		if (::MessageBox(NULL, s, "Sample Cleanup", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			for (UINT j=1; j<=m_SndFile.m_nSamples; j++)
			{
				if ((m_SndFile.Samples[j].uFlags & CHN_LOOP)
				 && (m_SndFile.Samples[j].nLength > m_SndFile.Samples[j].nLoopEnd + 2))
				{
					UINT lmax = m_SndFile.Samples[j].nLoopEnd + 2;
					if ((lmax < m_SndFile.Samples[j].nLength) && (lmax >= 16)) m_SndFile.Samples[j].nLength = lmax;
				}
			}
		} else nLoopOpt = 0;
	}
	if ((nRemoved) || (nLoopOpt))
	{
		if (nRemoved)
		{
			wsprintf(s, "%d unused sample(s) removed\n" ,nRemoved);
			AddToLog(s);
		}
		if (nLoopOpt)
		{
			wsprintf(s, "%d sample loop(s) optimized\n" ,nLoopOpt);
			AddToLog(s);
		}
		SetModified();
		return TRUE;
	}
	return FALSE;
}


UINT CModDoc::RemovePlugs(const bool (&keepMask)[MAX_MIXPLUGINS])
//---------------------------------------------------------------
{
	//Remove all plugins whose keepMask[plugindex] is false.
	UINT nRemoved=0;
	for (PLUGINDEX nPlug=0; nPlug<MAX_MIXPLUGINS; nPlug++)
	{
		SNDMIXPLUGIN* pPlug = &m_SndFile.m_MixPlugins[nPlug];		
		if (keepMask[nPlug] || !pPlug)
		{
			Log("Keeping mixplug addess (%d): %X\n", nPlug, &(pPlug->pMixPlugin));	
			continue;
		}

		if (pPlug->pPluginData)
		{
			delete pPlug->pPluginData;
			pPlug->pPluginData = NULL;
		}
		if (pPlug->pMixPlugin)
		{
			pPlug->pMixPlugin->Release();
			pPlug->pMixPlugin=NULL;
		}
		if (pPlug->pMixState)
		{
			delete pPlug->pMixState;
		}

		memset(&(pPlug->Info), 0, sizeof(SNDMIXPLUGININFO));
		Log("Zeroing range (%d) %X - %X\n", nPlug, &(pPlug->Info),  &(pPlug->Info)+sizeof(SNDMIXPLUGININFO));
		pPlug->nPluginDataSize=0;
		pPlug->fDryRatio=0;	
		pPlug->defaultProgram=0;
		nRemoved++;
	}

	return nRemoved;
}


BOOL CModDoc::RemoveUnusedPlugs() 
//-------------------------------
{
	bool usedmap[MAX_MIXPLUGINS];
	memset(usedmap, false, sizeof(usedmap));
	
	for (PLUGINDEX nPlug=0; nPlug < MAX_MIXPLUGINS; nPlug++) {

		//Is the plugin assigned to a channel?
		for (CHANNELINDEX nChn = 0; nChn < m_SndFile.GetNumChannels(); nChn++) {
			if (m_SndFile.ChnSettings[nChn].nMixPlugin == nPlug + 1u) {
				usedmap[nPlug]=true;
				break;
			}
		}

		//Is the plugin used by an instrument?
		for (INSTRUMENTINDEX nIns=1; nIns<=m_SndFile.GetNumInstruments(); nIns++) {
			if (m_SndFile.Instruments[nIns] && (m_SndFile.Instruments[nIns]->nMixPlug == nPlug+1)) {
				usedmap[nPlug]=true;
				break;
			}
		}

		//Is the plugin assigned to master?
		if (m_SndFile.m_MixPlugins[nPlug].Info.dwInputRouting & MIXPLUG_INPUTF_MASTEREFFECT) {
			usedmap[nPlug]=true;
		}

		//all outputs of used plugins count as used
		if (usedmap[nPlug]!=0) {
			if (m_SndFile.m_MixPlugins[nPlug].Info.dwOutputRouting & 0x80) {
				int output = m_SndFile.m_MixPlugins[nPlug].Info.dwOutputRouting & 0x7f;
				usedmap[output]=true;
			}
		}

	}

	UINT nRemoved = RemovePlugs(usedmap);

	if (nRemoved) {
		SetModified();
	}

	return nRemoved;
}


void CModDoc::RemoveAllInstruments(bool bConfirm)
//-----------------------------------------------
{
	if (!m_SndFile.m_nInstruments)
		return;

	char removeSamples = -1;
	if(bConfirm)
	{
		if (CMainFrame::GetMainFrame()->MessageBox("This will remove all the instruments in the song,\n"
			"Do you want to continue?", "Warning", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
		if (CMainFrame::GetMainFrame()->MessageBox("Do you want to convert all instruments to samples ?\n",
			NULL, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			ConvertInstrumentsToSamples();
		}
		
		if (::MessageBox(NULL, "Remove samples associated with an instrument if they are unused?", "Removing instrument", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			removeSamples = 1;
		}
	}

	for (INSTRUMENTINDEX i = 1; i <= m_SndFile.m_nInstruments; i++)
	{
		m_SndFile.DestroyInstrument(i,removeSamples);
	}

	m_SndFile.m_nInstruments = 0;
	SetModified();
	UpdateAllViews(NULL, HINT_MODTYPE);
}


BOOL CModDoc::RemoveUnusedInstruments()
//-------------------------------------
{
	BYTE usedmap[MAX_INSTRUMENTS];
	BYTE swapmap[MAX_INSTRUMENTS];
	BYTE swapdest[MAX_INSTRUMENTS];
	CHAR s[512];
	UINT nRemoved = 0;
	UINT nSwap, nIndex;
	bool bReorg = false;

	if (!m_SndFile.m_nInstruments) return FALSE;

	char removeSamples = -1;
	if ( !((m_SndFile.m_nType & MOD_TYPE_IT) && (m_SndFile.m_dwSongFlags&SONG_ITPROJECT))) { //never remove an instrument's samples in ITP.
		if(::MessageBox(NULL, "Remove samples associated with an instrument if they are unused?", "Removing instrument", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			removeSamples = 1;
		}
	} else {
		MessageBox(NULL, "This is an IT project, so no samples associated with a used instrument will be removed.", "Removing Instruments", MB_OK | MB_ICONINFORMATION);
	}

	BeginWaitCursor();
	memset(usedmap, 0, sizeof(usedmap));

	for(INSTRUMENTINDEX i = m_SndFile.m_nInstruments; i >= 1; i--)
	{
		if (!m_SndFile.IsInstrumentUsed(i))
		{
			BEGIN_CRITICAL();
// -> CODE#0003
// -> DESC="remove instrument's samples"
//			m_SndFile.DestroyInstrument(i);
			m_SndFile.DestroyInstrument(i, removeSamples);
// -! BEHAVIOUR_CHANGE#0003
			if ((i == m_SndFile.m_nInstruments) && (i>1)) m_SndFile.m_nInstruments--; else bReorg = true;
			END_CRITICAL();
			nRemoved++;
		} else
		{
			usedmap[i] = 1;
		}
	}
	EndWaitCursor();
	if ((bReorg) && (m_SndFile.m_nInstruments > 1)
	 && (::MessageBox(NULL, "Do you want to reorganize the remaining instruments?", "Instrument Cleanup", MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		BeginWaitCursor();
		BEGIN_CRITICAL();
		nSwap = 0;
		nIndex = 1;
		for (UINT j=1; j<=m_SndFile.m_nInstruments; j++)
		{
			if (usedmap[j])
			{
				while (nIndex<j)
				{
					if ((!usedmap[nIndex]) && (!m_SndFile.Instruments[nIndex]))
					{
						swapmap[nSwap] = j;
						swapdest[nSwap] = nIndex;
						m_SndFile.Instruments[nIndex] = m_SndFile.Instruments[j];
						m_SndFile.Instruments[j] = NULL;
						usedmap[nIndex] = 1;
						usedmap[j] = 0;
						nSwap++;
						nIndex++;
						break;
					}
					nIndex++;
				}
			}
		}
		while ((m_SndFile.m_nInstruments > 1) && (!m_SndFile.Instruments[m_SndFile.m_nInstruments])) m_SndFile.m_nInstruments--;
		END_CRITICAL();
		if (nSwap > 0)
		{
			for (UINT iPat=0; iPat<m_SndFile.Patterns.Size(); iPat++) if (m_SndFile.Patterns[iPat])
			{
				MODCOMMAND *p = m_SndFile.Patterns[iPat];
				UINT nLen = m_SndFile.m_nChannels * m_SndFile.PatternSize[iPat];
				while (nLen--)
				{
					if (p->instr)
					{
						for (UINT k=0; k<nSwap; k++)
						{
							if (p->instr == swapmap[k]) p->instr = swapdest[k];
						}
					}
					p++;
				}
			}
		}
		EndWaitCursor();
	}
	if (nRemoved)
	{
		wsprintf(s, "%d unused instrument(s) removed\n", nRemoved);
		AddToLog(s);
		SetModified();
		return TRUE;
	}
	return FALSE;
}


BOOL CModDoc::CompoCleanup()
//--------------------------
{
	//jojo.compocleanup
	if(::MessageBox(NULL, TEXT("WARNING: Compo cleanup will convert module to IT format, remove all patterns and reset song, sample and instrument attributes to default values. Continue?"), TEXT("Compo Cleanup"), MB_YESNO | MB_ICONWARNING) == IDNO)
		return FALSE;

	// Stop play.
	CMainFrame::GetMainFrame()->StopMod(this);

	BeginWaitCursor();
	BEGIN_CRITICAL();

	// convert to IT...
	ChangeModType(MOD_TYPE_IT);
	m_SndFile.m_nMixLevels = mixLevels_original;
	m_SndFile.m_nTempoMode = tempo_mode_classic;
	m_SndFile.m_dwSongFlags = SONG_LINEARSLIDES | SONG_EXFILTERRANGE;
	
	// clear order list
	m_SndFile.Order.Init();
	m_SndFile.Order[0] = 0;

	// remove all patterns
	m_SndFile.Patterns.Init();
	m_SndFile.Patterns.Insert(0, 64);
	m_SndFile.SetCurrentOrder(0);

	// Global vars
	m_SndFile.m_nDefaultTempo = 125;
	m_SndFile.m_nDefaultSpeed = 6;
	m_SndFile.m_nDefaultGlobalVolume = 256;
	m_SndFile.m_nSamplePreAmp = 48;
	m_SndFile.m_nVSTiVolume = 48;
	m_SndFile.m_nRestartPos = 0;

	// Set 4 default channels.
	m_SndFile.ReArrangeChannels(vector<CHANNELINDEX>(4, MAX_BASECHANNELS));

	//remove plugs
	bool keepMask[MAX_MIXPLUGINS]; memset(keepMask, 0, sizeof(keepMask));
	RemovePlugs(keepMask);

	// instruments
	if(m_SndFile.m_nInstruments && ::MessageBox(NULL, "Remove instruments?", "Compo Cleanup", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		// remove instruments
		RemoveAllInstruments(false);
	}
	else
	{
		// reset instruments
		for(UINT i = 1; i <= m_SndFile.m_nInstruments; i++)
		{
			m_SndFile.Instruments[i]->nFadeOut = 256;
			m_SndFile.Instruments[i]->nGlobalVol = 64;
			m_SndFile.Instruments[i]->nPan = 128;
			m_SndFile.Instruments[i]->dwFlags &= ~ENV_SETPANNING;
			m_SndFile.Instruments[i]->nMixPlug = 0;

			m_SndFile.Instruments[i]->nVolSwing = 0;
			m_SndFile.Instruments[i]->nPanSwing = 0;
			m_SndFile.Instruments[i]->nCutSwing = 0;
			m_SndFile.Instruments[i]->nResSwing = 0;

			//might be a good idea to leave those enabled...
			/*
			m_SndFile.Instruments[i]->dwFlags &= ~ENV_VOLUME;
			m_SndFile.Instruments[i]->dwFlags &= ~ENV_PANNING;
			m_SndFile.Instruments[i]->dwFlags &= ~ENV_PITCH;
			m_SndFile.Instruments[i]->dwFlags &= ~ENV_FILTER;
			*/
		}
	}

	// reset samples
	ctrlSmp::ResetSamples(m_SndFile, ctrlSmp::SmpResetCompo);

	// Set modflags.
	m_SndFile.SetModFlag(MSF_MIDICC_BUGEMULATION, false);
	m_SndFile.SetModFlag(MSF_OLDVOLSWING, false);
	m_SndFile.SetModFlag(MSF_COMPATIBLE_PLAY, true);

	END_CRITICAL();
	EndWaitCursor();

	UpdateAllViews(NULL, HINT_MODGENERAL, this);

	SetModified();
	return TRUE;
}


BOOL CModDoc::AdjustEndOfSample(UINT nSample)
//-------------------------------------------
{
	MODSAMPLE *pSmp;
	if (nSample >= MAX_SAMPLES) return FALSE;
	pSmp = &m_SndFile.Samples[nSample];
	if ((!pSmp->nLength) || (!pSmp->pSample)) return FALSE;

	ctrlSmp::AdjustEndOfSample(*pSmp, &m_SndFile);

	return TRUE;
}


PATTERNINDEX CModDoc::InsertPattern(ORDERINDEX nOrd, ROWINDEX nRows)
//------------------------------------------------------------------
{
	const int i = m_SndFile.Patterns.Insert(nRows);
	if(i < 0)
		return PATTERNINDEX_INVALID;

	//Increasing orderlist size if given order is beyond current limit,
	//or if the last order already has a pattern.
	if((nOrd == m_SndFile.Order.size() ||
		m_SndFile.Order.Last() < m_SndFile.Patterns.Size() ) &&
		m_SndFile.Order.GetLength() < m_SndFile.GetModSpecifications().ordersMax)
	{
		m_SndFile.Order.Append();
	}

	for (UINT j=0; j<m_SndFile.Order.size(); j++)
	{
		if (m_SndFile.Order[j] == i) break;
		if (m_SndFile.Order[j] == m_SndFile.Order.GetInvalidPatIndex())
		{
			m_SndFile.Order[j] = i;
			break;
		}
		if ((nOrd >= 0) && (j == (UINT)nOrd))
		{
			for (UINT k=m_SndFile.Order.size()-1; k>j; k--)
			{
				m_SndFile.Order[k] = m_SndFile.Order[k-1];
			}
			m_SndFile.Order[j] = i;
			break;
		}
	}

	SetModified();
	return i;
}


SAMPLEINDEX CModDoc::InsertSample(bool bLimit)
//--------------------------------------------
{
	SAMPLEINDEX i = 1;
	for(i = 1; i <= m_SndFile.m_nSamples; i++)
	{
		if ((!m_SndFile.m_szNames[i][0]) && (m_SndFile.Samples[i].pSample == NULL))
		{
			if ((!m_SndFile.m_nInstruments) || (!m_SndFile.IsSampleUsed(i)))
			break;
		}
	}
	if (((bLimit) && (i >= 200) && (!m_SndFile.m_nInstruments))
	 || (i >= m_SndFile.GetModSpecifications().samplesMax))
	{
		ErrorBox(IDS_ERR_TOOMANYSMP, CMainFrame::GetMainFrame());
		return SAMPLEINDEX_INVALID;
	}
	if (!m_SndFile.m_szNames[i][0]) strcpy(m_SndFile.m_szNames[i], "untitled");
	MODSAMPLE *pSmp = &m_SndFile.Samples[i];
	pSmp->nVolume = 256;
	pSmp->nGlobalVol = 64;
	pSmp->nPan = 128;
	pSmp->nC5Speed = 8363;
	pSmp->RelativeTone = 0;
	pSmp->nFineTune = 0;
	pSmp->nVibType = 0;
	pSmp->nVibSweep = 0;
	pSmp->nVibDepth = 0;
	pSmp->nVibRate = 0;
	pSmp->uFlags &= ~(CHN_PANNING|CHN_SUSTAINLOOP);
	if (m_SndFile.m_nType == MOD_TYPE_XM) pSmp->uFlags |= CHN_PANNING;
	if (i > m_SndFile.m_nSamples) m_SndFile.m_nSamples = i;
	SetModified();
	return i;
}


INSTRUMENTINDEX CModDoc::InsertInstrument(LONG lSample, LONG lDuplicate)
//-----------------------------------------------------------
{
	MODINSTRUMENT *pDup = NULL;
	INSTRUMENTINDEX nInstrumentMax = m_SndFile.GetModSpecifications().instrumentsMax - 1;
	if ((m_SndFile.m_nType != MOD_TYPE_XM) && !(m_SndFile.m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT))) return INSTRUMENTINDEX_INVALID;
	if ((lDuplicate > 0) && (lDuplicate <= (LONG)m_SndFile.m_nInstruments))
	{
		pDup = m_SndFile.Instruments[lDuplicate];
	}
	if ((!m_SndFile.m_nInstruments) && ((m_SndFile.m_nSamples > 1) || (m_SndFile.Samples[1].pSample)))
	{
		if (pDup) return INSTRUMENTINDEX_INVALID;
		UINT n = CMainFrame::GetMainFrame()->MessageBox("Convert existing samples to instruments first?", NULL, MB_YESNOCANCEL|MB_ICONQUESTION);
		if (n == IDYES)
		{
			UINT nInstruments = m_SndFile.m_nSamples;
			if (nInstruments > nInstrumentMax) nInstruments = nInstrumentMax;
			for (UINT smp=1; smp<=nInstruments; smp++)
			{
				m_SndFile.Samples[smp].uFlags &= ~CHN_MUTE;
				if (!m_SndFile.Instruments[smp])
				{
					MODINSTRUMENT *p = new MODINSTRUMENT;
					if (!p)
					{
						ErrorBox(IDS_ERR_OUTOFMEMORY, CMainFrame::GetMainFrame());
						return INSTRUMENTINDEX_INVALID;
					}
					InitializeInstrument(p, smp);
					m_SndFile.Instruments[smp] = p;
					lstrcpyn(p->name, m_SndFile.m_szNames[smp], sizeof(p->name));
				}
			}
			m_SndFile.m_nInstruments = nInstruments;
		} else
		if (n != IDNO) return INSTRUMENTINDEX_INVALID;
	}
	UINT newins = 0;
	for (UINT i=1; i<=m_SndFile.m_nInstruments; i++)
	{
		if (!m_SndFile.Instruments[i])
		{
			newins = i;
			break;
		}
	}
	if (!newins)
	{
		if (m_SndFile.m_nInstruments >= nInstrumentMax)
		{
			ErrorBox(IDS_ERR_TOOMANYINS, CMainFrame::GetMainFrame());
			return INSTRUMENTINDEX_INVALID;
		}
		newins = ++m_SndFile.m_nInstruments;
	}
	MODINSTRUMENT *pIns = new MODINSTRUMENT;
	if (pIns)
	{
		UINT newsmp = 0;
		if ((lSample > 0) && (lSample < m_SndFile.GetModSpecifications().samplesMax))
		{
			newsmp = lSample;
		} else
		if (!pDup)
		{
			for(SAMPLEINDEX k = 1; k <= m_SndFile.m_nSamples; k++)
			{
				if (!m_SndFile.IsSampleUsed(k))
				{
					newsmp = k;
					break;
				}
			}
			if (!newsmp)
			{
				int inssmp = InsertSample();
				if (inssmp != SAMPLEINDEX_INVALID) newsmp = inssmp;
			}
		}
		BEGIN_CRITICAL();
		if (pDup)
		{
			*pIns = *pDup;
// -> CODE#0023
// -> DESC="IT project files (.itp)"
			strcpy(m_SndFile.m_szInstrumentPath[newins-1],m_SndFile.m_szInstrumentPath[lDuplicate-1]);
			m_SndFile.instrumentModified[newins-1] = FALSE;
// -! NEW_FEATURE#0023
		} else
		{
			InitializeInstrument(pIns, newsmp);
		}
		m_SndFile.Instruments[newins] = pIns;
		END_CRITICAL();
		SetModified();
	} else
	{
		ErrorBox(IDS_ERR_OUTOFMEMORY, CMainFrame::GetMainFrame());
		return INSTRUMENTINDEX_INVALID;
	}
	return newins;
}


void CModDoc::InitializeInstrument(MODINSTRUMENT *pIns, UINT nsample)
//----------------------------------------------------------------------
{
	memset(pIns, 0, sizeof(MODINSTRUMENT));
	pIns->nFadeOut = 256;
	pIns->nGlobalVol = 64;
	pIns->nPan = 128;
	pIns->nPPC = 5*12;
	m_SndFile.SetDefaultInstrumentValues(pIns);
	for (UINT n=0; n<128; n++)
	{
		pIns->Keyboard[n] = nsample;
		pIns->NoteMap[n] = n+1;
	}
	pIns->pTuning = pIns->s_DefaultTuning;
}


bool CModDoc::RemoveOrder(ORDERINDEX n)
//-------------------------------
{
	if (n < m_SndFile.Order.size())
	{
		BEGIN_CRITICAL();
		for (ORDERINDEX i=n; i<m_SndFile.Order.size()-1; i++)
		{
			m_SndFile.Order[i] = m_SndFile.Order[i+1];
		}
		m_SndFile.Order[m_SndFile.Order.size()-1] = m_SndFile.Order.GetInvalidPatIndex();
		END_CRITICAL();
		SetModified();
		return true;
	}
	return false;
}


bool CModDoc::RemovePattern(PATTERNINDEX n)
//---------------------------------
{
	if ((n < m_SndFile.Patterns.Size()) && (m_SndFile.Patterns[n]))
	{
		BEGIN_CRITICAL();
		LPVOID p = m_SndFile.Patterns[n];
		m_SndFile.Patterns[n] = nullptr;
		m_SndFile.SetPatternName(n, "");
		CSoundFile::FreePattern(p);
		END_CRITICAL();
		SetModified();
		return true;
	}
	return false;
}


bool CModDoc::RemoveSample(SAMPLEINDEX n)
//--------------------------------
{
	if ((n) && (n <= m_SndFile.m_nSamples))
	{
		BEGIN_CRITICAL();
		m_SndFile.DestroySample(n);
		m_SndFile.m_szNames[n][0] = 0;
		while ((m_SndFile.m_nSamples > 1)
		 && (!m_SndFile.m_szNames[m_SndFile.m_nSamples][0])
		 && (!m_SndFile.Samples[m_SndFile.m_nSamples].pSample)) m_SndFile.m_nSamples--;
		END_CRITICAL();
		SetModified();
		return true;
	}
	return false;
}


bool CModDoc::RemoveInstrument(INSTRUMENTINDEX n)
//------------------------------------
{
	if ((n) && (n <= m_SndFile.m_nInstruments) && (m_SndFile.Instruments[n]))
	{
		BOOL bIns = FALSE;
		BEGIN_CRITICAL();
		m_SndFile.DestroyInstrument(n);
		if (n == m_SndFile.m_nInstruments) m_SndFile.m_nInstruments--;
		for (UINT i=1; i<MAX_INSTRUMENTS; i++) if (m_SndFile.Instruments[i]) bIns = TRUE;
		if (!bIns) m_SndFile.m_nInstruments = 0;
		END_CRITICAL();
		SetModified();
		return true;
	}
	return false;
}


bool CModDoc::MoveOrder(UINT nSourceNdx, UINT nDestNdx, bool bUpdate, bool bCopy)
//-------------------------------------------------------------------------------
{
	if ((nSourceNdx >= m_SndFile.Order.size()) || (nDestNdx >= m_SndFile.Order.size())) return false;
	if (nDestNdx >= m_SndFile.GetModSpecifications().ordersMax) return false;
	ORDERINDEX n = m_SndFile.Order[nSourceNdx];
	// Delete source
	if (!bCopy)
	{
		for (ORDERINDEX i = nSourceNdx; i < m_SndFile.Order.size() - 1; i++) m_SndFile.Order[i] = m_SndFile.Order[i+1];
		if (nSourceNdx < nDestNdx) nDestNdx--;
	}
	// Insert at dest
	for (ORDERINDEX j = m_SndFile.Order.size() - 1; j > nDestNdx; j--) m_SndFile.Order[j] = m_SndFile.Order[j-1];
	m_SndFile.Order[nDestNdx] = n;
	if (bUpdate)
	{
		UpdateAllViews(NULL, HINT_MODSEQUENCE, NULL);
	}
	return true;
}


BOOL CModDoc::ExpandPattern(PATTERNINDEX nPattern)
//----------------------------------------
{
// -> CODE#0008
// -> DESC="#define to set pattern size"

	if ((nPattern >= m_SndFile.Patterns.Size()) || (!m_SndFile.Patterns[nPattern])) return FALSE;
	if(m_SndFile.Patterns[nPattern].Expand())
		return FALSE;
	else
		return TRUE;
}


BOOL CModDoc::ShrinkPattern(PATTERNINDEX nPattern)
//----------------------------------------
{
	if ((nPattern >= m_SndFile.Patterns.Size()) || (!m_SndFile.Patterns[nPattern])) return FALSE;
	if(m_SndFile.Patterns[nPattern].Shrink())
		return FALSE;
	else
		return TRUE;
}


// Clipboard format:
// Hdr: "ModPlug Tracker S3M\n"
// Full:  '|C#401v64A06'
// Reset: '|...........'
// Empty: '|           '
// End of row: '\n'

static LPCSTR lpszClipboardPatternHdr = "ModPlug Tracker %3s\x0D\x0A";

BOOL CModDoc::CopyPattern(PATTERNINDEX nPattern, DWORD dwBeginSel, DWORD dwEndSel)
//------------------------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	DWORD dwMemSize;
	HGLOBAL hCpy;
	UINT nrows = (dwEndSel >> 16) - (dwBeginSel >> 16) + 1;
	UINT ncols = ((dwEndSel & 0xFFFF) >> 3) - ((dwBeginSel & 0xFFFF) >> 3) + 1;

	if ((!pMainFrm) || (nPattern >= m_SndFile.Patterns.Size()) || (!m_SndFile.Patterns[nPattern])) return FALSE;
	BeginWaitCursor();
	dwMemSize = strlen(lpszClipboardPatternHdr) + 1;
	dwMemSize += nrows * (ncols * 12 + 2);
	if ((pMainFrm->OpenClipboard()) && ((hCpy = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwMemSize))!=NULL))
	{
		LPCSTR pszFormatName;
		EmptyClipboard();
		switch(m_SndFile.m_nType)
		{
		case MOD_TYPE_S3M:	pszFormatName = "S3M"; break;
		case MOD_TYPE_XM:	pszFormatName = "XM"; break;
		case MOD_TYPE_IT:	pszFormatName = "IT"; break;
		case MOD_TYPE_MPT:	pszFormatName = "MPT"; break;
		default:			pszFormatName = "MOD"; break;
		}
		LPSTR p = (LPSTR)GlobalLock(hCpy);
		if (p)
		{
			UINT colmin = dwBeginSel & 0xFFFF;
			UINT colmax = dwEndSel & 0xFFFF;
			wsprintf(p, lpszClipboardPatternHdr, pszFormatName);
			p += strlen(p);
			for (UINT row=0; row<nrows; row++)
			{
				MODCOMMAND *m = m_SndFile.Patterns[nPattern];
				if ((row + (dwBeginSel >> 16)) >= m_SndFile.PatternSize[nPattern]) break;
				m += (row+(dwBeginSel >> 16))*m_SndFile.m_nChannels;
				m += (colmin >> 3);
				for (UINT col=0; col<ncols; col++, m++, p+=12)
				{
					UINT ncursor = ((colmin>>3)+col) << 3;
					p[0] = '|';
					// Note
					if ((ncursor >= colmin) && (ncursor <= colmax))
					{
						UINT note = m->note;
						switch(note)
						{
						case 0:		p[1] = p[2] = p[3] = '.'; break;
						case NOTE_KEYOFF:	p[1] = p[2] = p[3] = '='; break;
						case NOTE_NOTECUT:	p[1] = p[2] = p[3] = '^'; break;
						case NOTE_FADE:	p[1] = p[2] = p[3] = '~'; break;
						case NOTE_PC: p[1] = 'P'; p[2] = 'C'; p[3] = ' '; break;
						case NOTE_PCS: p[1] = 'P'; p[2] = 'C'; p[3] = 'S'; break;
						default:
							p[1] = szNoteNames[(note-1) % 12][0];
							p[2] = szNoteNames[(note-1) % 12][1];
							p[3] = '0' + (note-1) / 12;
						}
					} else
					{
						// No note
						p[1] = p[2] = p[3] = ' ';
					}
					// Instrument
					ncursor++;
					if ((ncursor >= colmin) && (ncursor <= colmax))
					{
						if (m->instr)
						{
							p[4] = '0' + (m->instr / 10);
							p[5] = '0' + (m->instr % 10);
						} else p[4] = p[5] = '.';
					} else
					{
						p[4] = p[5] = ' ';
					}
					// Volume
					ncursor++;
					if ((ncursor >= colmin) && (ncursor <= colmax))
					{
						if(m->note == NOTE_PC || m->note == NOTE_PCS)
						{
							const uint16 val = m->GetValueVolCol();
							p[6] = GetDigit<2>(val);
							p[7] = GetDigit<1>(val);
							p[8] = GetDigit<0>(val);
						}
						else
						{
							if ((m->volcmd) && (m->volcmd <= MAX_VOLCMDS))
							{
								p[6] = gszVolCommands[m->volcmd];
								p[7] = '0' + (m->vol / 10);
								p[8] = '0' + (m->vol % 10);
							} else p[6] = p[7] = p[8] = '.';
						}
					} else
					{
						p[6] = p[7] = p[8] = ' ';
					}
					// Effect
					ncursor++;
					if (((ncursor >= colmin) && (ncursor <= colmax))
					 || ((ncursor+1 >= colmin) && (ncursor+1 <= colmax)))
					{
						if(m->note == NOTE_PC || m->note == NOTE_PCS)
						{
							const uint16 val = m->GetValueEffectCol();
							p[9] = GetDigit<2>(val);
							p[10] = GetDigit<1>(val);
							p[11] = GetDigit<0>(val);
						}
						else
						{
							if (m->command)
							{
								if (m_SndFile.m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))
									p[9] = gszS3mCommands[m->command];
								else
									p[9] = gszModCommands[m->command];
							} else p[9] = '.';
							if (m->param)
							{
								p[10] = szHexChar[m->param >> 4];
								p[11] = szHexChar[m->param & 0x0F];
							} else p[10] = p[11] = '.';
						}
					} else
					{
						p[9] = p[10] = p[11] = ' ';
					}
				}
				*p++ = 0x0D;
				*p++ = 0x0A;
			}
			*p = 0;
		}
		GlobalUnlock(hCpy);
		SetClipboardData (CF_TEXT, (HANDLE) hCpy);
		CloseClipboard();
	}
	EndWaitCursor();
	return TRUE;
}


//rewbs.mixpaste: using eric's method, as it is fat more elegant.
// -> CODE#0014
// -> DESC="vst wet/dry slider"
//BOOL CModDoc::PastePattern(UINT nPattern, DWORD dwBeginSel)
BOOL CModDoc::PastePattern(PATTERNINDEX nPattern, DWORD dwBeginSel, BOOL mix, BOOL ITStyleMix)
// -! NEW_FEATURE#0014
//---------------------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	if ((!pMainFrm) || (nPattern >= m_SndFile.Patterns.Size()) || (!m_SndFile.Patterns[nPattern])) return FALSE;
	BeginWaitCursor();
	if (pMainFrm->OpenClipboard())
	{
		HGLOBAL hCpy = ::GetClipboardData(CF_TEXT);
		LPSTR p;

		if ((hCpy) && ((p = (LPSTR)GlobalLock(hCpy)) != NULL))
		{
			PrepareUndo(nPattern, 0, 0, m_SndFile.m_nChannels, m_SndFile.PatternSize[nPattern]);
			BYTE spdmax = (m_SndFile.m_nType & MOD_TYPE_MOD) ? 0x20 : 0x1F;
			DWORD dwMemSize = GlobalSize(hCpy);
			MODCOMMAND *m = m_SndFile.Patterns[nPattern];
			UINT nrow = dwBeginSel >> 16;
			UINT ncol = (dwBeginSel & 0xFFFF) >> 3;
			UINT col;
			BOOL bS3M = FALSE, bOk = FALSE;
			UINT len = 0;
			MODCOMMAND origModCmd;

			ORDERINDEX oCurrentOrder; //jojo.echopaste
			ROWINDEX rTemp;
			PATTERNINDEX pTemp;
			GetEditPosition(rTemp, pTemp, oCurrentOrder);

			if ((nrow >= m_SndFile.PatternSize[nPattern]) || (ncol >= m_SndFile.m_nChannels)) goto PasteDone;
			m += nrow * m_SndFile.m_nChannels;
			// Search for signature
			for (;;)
			{
				if (len + 11 >= dwMemSize) goto PasteDone;
				char c = p[len++];
				if (!c) goto PasteDone;
				if ((c == 0x0D) && (len > 3))
				{
					//if ((p[len-3] == 'I') || (p[len-4] == 'S')) bS3M = TRUE;
					//				IT?					S3M?				MPT?
					if ((p[len-3] == 'I') || (p[len-4] == 'S') || (p[len-3] == 'P')) bS3M = TRUE;
					break;
				}
			}
			bOk = TRUE;
			while ((nrow < m_SndFile.PatternSize[nPattern]) && (len + 11 < dwMemSize))
			{
				// Search for column separator
				while (p[len] != '|')
				{
					if (len + 11 >= dwMemSize) goto PasteDone;
					if (!p[len]) goto PasteDone;
					len++;
				}
				col = ncol;
				// Paste columns
				while ((p[len] == '|') && (len + 11 < dwMemSize))
				{
					origModCmd = m[col]; // ITSyle mixpaste requires that we keep a copy of the thing we are about to paste on
					                     // so that we can refer back to check if there was anything in e.g. the note column before we pasted.
					LPSTR s = p+len+1;
					if (col < m_SndFile.m_nChannels)
					{
						// Note
						if (s[0] > ' ' && (!mix || ((!ITStyleMix && origModCmd.note==0) || 
												     (ITStyleMix && origModCmd.note==0 && origModCmd.instr==0 && origModCmd.volcmd==0))))
						{
							m[col].note = NOTE_NONE;
							if (s[0] == '=') m[col].note = NOTE_KEYOFF; else
							if (s[0] == '^') m[col].note = NOTE_NOTECUT; else
							if (s[0] == '~') m[col].note = NOTE_FADE; else
							if (s[0] == 'P')
							{
								if(s[2] == 'S')
									m[col].note = NOTE_PCS;
								else
									m[col].note = NOTE_PC;
							} else
							if (s[0] != '.')
							{
								for (UINT i=0; i<12; i++)
								{
									if ((s[0] == szNoteNames[i][0])
									 && (s[1] == szNoteNames[i][1])) m[col].note = i+1;
								}
								if (m[col].note) m[col].note += (s[2] - '0') * 12;
							}
						}
						// Instrument
						if (s[3] > ' ' && (!mix || ( (!ITStyleMix && origModCmd.instr==0) || 
												     (ITStyleMix  && origModCmd.note==0 && origModCmd.instr==0 && origModCmd.volcmd==0) ) ))

						{
							if ((s[3] >= '0') && (s[3] <= ('0'+(MAX_SAMPLES/10))))
							{
								m[col].instr = (s[3]-'0') * 10 + (s[4]-'0');
							} else m[col].instr = 0;
						}
						// Volume
						if (s[5] > ' ' && (!mix || ((!ITStyleMix && origModCmd.volcmd==0) || 
												     (ITStyleMix && origModCmd.note==0 && origModCmd.instr==0 && origModCmd.volcmd==0))))

						{
							if (s[5] != '.')
							{
								if(m[col].note == NOTE_PCS || m[col].note == NOTE_PC)
								{
									char val[4];
									memcpy(val, s+5, 3);
									val[3] = 0;
									m[col].SetValueVolCol(ConvertStrTo<uint16>(val));
								}
								else
								{
									m[col].volcmd = 0;
									for (UINT i=1; i<MAX_VOLCMDS; i++)
									{
										if (s[5] == gszVolCommands[i])
										{
											m[col].volcmd = i;
											break;
										}
									}
									m[col].vol = (s[6]-'0')*10 + (s[7]-'0');
								}
							} else m[col].volcmd = m[col].vol = 0;
						}
						
						if(m[col].note == NOTE_PCS || m[col].note == NOTE_PC)
						{
							if(s[8] != '.')
							{
								char val[4];
								memcpy(val, s+8, 3);
								val[3] = 0;
								m[col].SetValueEffectCol(ConvertStrTo<uint16>(val));
							}
						}
						else
						{
							if (s[8] > ' ' && (!mix || ((!ITStyleMix && origModCmd.command==0) || 
														(ITStyleMix && origModCmd.command==0 && origModCmd.param==0))))
							{
								m[col].command = 0;
								if (s[8] != '.')
								{
									LPCSTR psc = (bS3M) ? gszS3mCommands : gszModCommands;
									for (UINT i=1; i<MAX_EFFECTS; i++)
									{
										if ((s[8] == psc[i]) && (psc[i] != '?')) m[col].command = i;
									}
								}
							}
							// Effect value
							if (s[9] > ' ' && (!mix || ((!ITStyleMix && origModCmd.param==0) || 
														(ITStyleMix && origModCmd.command==0 && origModCmd.param==0))))
							{
								m[col].param = 0;
								if (s[9] != '.')
								{
									for (UINT i=0; i<16; i++)
									{
										if (s[9] == szHexChar[i]) m[col].param |= (i<<4);
										if (s[10] == szHexChar[i]) m[col].param |= i;
									}
								}
							}
							// Checking command
							if (m_SndFile.m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM))
							{
								switch (m[col].command)
								{
								case CMD_SPEED:
								case CMD_TEMPO:
									if (!bS3M) m[col].command = (m[col].param <= spdmax) ? CMD_SPEED : CMD_TEMPO;
									else
									{
										if ((m[col].command == CMD_SPEED) && (m[col].param > spdmax)) m[col].param = CMD_TEMPO; else
										if ((m[col].command == CMD_TEMPO) && (m[col].param <= spdmax)) m[col].param = CMD_SPEED;
									}
									break;
								}
							} else
							{
								switch (m[col].command)
								{
								case CMD_SPEED:
								case CMD_TEMPO:
									if (!bS3M) m[col].command = (m[col].param <= spdmax) ? CMD_SPEED : CMD_TEMPO;
									break;
								}
							}
						}
					}
					len += 12;
					col++;
				}
				// Next row
				m += m_SndFile.m_nChannels;
				nrow++;

				//jojo.echopaste
				if(CMainFrame::m_dwPatternSetup & PATTERN_OVERFLOWPASTE)
				{
					while(nrow >= m_SndFile.PatternSize[nPattern])
					{
						nrow = 0;
						ORDERINDEX oNextOrder = m_SndFile.Order.GetNextOrderIgnoringSkips(oCurrentOrder);
						if((oNextOrder <= 0) || (oNextOrder >= m_SndFile.Order.size())) goto PasteDone;
						nPattern = m_SndFile.Order[oNextOrder];
						if(m_SndFile.Patterns.IsValidPat(nPattern) == false) goto PasteDone;
						m = m_SndFile.Patterns[nPattern];
						PrepareUndo(nPattern, 0,0, m_SndFile.m_nChannels, m_SndFile.PatternSize[nPattern]);
						oCurrentOrder = oNextOrder;
					}
				}
			
			}
		PasteDone:
			GlobalUnlock(hCpy);
			if (bOk)
			{
				SetModified();
				UpdateAllViews(NULL, HINT_PATTERNDATA | (nPattern << HINT_SHIFT_PAT), NULL);
			}
		}
		CloseClipboard();
	}
	EndWaitCursor();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Copy/Paste envelope

static LPCSTR pszEnvHdr = "Modplug Tracker Envelope\x0D\x0A";
static LPCSTR pszEnvFmt = "%d,%d,%d,%d,%d,%d,%d,%d\x0D\x0A";

BOOL CModDoc::CopyEnvelope(UINT nIns, UINT nEnv)
//----------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();
	HANDLE hCpy;
	CHAR s[4096];
	MODINSTRUMENT *pIns;
	DWORD dwMemSize;
	UINT bSus, bLoop, bCarry;

	if ((nIns < 1) || (nIns > m_SndFile.m_nInstruments) || (!m_SndFile.Instruments[nIns]) || (!pMainFrm)) return FALSE;
	BeginWaitCursor();
	pIns = m_SndFile.Instruments[nIns];
	
	INSTRUMENTENVELOPE *pEnv = nullptr;

	switch(nEnv)
	{
	case ENV_PANNING:
		pEnv = &pIns->PanEnv;
		bLoop = (pIns->dwFlags & ENV_PANLOOP) ? 1 : 0;
		bSus = (pIns->dwFlags & ENV_PANSUSTAIN) ? 1 : 0;
		bCarry = (pIns->dwFlags & ENV_PANCARRY) ? 1 : 0;
		break;

	case ENV_PITCH:
		pEnv = &pIns->PitchEnv;
		bLoop = (pIns->dwFlags & ENV_PITCHLOOP) ? 1 : 0;
		bSus = (pIns->dwFlags & ENV_PITCHSUSTAIN) ? 1 : 0;
		bCarry = (pIns->dwFlags & ENV_PITCHCARRY) ? 1 : 0;
		break;

	default:
		pEnv = &pIns->VolEnv;
		bLoop = (pIns->dwFlags & ENV_VOLLOOP) ? 1 : 0;
		bSus = (pIns->dwFlags & ENV_VOLSUSTAIN) ? 1 : 0;
		bCarry = (pIns->dwFlags & ENV_VOLCARRY) ? 1 : 0;
		break;
	}
	strcpy(s, pszEnvHdr);
	wsprintf(s + strlen(s), pszEnvFmt, pEnv->nNodes, pEnv->nSustainStart, pEnv->nSustainEnd, pEnv->nLoopStart, pEnv->nLoopEnd, bSus, bLoop, bCarry);
	for (UINT i = 0; i < pEnv->nNodes; i++)
	{
		if (strlen(s) >= sizeof(s)-32) break;
		wsprintf(s+strlen(s), "%d,%d\x0D\x0A", pEnv->Ticks[i], pEnv->Values[i]);
	}

	//Writing release node
	if(strlen(s) < sizeof(s) - 32)
		wsprintf(s+strlen(s), "%u\x0D\x0A", pEnv->nReleaseNode);

	dwMemSize = strlen(s)+1;
	if ((pMainFrm->OpenClipboard()) && ((hCpy = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwMemSize))!=NULL))
	{
		EmptyClipboard();
		LPBYTE p = (LPBYTE)GlobalLock(hCpy);
		memcpy(p, s, dwMemSize);
		GlobalUnlock(hCpy);
		SetClipboardData (CF_TEXT, (HANDLE)hCpy);
		CloseClipboard();
	}
	EndWaitCursor();
	return TRUE;
}


BOOL CModDoc::PasteEnvelope(UINT nIns, UINT nEnv)
//-----------------------------------------------
{
	CMainFrame *pMainFrm = CMainFrame::GetMainFrame();

	if ((nIns < 1) || (nIns > m_SndFile.m_nInstruments) || (!m_SndFile.Instruments[nIns]) || (!pMainFrm)) return FALSE;
	BeginWaitCursor();
	if (!pMainFrm->OpenClipboard())
	{
		EndWaitCursor();
		return FALSE;
	}
	HGLOBAL hCpy = ::GetClipboardData(CF_TEXT);
	LPCSTR p;
	if ((hCpy) && ((p = (LPSTR)GlobalLock(hCpy)) != NULL))
	{
		MODINSTRUMENT *pIns = m_SndFile.Instruments[nIns];
		INSTRUMENTENVELOPE *pEnv = nullptr;

		UINT susBegin=0, susEnd=0, loopBegin=0, loopEnd=0, bSus=0, bLoop=0, bCarry=0, nPoints=0, releaseNode = ENV_RELEASE_NODE_UNSET;
		DWORD dwMemSize = GlobalSize(hCpy), dwPos = strlen(pszEnvHdr);
		if ((dwMemSize > dwPos) && (!_strnicmp(p, pszEnvHdr, dwPos-2)))
		{
			for (UINT h=0; h<8; h++)
			{
				while ((dwPos < dwMemSize) && ((p[dwPos] < '0') || (p[dwPos] > '9'))) dwPos++;
				if (dwPos >= dwMemSize) break;
				int n = atoi(p+dwPos);
				switch(h)
				{
				case 0:		nPoints = n; break;
				case 1:		susBegin = n; break;
				case 2:		susEnd = n; break;
				case 3:		loopBegin = n; break;
				case 4:		loopEnd = n; break;
				case 5:		bSus = n; break;
				case 6:		bLoop = n; break;
				case 7:		bCarry = n; break;
				}
				while ((dwPos < dwMemSize) && ((p[dwPos] >= '0') && (p[dwPos] <= '9'))) dwPos++;
			}
			nPoints = min(nPoints, m_SndFile.GetModSpecifications().envelopePointsMax);
			if (susEnd >= nPoints) susEnd = 0;
			if (susBegin > susEnd) susBegin = susEnd;
			if (loopEnd >= nPoints) loopEnd = 0;
			if (loopBegin > loopEnd) loopBegin = loopEnd;

			switch(nEnv)
			{
			case ENV_PANNING:
				pEnv = &pIns->PanEnv;
				pIns->dwFlags &= ~(ENV_PANLOOP|ENV_PANSUSTAIN|ENV_PANCARRY);
				if (bLoop) pIns->dwFlags |= ENV_PANLOOP;
				if (bSus) pIns->dwFlags |= ENV_PANSUSTAIN;
				if (bCarry) pIns->dwFlags |= ENV_PANCARRY;
				break;

			case ENV_PITCH:
				pEnv = &pIns->PitchEnv;
				pIns->dwFlags &= ~(ENV_PITCHLOOP|ENV_PITCHSUSTAIN|ENV_PITCHCARRY);
				if (bLoop) pIns->dwFlags |= ENV_PITCHLOOP;
				if (bSus) pIns->dwFlags |= ENV_PITCHSUSTAIN;
				if (bCarry) pIns->dwFlags |= ENV_PITCHCARRY;
				break;

			default:
				pEnv = &pIns->VolEnv;
				pIns->dwFlags &= ~(ENV_VOLLOOP|ENV_VOLSUSTAIN|ENV_VOLCARRY);
				if (bLoop) pIns->dwFlags |= ENV_VOLLOOP;
				if (bSus) pIns->dwFlags |= ENV_VOLSUSTAIN;
				if (bCarry) pIns->dwFlags |= ENV_VOLCARRY;
				break;
			}
			pEnv->nNodes = nPoints;
			pEnv->nSustainStart = susBegin;
			pEnv->nSustainEnd = susEnd;
			pEnv->nLoopStart = loopBegin;
			pEnv->nLoopEnd = loopEnd;
			pEnv->nReleaseNode = releaseNode;

			int oldn = 0;
			for (UINT i=0; i<nPoints; i++)
			{
				while ((dwPos < dwMemSize) && ((p[dwPos] < '0') || (p[dwPos] > '9'))) dwPos++;
				if (dwPos >= dwMemSize) break;
				int n1 = atoi(p+dwPos);
				while ((dwPos < dwMemSize) && (p[dwPos] != ',')) dwPos++;
				while ((dwPos < dwMemSize) && ((p[dwPos] < '0') || (p[dwPos] > '9'))) dwPos++;
				if (dwPos >= dwMemSize) break;
				int n2 = atoi(p+dwPos);
				if ((n1 < oldn) || (n1 > 0x3FFF)) n1 = oldn+1;
				pEnv->Ticks[i] = (WORD)n1;
				pEnv->Values[i] = (BYTE)n2;
				oldn = n1;
				while ((dwPos < dwMemSize) && (p[dwPos] != 0x0D)) dwPos++;
				if (dwPos >= dwMemSize) break;
			}

			//Read releasenode information.
			if(dwPos < dwMemSize)
			{
				BYTE r = static_cast<BYTE>(atoi(p + dwPos));
				if(r == 0 || r >= nPoints) r = ENV_RELEASE_NODE_UNSET;
				pEnv->nReleaseNode = r;
			}
		}
		GlobalUnlock(hCpy);
		CloseClipboard();
		SetModified();
		UpdateAllViews(NULL, (nIns << HINT_SHIFT_INS) | HINT_ENVELOPE, NULL);
	}
	EndWaitCursor();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Undo Functions

BOOL CModDoc::ClearUndo()
//-----------------------
{
	for (UINT i=0; i<MAX_UNDO_LEVEL; i++)
	{
		if (PatternUndo[i].pbuffer) delete[] PatternUndo[i].pbuffer;
		PatternUndo[i].cx = 0;
		PatternUndo[i].cy = 0;
		PatternUndo[i].pbuffer = NULL;
	}
	return TRUE;
}


BOOL CModDoc::CanUndo()
//---------------------
{
	return (PatternUndo[0].pbuffer) ? TRUE : FALSE;
}


BOOL CModDoc::PrepareUndo(UINT pattern, UINT x, UINT y, UINT cx, UINT cy)
//-----------------------------------------------------------------------
{
	MODCOMMAND *pUndo, *pPattern;
	UINT nRows;
	BOOL bUpdate;

	if ((pattern >= m_SndFile.Patterns.Size()) || (!m_SndFile.Patterns[pattern])) return FALSE;
	nRows = m_SndFile.PatternSize[pattern];
	pPattern = m_SndFile.Patterns[pattern];
	if ((y >= nRows) || (cx < 1) || (cy < 1) || (x >= m_SndFile.m_nChannels)) return FALSE;
	if (y+cy >= nRows) cy = nRows-y;
	if (x+cx >= m_SndFile.m_nChannels) cx = m_SndFile.m_nChannels - x;
	BeginWaitCursor();
	pUndo = new MODCOMMAND[cx*cy];
	if (!pUndo)
	{
		EndWaitCursor();
		return FALSE;
	}
	bUpdate = (PatternUndo[0].pbuffer) ? FALSE : TRUE;
	if (PatternUndo[MAX_UNDO_LEVEL-1].pbuffer)
	{
		delete[] PatternUndo[MAX_UNDO_LEVEL-1].pbuffer;
		PatternUndo[MAX_UNDO_LEVEL-1].pbuffer = NULL;
	}
	for (UINT i=MAX_UNDO_LEVEL-1; i>=1; i--)
	{
		PatternUndo[i] = PatternUndo[i-1];
	}
	PatternUndo[0].pattern = pattern;
	PatternUndo[0].patternsize = m_SndFile.PatternSize[pattern];
	PatternUndo[0].column = x;
	PatternUndo[0].row = y;
	PatternUndo[0].cx = cx;
	PatternUndo[0].cy = cy;
	PatternUndo[0].pbuffer = pUndo;
	pPattern += x + y*m_SndFile.m_nChannels;
	for (UINT iy=0; iy<cy; iy++)
	{
		memcpy(pUndo, pPattern, cx*sizeof(MODCOMMAND));
		pUndo += cx;
		pPattern += m_SndFile.m_nChannels;
	}
	EndWaitCursor();
	if (bUpdate) UpdateAllViews(NULL, HINT_UNDO);
	return TRUE;
}


UINT CModDoc::DoUndo()
//--------------------
{
	MODCOMMAND *pUndo, *pPattern;
	UINT nPattern, nRows;

	if ((!PatternUndo[0].pbuffer) || (PatternUndo[0].pattern >= m_SndFile.Patterns.Size())) return (UINT)-1;
	nPattern = PatternUndo[0].pattern;
	nRows = PatternUndo[0].patternsize;
	if (PatternUndo[0].column + PatternUndo[0].cx <= m_SndFile.m_nChannels)
	{
		if ((!m_SndFile.Patterns[nPattern]) || (m_SndFile.PatternSize[nPattern] < nRows))
		{
			MODCOMMAND *newPattern = CSoundFile::AllocatePattern(nRows, m_SndFile.m_nChannels);
			MODCOMMAND *oldPattern = m_SndFile.Patterns[nPattern];
			if (!newPattern) return (UINT)-1;
			const ROWINDEX nOldRowCount = m_SndFile.Patterns[nPattern].GetNumRows();
			m_SndFile.Patterns[nPattern].SetData(newPattern, nRows);
			if (oldPattern)
			{
				memcpy(newPattern, oldPattern, m_SndFile.m_nChannels*nOldRowCount*sizeof(MODCOMMAND));
				CSoundFile::FreePattern(oldPattern);
			}
		}
		pUndo = PatternUndo[0].pbuffer;
		pPattern = m_SndFile.Patterns[nPattern];
		if (!m_SndFile.Patterns[nPattern]) return (UINT)-1;
		pPattern += PatternUndo[0].column + (PatternUndo[0].row * m_SndFile.m_nChannels);
		for (UINT iy=0; iy<PatternUndo[0].cy; iy++)
		{
			memcpy(pPattern, pUndo, PatternUndo[0].cx * sizeof(MODCOMMAND));
			pPattern += m_SndFile.m_nChannels;
			pUndo += PatternUndo[0].cx;
		}
	}		
	delete[] PatternUndo[0].pbuffer;
	for (UINT i=0; i<MAX_UNDO_LEVEL-1; i++)
	{
		PatternUndo[i] = PatternUndo[i+1];
	}
	PatternUndo[MAX_UNDO_LEVEL-1].pbuffer = NULL;
	if (!PatternUndo[0].pbuffer) UpdateAllViews(NULL, HINT_UNDO);
	return nPattern;
}


void CModDoc::CheckUnusedChannels(BOOL mask[MAX_CHANNELS], CHANNELINDEX maxRemoveCount)
//--------------------------------------------------------
{
	// Checking for unused channels
	for (int iRst=m_SndFile.m_nChannels-1; iRst>=0; iRst--) //rewbs.removeChanWindowCleanup
	{
		mask[iRst] = TRUE;
		for (UINT ipat=0; ipat<m_SndFile.Patterns.Size(); ipat++) if (m_SndFile.Patterns[ipat])
		{
			MODCOMMAND *p = m_SndFile.Patterns[ipat] + iRst;
			UINT len = m_SndFile.PatternSize[ipat];
			for (UINT idata=0; idata<len; idata++, p+=m_SndFile.m_nChannels)
			{
				if (*((LPDWORD)p))
				{
					mask[iRst] = FALSE;
					break;
				}
			}
			if (!mask[iRst]) break;
		}
		if (mask[iRst])
		{
			if ((--maxRemoveCount) == 0) break;
		}
	}
}


