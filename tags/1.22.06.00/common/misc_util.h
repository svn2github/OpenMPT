/*
 * misc_util.h
 * -----------
 * Purpose: Various useful utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <limits>
#include <sstream>
#include <string>

#include <cmath>
#include <cstdlib>

#include <string.h>

#include "typedefs.h"

#if defined(HAS_TYPE_TRAITS)
#include <type_traits>
#endif

//Convert object(typically number) to string
template<class T>
inline std::string Stringify(const T& x)
//--------------------------------------
{
	std::ostringstream o;
	if(!(o << x)) return "FAILURE";
	else return o.str();
}

template<> inline std::string Stringify(const signed char& x) { return Stringify((signed int)x); }
template<> inline std::string Stringify(const unsigned char& x) { return Stringify((unsigned int)x); }

//Convert string to number.
template<class T>
inline T ConvertStrTo(const char *str)
//------------------------------------
{
	#ifdef HAS_TYPE_TRAITS
		static_assert(std::is_const<T>::value == false && std::is_volatile<T>::value == false, "Const and volatile types are not handled correctly.");
	#endif
	if(std::numeric_limits<T>::is_integer)
		return static_cast<T>(atoi(str));
	else
		return static_cast<T>(atof(str));
}

#if MPT_COMPILER_MSVC
#define cxx11_strtoll  _strtoi64
#define cxx11_strtoull _strtoui64
#else
#define cxx11_strtoll  std::strtoll
#define cxx11_strtoull std::strtoull
#endif
template<> inline signed int       ConvertStrTo(const char *str) {return (signed int)std::strtol(str, nullptr, 10);}
template<> inline signed long      ConvertStrTo(const char *str) {return std::strtol(str, nullptr, 10);}
template<> inline signed long long ConvertStrTo(const char *str) {return cxx11_strtoll(str, nullptr, 10);}
template<> inline unsigned int       ConvertStrTo(const char *str) {return (unsigned int)std::strtoul(str, nullptr, 10);}
template<> inline unsigned long      ConvertStrTo(const char *str) {return std::strtoul(str, nullptr, 10);}
template<> inline unsigned long long ConvertStrTo(const char *str) {return cxx11_strtoull(str, nullptr, 10);}


// Memset given object to zero.
template <class T>
inline void MemsetZero(T &a)
//--------------------------
{
#ifdef HAS_TYPE_TRAITS
	static_assert(std::is_pointer<T>::value == false, "Won't memset pointers.");
	static_assert(std::is_pod<T>::value == true, "Won't memset non-pods.");
#endif
	memset(&a, 0, sizeof(T));
}


// Copy given object to other location.
template <class T>
inline T &MemCopy(T &destination, const T &source)
//------------------------------------------------
{
#ifdef HAS_TYPE_TRAITS
	static_assert(std::is_pointer<T>::value == false, "Won't copy pointers.");
	static_assert(std::is_pod<T>::value == true, "Won't copy non-pods.");
#endif
	return *static_cast<T *>(memcpy(&destination, &source, sizeof(T)));
}


namespace mpt {

// Saturate the value of src to the domain of Tdst
template <typename Tdst, typename Tsrc>
inline Tdst saturate_cast(Tsrc src)
//---------------------------------
{
	// This code tries not only to obviously avoid overflows but also to avoid signed/unsigned comparison warnings and type truncation warnings (which in fact would be safe here) by explicit casting.
	STATIC_ASSERT(std::numeric_limits<Tdst>::is_integer);
	STATIC_ASSERT(std::numeric_limits<Tsrc>::is_integer);
	if(std::numeric_limits<Tdst>::is_signed && std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(src);
		}
		return static_cast<Tdst>(std::max<Tsrc>(std::numeric_limits<Tdst>::min(), std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	} else if(!std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(src);
		}
		return static_cast<Tdst>(std::min<Tsrc>(src, std::numeric_limits<Tdst>::max()));
	} else if(std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed)
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(std::min<Tsrc>(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max())));
		}
		return static_cast<Tdst>(std::max<Tsrc>(std::numeric_limits<Tdst>::min(), std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	} else // Tdst unsigned, Tsrc signed
	{
		if(sizeof(Tdst) >= sizeof(Tsrc))
		{
			return static_cast<Tdst>(std::max<Tsrc>(0, src));
		}
		return static_cast<Tdst>(std::max<Tsrc>(0, std::min<Tsrc>(src, std::numeric_limits<Tdst>::max())));
	}
}

template <typename Tdst>
inline Tdst saturate_cast(double src)
//-----------------------------------
{
	if(src >= std::numeric_limits<Tdst>::max())
	{
		return std::numeric_limits<Tdst>::max();
	}
	if(src <= std::numeric_limits<Tdst>::min())
	{
		return std::numeric_limits<Tdst>::min();
	}
	return static_cast<Tdst>(src);
}

template <typename Tdst>
inline Tdst saturate_cast(float src)
//----------------------------------
{
	if(src >= std::numeric_limits<Tdst>::max())
	{
		return std::numeric_limits<Tdst>::max();
	}
	if(src <= std::numeric_limits<Tdst>::min())
	{
		return std::numeric_limits<Tdst>::min();
	}
	return static_cast<Tdst>(src);
}

} // namespace mpt


// Limits 'val' to given range. If 'val' is less than 'lowerLimit', 'val' is set to value 'lowerLimit'.
// Similarly if 'val' is greater than 'upperLimit', 'val' is set to value 'upperLimit'.
// If 'lowerLimit' > 'upperLimit', 'val' won't be modified.
template<class T, class C>
inline void Limit(T& val, const C lowerLimit, const C upperLimit)
//---------------------------------------------------------------
{
	if(lowerLimit > upperLimit) return;
	if(val < lowerLimit) val = lowerLimit;
	else if(val > upperLimit) val = upperLimit;
}


// Like Limit, but returns value
template<class T, class C>
inline T Clamp(T val, const C lowerLimit, const C upperLimit)
//-----------------------------------------------------------
{
	if(val < lowerLimit) return lowerLimit;
	else if(val > upperLimit) return upperLimit;
	else return val;
}


// Like Limit, but with upperlimit only.
template<class T, class C>
inline void LimitMax(T& val, const C upperLimit)
//----------------------------------------------
{
	if(val > upperLimit)
		val = upperLimit;
}


// Like Limit, but returns value
#ifndef CLAMP
#define CLAMP(number, low, high) MIN(high, MAX(low, number))
#endif


static inline char SanitizeFilenameChar(char c)
//---------------------------------------------
{
	if(	c == '\\' ||
		c == '\"' ||
		c == '/'  ||
		c == ':'  ||
		c == '?'  ||
		c == '<'  ||
		c == '>'  ||
		c == '*')
	{
		c = '_';
	}
	return c;
}

static inline wchar_t SanitizeFilenameChar(wchar_t c)
//---------------------------------------------------
{
	if(	c == L'\\' ||
		c == L'\"' ||
		c == L'/'  ||
		c == L':'  ||
		c == L'?'  ||
		c == L'<'  ||
		c == L'>'  ||
		c == L'*')
	{
		c = L'_';
	}
	return c;
}

// Sanitize a filename (remove special chars)
template <size_t size>
void SanitizeFilename(char (&buffer)[size])
//-----------------------------------------
{
	STATIC_ASSERT(size > 0);
	for(size_t i = 0; i < size; i++)
	{
		buffer[i] = SanitizeFilenameChar(buffer[i]);
	}
}
template <size_t size>
void SanitizeFilename(wchar_t (&buffer)[size])
//--------------------------------------------
{
	STATIC_ASSERT(size > 0);
	for(size_t i = 0; i < size; i++)
	{
		buffer[i] = SanitizeFilenameChar(buffer[i]);
	}
}
static inline void SanitizeFilename(std::string &str)
//---------------------------------------------------
{
	for(size_t i = 0; i < str.length(); i++)
	{
		str[i] = SanitizeFilenameChar(str[i]);
	}
}
static inline void SanitizeFilename(std::wstring &str)
//----------------------------------------------------
{
	for(size_t i = 0; i < str.length(); i++)
	{
		str[i] = SanitizeFilenameChar(str[i]);
	}
}
#ifdef _MFC_VER
static inline void SanitizeFilename(CString &str)
//-----------------------------------------------
{
	std::basic_string<TCHAR> tmp = str;
	SanitizeFilename(tmp);
	str = tmp.c_str();
}
#endif


// Greatest Common Divisor.
template <class T>
T gcd(T a, T b)
//-------------
{
	if(a < 0)
		a = -a;
	if(b < 0)
		b = -b;
	do
	{
		if(a == 0)
			return b;
		b %= a;
		if(b == 0)
			return a;
		a %= b;
	} while(true);
}


// Returns sign of a number (-1 for negative numbers, 1 for positive numbers, 0 for 0)
template <class T>
int sgn(T value)
//--------------
{
	return (value > T(0)) - (value < T(0));
}


// Convert a variable-length MIDI integer <value> to a normal integer <result>.
// Function returns how many bytes have been read.
template <class TIn, class TOut>
size_t ConvertMIDI2Int(TOut &result, TIn value)
//---------------------------------------------
{
	return ConvertMIDI2Int(result, (uint8 *)(&value), sizeof(TIn));
}


// Convert a variable-length MIDI integer held in the byte buffer <value> to a normal integer <result>.
// maxLength bytes are read from the byte buffer at max.
// Function returns how many bytes have been read.
// TODO This should report overflow errors if TOut is too small!
template <class TOut>
size_t ConvertMIDI2Int(TOut &result, uint8 *value, size_t maxLength)
//------------------------------------------------------------------
{
	static_assert(std::numeric_limits<TOut>::is_integer == true, "Output type is a not an integer");

	if(maxLength <= 0)
	{
		result = 0;
		return 0;
	}
	size_t bytesUsed = 0;
	result = 0;
	uint8 b;
	do
	{
		b = *value;
		result <<= 7;
		result |= (b & 0x7F);
		value++;
	} while (++bytesUsed < maxLength && (b & 0x80) != 0);
	return bytesUsed;
}


// Convert an integer <value> to a variable-length MIDI integer, held in the byte buffer <result>.
// maxLength bytes are written to the byte buffer at max.
// Function returns how many bytes have been written.
template <class TIn>
size_t ConvertInt2MIDI(uint8 *result, size_t maxLength, TIn value)
//----------------------------------------------------------------
{
	static_assert(std::numeric_limits<TIn>::is_integer == true, "Input type is a not an integer");

	if(maxLength <= 0)
	{
		*result = 0;
		return 0;
	}
	size_t bytesUsed = 0;
	do
	{
		*result = (value & 0x7F);
		value >>= 7;
		if(value != 0)
		{
			*result |= 0x80;
		}
		result++;
	} while (++bytesUsed < maxLength && value != 0);
	return bytesUsed;
}


namespace Util
{

	// Minimum of 3 values
	template <class T> inline const T& Min(const T& a, const T& b, const T& c) {return std::min(std::min(a, b), c);}

	// Returns maximum value of given integer type.
	template <class T> inline T MaxValueOfType(const T&) {static_assert(std::numeric_limits<T>::is_integer == true, "Only integer types are allowed."); return (std::numeric_limits<T>::max)();}

	/// Returns value rounded to nearest integer.
#if MPT_COMPILER_MSVC && MPT_MSVC_BEFORE(2012,0)
	// revisit this with vs2012
	inline double Round(const double& val) {if(val >= 0.0) return std::floor(val + 0.5); else return std::ceil(val - 0.5);}
	inline float Round(const float& val) {if(val >= 0.0f) return std::floor(val + 0.5f); else return std::ceil(val - 0.5f);}
#else
	inline double Round(const double& val) { return std::round(val); }
	inline float Round(const float& val) { return std::round(val); }
#endif

	/// Rounds given double value to nearest integer value of type T.
	template <class T> inline T Round(const double& val)
	{
		static_assert(std::numeric_limits<T>::is_integer == true, "Type is a not an integer");
		const double valRounded = Round(val);
		ASSERT(valRounded >= (std::numeric_limits<T>::min)() && valRounded <= (std::numeric_limits<T>::max)());
		const T intval = static_cast<T>(valRounded);
		return intval;
	}

	template <class T> inline T Round(const float& val)
	{
		static_assert(std::numeric_limits<T>::is_integer == true, "Type is a not an integer");
		const float valRounded = Round(val);
		ASSERT(valRounded >= (std::numeric_limits<T>::min)() && valRounded <= (std::numeric_limits<T>::max)());
		const T intval = static_cast<T>(valRounded);
		return intval;
	}
	
}

namespace Util {

	inline int32 muldiv(int32 a, int32 b, int32 c)
	{
		return static_cast<int32>( ( static_cast<int64>(a) * b ) / c );
	}

	inline int32 muldivr(int32 a, int32 b, int32 c)
	{
		return static_cast<int32>( ( static_cast<int64>(a) * b + ( c / 2 ) ) / c );
	}

	// Do not use overloading because catching unsigned version by accident results in slower X86 code.
	inline uint32 muldiv_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return static_cast<uint32>( ( static_cast<uint64>(a) * b ) / c );
	}
	inline uint32 muldivr_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return static_cast<uint32>( ( static_cast<uint64>(a) * b + ( c / 2 ) ) / c );
	}

	inline int32 muldivrfloor(int64 a, uint32 b, uint32 c)
	{
		a *= b;
		a += c / 2;
		return (a >= 0) ? mpt::saturate_cast<int32>(a / c) : mpt::saturate_cast<int32>((a - (c - 1)) / c);
	}

	template<typename T, std::size_t n>
	class fixed_size_queue
	{
	private:
		T buffer[n+1];
		std::size_t read_position;
		std::size_t write_position;
	public:
		fixed_size_queue() : read_position(0), write_position(0)
		{
			return;
		}
		void clear()
		{
			read_position = 0;
			write_position = 0;
		}
		std::size_t read_size() const
		{
			if ( write_position > read_position )
			{
				return write_position - read_position;
			} else if ( write_position < read_position )
			{
				return write_position - read_position + n + 1;
			} else
			{
				return 0;
			}
		}
		std::size_t write_size() const
		{
			if ( write_position > read_position )
			{
				return read_position - write_position + n;
			} else if ( write_position < read_position )
			{
				return read_position - write_position - 1;
			} else
			{
				return n;
			}
		}
		bool push( const T & v )
		{
			if ( !write_size() )
			{
				return false;
			}
			buffer[write_position] = v;
			write_position = ( write_position + 1 ) % ( n + 1 );
			return true;
		}
		bool pop() {
			if ( !read_size() )
			{
				return false;
			}
			read_position = ( read_position + 1 ) % ( n + 1 );
			return true;
		}
		T peek() {
			if ( !read_size() )
			{
				return T();
			}
			return buffer[read_position];
		}
		const T * peek_p()
		{
			if ( !read_size() )
			{
				return nullptr;
			}
			return &(buffer[read_position]);
		}
		const T * peek_next_p()
		{
			if ( read_size() < 2 )
			{
				return nullptr;
			}
			return &(buffer[(read_position+1)%(n+1)]);
		}
	};

} // namespace Util


#ifdef ENABLE_ASM
#define PROCSUPPORT_MMX        0x00001 // Processor supports MMX instructions
#define PROCSUPPORT_SSE        0x00010 // Processor supports SSE instructions
#define PROCSUPPORT_SSE2       0x00020 // Processor supports SSE2 instructions
#define PROCSUPPORT_SSE3       0x00040 // Processor supports SSE3 instructions
#define PROCSUPPORT_AMD_MMXEXT 0x10000 // Processor supports AMD MMX extensions
#define PROCSUPPORT_AMD_3DNOW  0x20000 // Processor supports AMD 3DNow! instructions
#define PROCSUPPORT_AMD_3DNOW2 0x40000 // Processor supports AMD 3DNow!2 instructions
extern uint32 ProcSupport;
void InitProcSupport();
static inline uint32 GetProcSupport()
{
	return ProcSupport;
}
#endif // ENABLE_ASM

