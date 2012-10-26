/*
 * Undo.h
 * ------
 * Purpose: Editor undo buffer functionality.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#define MAX_UNDO_LEVEL 100000	// 100,000 undo steps for each undo type!

/////////////////////////////////////////////////////////////////////////////////////////
// Pattern Undo


//================
class CPatternUndo
//================
{
protected:

	// Additional undo information, as required
	struct ChannelInfo
	{
		ModChannelSettings *settings;
		CHANNELINDEX oldNumChannels;

		ChannelInfo(CHANNELINDEX numChannels) : oldNumChannels(numChannels)
		{
			settings = new ModChannelSettings[numChannels];
		}

		~ChannelInfo()
		{
			delete[] settings;
		}
	};

	struct UndoInfo
	{
		PATTERNINDEX pattern;
		ROWINDEX patternsize;
		CHANNELINDEX firstChannel, numChannels;
		ROWINDEX firstRow, numRows;
		ModCommand *pbuffer;
		ChannelInfo *channelInfo;
		bool linkToPrevious;
	};

	std::vector<UndoInfo> UndoBuffer;
	CModDoc *m_pModDoc;

	// Pattern undo helper functions
	void DeleteUndoStep(size_t step);
	PATTERNINDEX Undo(bool linkedFromPrevious);

public:

	// Pattern undo functions
	void ClearUndo();
	bool PrepareUndo(PATTERNINDEX pattern, CHANNELINDEX firstChn, ROWINDEX firstRow, CHANNELINDEX numChns, ROWINDEX numRows, bool linkToPrevious = false, bool storeChannelInfo = false);
	PATTERNINDEX Undo();
	bool CanUndo() const;
	void RemoveLastUndoStep();

	void SetParent(CModDoc *pModDoc) { m_pModDoc = pModDoc; }

	CPatternUndo()
	{
		UndoBuffer.clear();
		m_pModDoc = nullptr;
	};

	~CPatternUndo()
	{
		ClearUndo();
	};

};



/////////////////////////////////////////////////////////////////////////////////////////
// Sample Undo

// We will differentiate between different types of undo actions so that we don't have to copy the whole sample everytime.
enum sampleUndoTypes
{
	sundo_none,		// no changes to sample itself, e.g. loop point update
	sundo_update,	// silence, amplify, normalize, dc offset - update complete sample section
	sundo_delete,	// delete part of the sample
	sundo_invert,	// invert sample phase, apply again to undo
	sundo_reverse,	// reverse sample, dito
	sundo_unsign,	// unsign sample, dito
	sundo_insert,	// insert data, delete inserted data to undo
	sundo_replace,	// replace complete sample (16->8Bit, up/downsample, downmix to mono, pitch shifting / time stretching, trimming, pasting)
};


//===============
class CSampleUndo
//===============
{
protected:

	struct UndoInfo
	{
		ModSample OldSample;
		char szOldName[MAX_SAMPLENAME];
		LPSTR SamplePtr;
		SmpLength nChangeStart, nChangeEnd;
		sampleUndoTypes nChangeType;
	};

	// Undo buffer
	std::vector<std::vector<UndoInfo> > UndoBuffer;
	CModDoc *m_pModDoc;

	// Sample undo helper functions
	void DeleteUndoStep(const SAMPLEINDEX smp, const size_t step);
	bool SampleBufferExists(const SAMPLEINDEX smp, bool forceCreation = true);
	void RestrictBufferSize();
	size_t GetUndoBufferCapacity();

public:

	// Sample undo functions
	void ClearUndo();
	void ClearUndo(const SAMPLEINDEX smp);
	bool PrepareUndo(const SAMPLEINDEX smp, sampleUndoTypes changeType, SmpLength changeStart = 0, SmpLength changeEnd = 0);
	bool Undo(const SAMPLEINDEX smp);
	bool CanUndo(const SAMPLEINDEX smp);
	void RemoveLastUndoStep(const SAMPLEINDEX smp);

	void SetParent(CModDoc *pModDoc) { m_pModDoc = pModDoc; }

	CSampleUndo()
	{
		UndoBuffer.clear();
		m_pModDoc = nullptr;
	};

	~CSampleUndo()
	{
		ClearUndo();
	};

};