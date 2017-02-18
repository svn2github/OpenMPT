#ifndef ORDERTOPATTERNTABLE_H
#define ORDERTOPATTERNTABLE_H

#include "serialization_utils.h"
#include <vector>
using std::vector;

class CSoundFile;
class COrderToPatternTable;

class COrderSerialization : public srlztn::ABCSerializationStreamer
//=========================================================
{
public:
	COrderSerialization(COrderToPatternTable& ordertable) : m_rOrders(ordertable) {}
	virtual void ProWrite(srlztn::OUTSTREAM& ostrm) const;
	virtual void ProRead(srlztn::INSTREAM& istrm, const uint64 /*datasize*/);
private:
	COrderToPatternTable& m_rOrders;
};

//==============================================
class COrderToPatternTable : public vector<PATTERNINDEX>
//==============================================
{
public:
	COrderToPatternTable(const CSoundFile& sndFile) : m_rSndFile(sndFile) {}

	bool ReadAsByte(const BYTE* pFrom, const int howMany, const int memLength);

	size_t WriteAsByte(FILE* f, const UINT count);

	size_t WriteToByteArray(BYTE* dest, const UINT numOfBytes, const UINT destSize);

	//Deprecated function used for MPTm's created in 1.17.02.46 - 1.17.02.48.
	DWORD Unserialize(const BYTE* const src, const DWORD memLength);
	
	//Returns true if the IT orderlist datafield is not sufficient to store orderlist information.
	bool NeedsExtraDatafield() const;

	PATTERNINDEX GetInvalidPatIndex() const; //To correspond 0xFF
	PATTERNINDEX GetIgnoreIndex() const; //To correspond 0xFE

	COrderSerialization* NewReadWriteObject() {return new COrderSerialization(*this);}

private:
	const CSoundFile& m_rSndFile;
};



#endif
