/*
 * mptTypeTraits.h
 * ---------------
 * Purpose: C++11 similar type_traits header plus some OpenMPT specific traits.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <type_traits>


OPENMPT_NAMESPACE_BEGIN


namespace mpt {


// Tell which types are safe for mpt::byte_cast.
// signed char is actually not allowed to alias into an object representation,
// which means that, if the actual type is not itself signed char but char or
// unsigned char instead, dereferencing the signed char pointer is undefined
// behaviour.
template <typename T> struct is_byte_castable : public std::false_type { };
template <> struct is_byte_castable<char>                : public std::true_type { };
template <> struct is_byte_castable<unsigned char>       : public std::true_type { };
#if MPT_BYTE_IS_STD_BYTE
template <> struct is_byte_castable<mpt::byte>           : public std::true_type { };
#endif
template <> struct is_byte_castable<const char>          : public std::true_type { };
template <> struct is_byte_castable<const unsigned char> : public std::true_type { };
#if MPT_BYTE_IS_STD_BYTE
template <> struct is_byte_castable<const mpt::byte>     : public std::true_type { };
#endif


template <typename T> struct is_byte        : public std::false_type { };
template <> struct is_byte<mpt::byte>       : public std::true_type  { };
template <> struct is_byte<const mpt::byte> : public std::true_type  { };


// Tell which types are safe to binary write into files.
// By default, no types are safe.
// When a safe type gets defined,
// also specialize this template so that IO functions will work.
template <typename T> struct is_binary_safe : public std::false_type { }; 

// Specialization for byte types.
template <> struct is_binary_safe<char>      : public std::true_type { };
template <> struct is_binary_safe<uint8>     : public std::true_type { };
template <> struct is_binary_safe<int8>      : public std::true_type { };
#if MPT_BYTE_IS_STD_BYTE
template <> struct is_binary_safe<mpt::byte> : public std::true_type { };
#endif

// Generic Specialization for arrays.
template <typename T, std::size_t N> struct is_binary_safe<T[N]> : public is_binary_safe<T> { };
template <typename T, std::size_t N> struct is_binary_safe<const T[N]> : public is_binary_safe<T> { };


} // namespace mpt

#define MPT_BINARY_STRUCT(type, size) \
	MPT_STATIC_ASSERT(sizeof( type ) == (size) ); \
	MPT_STATIC_ASSERT(alignof( type ) == 1); \
	MPT_STATIC_ASSERT(std::is_standard_layout< type >::value); \
	namespace mpt { \
		template <> struct is_binary_safe< type > : public std::true_type { }; \
	} \
/**/


OPENMPT_NAMESPACE_END
