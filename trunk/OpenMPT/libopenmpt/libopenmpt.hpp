/*
 * libopenmpt.hpp
 * --------------
 * Purpose: libopenmpt public c++ interface
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_HPP
#define LIBOPENMPT_HPP

#include "libopenmpt_config.h"

#include <exception>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include <cstdint>

namespace openmpt {

class LIBOPENMPT_CXX_API exception : public std::exception {
public:
	exception( const char * text ) throw();
	virtual ~exception() throw();
public:
	virtual const char * what() const throw();
private:
	const char * const text;
}; // class exception

LIBOPENMPT_CXX_API std::uint32_t get_library_version();

LIBOPENMPT_CXX_API std::uint32_t get_core_version();

namespace detail {

LIBOPENMPT_CXX_API void version_compatible_or_throw( std::int32_t api_version );

class api_version_checker {
public:
	inline api_version_checker( std::int32_t api_version = OPENMPT_API_VERSION ) {
		version_compatible_or_throw( api_version );
	}
}; // class api_version_checker

} // namespace detail

namespace string {

static const char * const library_version = "library_version";
static const char * const core_version    = "core_version";
static const char * const build           = "build";
static const char * const credits         = "credits";
static const char * const contact         = "contact";

LIBOPENMPT_CXX_API std::string get( const std::string & key );

} // namespace string

LIBOPENMPT_CXX_API std::vector<std::string> get_supported_extensions();

LIBOPENMPT_CXX_API bool is_extension_supported( const std::string & extension );

LIBOPENMPT_CXX_API double could_open_propability( std::istream & stream, double effort = 1.0, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );

class module_impl;

class interactive_module;

class LIBOPENMPT_CXX_API module {

	friend class interactive_module;

public:

	enum render_param {
		RENDER_MASTERGAIN_DB            = 1,
		RENDER_STEREOSEPARATION_PERCENT = 2,
		RENDER_REPEATCOUNT              = 3,
		RENDER_QUALITY_PERCENT          = 4,
		RENDER_MAXMIXCHANNELS           = 5,
		RENDER_INTERPOLATION_MODE       = 6,
		RENDER_VOLUMERAMP_IN_SAMPLES    = 7,
		RENDER_VOLUMERAMP_OUT_SAMPLES   = 8,
	};

	enum interpolation_mode {
		INTERPOLATION_NEAREST           = 1,
		INTERPOLATION_LINEAR            = 2,
		INTERPOLATION_SPLINE            = 3,
		INTERPOLATION_POLYPHASE         = 4,
		INTERPOLATION_FIR_HANN          = 5,
		INTERPOLATION_FIR_HAMMING       = 6,
		INTERPOLATION_FIR_BLACKMANEXACT = 7,
		INTERPOLATION_FIR_BLACKMAN3T61  = 8,
		INTERPOLATION_FIR_BLACKMAN3T67  = 9,
		INTERPOLATION_FIR_BLACKMAN4T92  = 10,
		INTERPOLATION_FIR_BLACKMAN4T74  = 11,
		INTERPOLATION_FIR_KAISER4T      = 12,
	};

	enum command_index {
		command_note        = 0,
		command_instrument  = 1,
		command_volumeffect = 2,
		command_effect      = 3,
		command_volume      = 4,
		command_parameter   = 5,
	};

private:
	module_impl * impl;
private:
	// non-copyable
	module( const module & );
	void operator = ( const module & );
private:
	// for interactive_module
	module();
	void set_impl( module_impl * i );
public:
	module( std::istream & stream, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const std::vector<std::uint8_t> & data, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const std::uint8_t * beg, const std::uint8_t * end, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const std::uint8_t * data, std::size_t size, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const std::vector<char> & data, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const char * beg, const char * end, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const char * data, std::size_t size, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	module( const void * data, std::size_t size, std::ostream & log = std::clog, const detail::api_version_checker & apicheck = detail::api_version_checker() );
	virtual ~module();
public:

	std::int32_t get_render_param( int command ) const;
	void set_render_param( int command, std::int32_t value );

	void select_subsong( std::int32_t subsong );
 
	double seek_seconds( double seconds );

	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * mono );
	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * left, std::int16_t * right );
	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * left, std::int16_t * right, std::int16_t * back_left, std::int16_t * back_right );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * mono );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * left, float * right );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * left, float * right, float * back_left, float * back_right );

	double get_current_position_seconds() const;

	double get_duration_seconds() const;

	std::vector<std::string> get_metadata_keys() const;
	std::string get_metadata( const std::string & key ) const;

	std::int32_t get_current_speed() const;
	std::int32_t get_current_tempo() const;
	std::int32_t get_current_order() const;
	std::int32_t get_current_pattern() const;
	std::int32_t get_current_row() const;
	std::int32_t get_current_playing_channels() const;

	std::int32_t get_num_subsongs() const;
	std::int32_t get_num_channels() const;
	std::int32_t get_num_orders() const;
	std::int32_t get_num_patterns() const;
	std::int32_t get_num_instruments() const;
	std::int32_t get_num_samples() const;

	std::vector<std::string> get_subsong_names() const;
	std::vector<std::string> get_channel_names() const;
	std::vector<std::string> get_order_names() const;
	std::vector<std::string> get_pattern_names() const;
	std::vector<std::string> get_instrument_names() const;
	std::vector<std::string> get_sample_names() const;

	std::int32_t get_order_pattern( std::int32_t order ) const;

	std::int32_t get_pattern_num_rows( std::int32_t pattern ) const;

	std::uint8_t get_pattern_row_channel_command( std::int32_t pattern, std::int32_t row, std::int32_t channel, int command ) const;

	// remember to add new functions to both C and C++ interfaces and to increase OPENMPT_API_VERSION_MINOR

}; // class module

} // namespace openmpt

#endif // LIBOPENMPT_HPP
