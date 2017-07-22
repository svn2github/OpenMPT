/*
 * tuning.h
 * --------
 * Purpose: Alternative sample tuning.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <map>

#include "tuningbase.h"


OPENMPT_NAMESPACE_BEGIN


namespace Tuning {


//==============
class CTuningRTI
//==============
{

public:

	static const char s_FileExtension[5];

	static const TUNINGTYPE TT_GENERAL;
	static const TUNINGTYPE TT_GROUPGEOMETRIC;
	static const TUNINGTYPE TT_GEOMETRIC;

	static const RATIOTYPE s_DefaultFallbackRatio;
	static const NOTEINDEXTYPE s_StepMinDefault = -64;
	static const UNOTEINDEXTYPE s_RatioTableSizeDefault = 128;
	static const USTEPINDEXTYPE s_RatioTableFineSizeMaxDefault = 1000;

public:

	//To return ratio of certain note.
	RATIOTYPE GetRatio(const NOTEINDEXTYPE& stepsFromCentre) const;

	//To return ratio from a 'step'(noteindex + stepindex)
	RATIOTYPE GetRatio(const NOTEINDEXTYPE& stepsFromCentre, const STEPINDEXTYPE& fineSteps) const;

	UNOTEINDEXTYPE GetRatioTableSize() const {return static_cast<UNOTEINDEXTYPE>(m_RatioTable.size());}

	NOTEINDEXTYPE GetRatioTableBeginNote() const {return m_StepMin;}

	//Tuning might not be valid for arbitrarily large range,
	//so this can be used to ask where it is valid. Tells the lowest and highest
	//note that are valid.
	VRPAIR GetValidityRange() const {return VRPAIR(m_StepMin, static_cast<NOTEINDEXTYPE>(m_StepMin + static_cast<NOTEINDEXTYPE>(m_RatioTable.size()) - 1));}

	//Return true if note is within validity range - false otherwise.
	bool IsValidNote(const NOTEINDEXTYPE n) const {return (n >= GetValidityRange().first && n <= GetValidityRange().second);}

	//Checking that step distances can be presented with
	//value range of STEPINDEXTYPE with given finestepcount and validityrange.
	bool IsStepCountRangeSufficient(USTEPINDEXTYPE fs, VRPAIR vrp);

	UNOTEINDEXTYPE GetGroupSize() const {return m_GroupSize;}

	RATIOTYPE GetGroupRatio() const {return m_GroupRatio;}

	//To return (fine)stepcount between two consecutive mainsteps.
	USTEPINDEXTYPE GetFineStepCount() const {return m_FineStepCount;}

	//To return 'directed distance' between given notes.
	STEPINDEXTYPE GetStepDistance(const NOTEINDEXTYPE& from, const NOTEINDEXTYPE& to) const
		{return (to - from)*(static_cast<NOTEINDEXTYPE>(GetFineStepCount())+1);}

	//To return 'directed distance' between given steps.
	STEPINDEXTYPE GetStepDistance(const NOTEINDEXTYPE& noteFrom, const STEPINDEXTYPE& stepDistFrom, const NOTEINDEXTYPE& noteTo, const STEPINDEXTYPE& stepDistTo) const
		{return GetStepDistance(noteFrom, noteTo) + stepDistTo - stepDistFrom;}

	//To set finestepcount between two consecutive mainsteps and
	//return GetFineStepCount(). This might not be the same as
	//parameter fs if something fails. Finestep count == 0 means that
	//stepdistances become the same as note distances.
	USTEPINDEXTYPE SetFineStepCount(const USTEPINDEXTYPE& fs);

	//Multiply all ratios by given number.
	bool Multiply(const RATIOTYPE&);

	bool SetRatio(const NOTEINDEXTYPE& s, const RATIOTYPE& r);

	TUNINGTYPE GetType() const {return m_TuningType;}

	std::string GetNoteName(const NOTEINDEXTYPE& x, bool addOctave = true) const;

	bool SetNoteName(const NOTEINDEXTYPE&, const std::string&);

	bool ClearNoteName(const NOTEINDEXTYPE& n, const bool clearAll = false);

	static CTuningRTI* Deserialize(std::istream& inStrm);

	//Try to read old version (v.3) and return pointer to new instance if succesfull, else nullptr.
	static CTuningRTI* DeserializeOLD(std::istream&);

	Tuning::SerializationResult Serialize(std::ostream& out) const;

#ifdef MODPLUG_TRACKER
	bool WriteSCL(std::ostream &f, const mpt::PathString &filename) const;
#endif

	bool UpdateRatioGroupGeometric(NOTEINDEXTYPE s, RATIOTYPE r);

	//Create GroupGeometric tuning of *this using virtual ProCreateGroupGeometric.
	bool CreateGroupGeometric(const std::vector<RATIOTYPE>&, const RATIOTYPE&, const VRPAIR vr, const NOTEINDEXTYPE ratiostartpos);

	//Create GroupGeometric of *this using ratios from 'itself' and ratios starting from
	//position given as third argument.
	bool CreateGroupGeometric(const NOTEINDEXTYPE&, const RATIOTYPE&, const NOTEINDEXTYPE&);

	//Create geometric tuning of *this using ratio(0) = 1.
	bool CreateGeometric(const UNOTEINDEXTYPE& p, const RATIOTYPE& r) {return CreateGeometric(p,r,GetValidityRange());}
	bool CreateGeometric(const UNOTEINDEXTYPE&, const RATIOTYPE&, const VRPAIR vr);

	bool ChangeGroupsize(const NOTEINDEXTYPE&);
	bool ChangeGroupRatio(const RATIOTYPE&);

	void SetName(const std::string& s) { m_TuningName = s; }
	std::string GetName() const {return m_TuningName;}

public:

	CTuningRTI();

private:

	//Return value: true if change was not done, and false otherwise, in case which
	//tuningtype is automatically changed to general.
	bool ProSetRatio(const NOTEINDEXTYPE&, const RATIOTYPE&);

	//The two methods below return false if action was done, true otherwise.
	bool ProCreateGroupGeometric(const std::vector<RATIOTYPE>&, const RATIOTYPE&, const VRPAIR&, const NOTEINDEXTYPE ratiostartpos);
	bool ProCreateGeometric(const UNOTEINDEXTYPE&, const RATIOTYPE&, const VRPAIR&);

	void ProSetFineStepCount(const USTEPINDEXTYPE&);

	std::string ProGetNoteName(const NOTEINDEXTYPE& xi, bool addOctave) const;

	//Note: Groupsize is restricted to interval [0, NOTEINDEXTYPE_MAX]
	NOTEINDEXTYPE ProSetGroupSize(const UNOTEINDEXTYPE& p) {return m_GroupSize = (p<=static_cast<UNOTEINDEXTYPE>(NOTEINDEXTYPE_MAX)) ? static_cast<NOTEINDEXTYPE>(p) : NOTEINDEXTYPE_MAX;}
	RATIOTYPE ProSetGroupRatio(const RATIOTYPE& pr) {return m_GroupRatio = (pr >= 0) ? pr : -pr;}

	//Return true if data loading failed, false otherwise.
	bool ProProcessUnserializationdata(UNOTEINDEXTYPE ratiotableSize);

	//GroupGeometric.
	bool CreateRatioTableGG(const std::vector<RATIOTYPE>&, const RATIOTYPE, const VRPAIR& vr, const NOTEINDEXTYPE ratiostartpos);

	//Note: Stepdiff should be in range [1, finestepcount]
	RATIOTYPE GetRatioFine(const NOTEINDEXTYPE& note, USTEPINDEXTYPE stepDiff) const;

	//GroupPeriodic-specific.
	//Get the corresponding note in [0, period-1].
	//For example GetRefNote(-1) is to return note :'groupsize-1'.
	NOTEINDEXTYPE GetRefNote(NOTEINDEXTYPE note) const;

	bool IsNoteInTable(const NOTEINDEXTYPE& s) const
	{
		if(s < m_StepMin || s >= m_StepMin + static_cast<NOTEINDEXTYPE>(m_RatioTable.size()))
			return false;
		else
			return true;
	}

private:

	TUNINGTYPE m_TuningType;

	//Noteratios
	std::vector<RATIOTYPE> m_RatioTable;

	//'Fineratios'
	std::vector<RATIOTYPE> m_RatioTableFine;

	//The lowest index of note in the table
	NOTEINDEXTYPE m_StepMin; // this should REALLY be called 'm_NoteMin' renaming was missed in r192

	//For groupgeometric tunings, tells the 'group size' and 'group ratio'
	//m_GroupSize should always be >= 0.
	NOTEINDEXTYPE m_GroupSize;
	RATIOTYPE m_GroupRatio;

	USTEPINDEXTYPE m_FineStepCount;

	std::string m_TuningName;

	std::map<NOTEINDEXTYPE, std::string> m_NoteNameMap;

}; // class CTuningRTI


typedef CTuningRTI CTuning;


} // namespace Tuning


OPENMPT_NAMESPACE_END
