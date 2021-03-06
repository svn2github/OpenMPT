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
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include <cstdint>

namespace openmpt {

class LIBOPENMPT_CXX_API exception : public std::exception {
private:
	char * text;
public:
	exception( const std::string & text ) throw();
	virtual ~exception() throw();
	virtual const char * what() const throw();
}; // class exception

LIBOPENMPT_CXX_API std::uint32_t get_library_version();

LIBOPENMPT_CXX_API std::uint32_t get_core_version();

namespace string {

static const char library_version[] = "library_version";
static const char core_version   [] = "core_version";
static const char build          [] = "build";
static const char credits        [] = "credits";
static const char contact        [] = "contact";

LIBOPENMPT_CXX_API std::string get( const std::string & key );

} // namespace string

LIBOPENMPT_CXX_API std::vector<std::string> get_supported_extensions();

LIBOPENMPT_CXX_API bool is_extension_supported( const std::string & extension );

LIBOPENMPT_CXX_API double could_open_propability( std::istream & stream, double effort = 1.0, std::ostream & log = std::clog );

class module_impl;

class interactive_module;

namespace detail {

typedef std::map< std::string, std::string > initial_ctls_map;

} // namespace detail

class LIBOPENMPT_CXX_API module {

	friend class interactive_module;

public:

	enum render_param {
		RENDER_MASTERGAIN_MILLIBEL        = 1,
		RENDER_STEREOSEPARATION_PERCENT   = 2,
		RENDER_INTERPOLATIONFILTER_LENGTH = 3,
		RENDER_VOLUMERAMPING_STRENGTH     = 4
	};

	enum command_index {
		command_note        = 0,
		command_instrument  = 1,
		command_volumeffect = 2,
		command_effect      = 3,
		command_volume      = 4,
		command_parameter   = 5
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
	module( std::istream & stream, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const std::vector<std::uint8_t> & data, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const std::uint8_t * beg, const std::uint8_t * end, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const std::uint8_t * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const std::vector<char> & data, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const char * beg, const char * end, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const char * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	module( const void * data, std::size_t size, std::ostream & log = std::clog, const std::map< std::string, std::string > & ctls = detail::initial_ctls_map() );
	virtual ~module();
public:

	void select_subsong( std::int32_t subsong );
	void set_repeat_count( std::int32_t repeat_count );
	std::int32_t get_repeat_count() const;

	double get_duration_seconds() const;

	double set_position_seconds( double seconds );
	double get_position_seconds() const;

	std::int32_t get_render_param( int param ) const;
	void set_render_param( int param, std::int32_t value );

	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * mono );
	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * left, std::int16_t * right );
	std::size_t read( std::int32_t samplerate, std::size_t count, std::int16_t * left, std::int16_t * right, std::int16_t * rear_left, std::int16_t * rear_right );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * mono );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * left, float * right );
	std::size_t read( std::int32_t samplerate, std::size_t count, float * left, float * right, float * rear_left, float * rear_right );
	std::size_t read_interleaved_stereo( std::int32_t samplerate, std::size_t count, std::int16_t * interleaved_stereo );
	std::size_t read_interleaved_quad( std::int32_t samplerate, std::size_t count, std::int16_t * interleaved_quad );
	std::size_t read_interleaved_stereo( std::int32_t samplerate, std::size_t count, float * interleaved_stereo );
	std::size_t read_interleaved_quad( std::int32_t samplerate, std::size_t count, float * interleaved_quad );

	std::vector<std::string> get_metadata_keys() const;
	std::string get_metadata( const std::string & key ) const;

	std::int32_t get_current_speed() const;
	std::int32_t get_current_tempo() const;
	std::int32_t get_current_order() const;
	std::int32_t get_current_pattern() const;
	std::int32_t get_current_row() const;
	std::int32_t get_current_playing_channels() const;

	float get_current_channel_vu_left( std::int32_t channel ) const;
	float get_current_channel_vu_right( std::int32_t channel ) const;

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

	std::string format_pattern_row_channel( std::int32_t pattern, std::int32_t row, std::int32_t channel, std::size_t width = 0, bool pad = true ) const;
	std::string highlight_pattern_row_channel( std::int32_t pattern, std::int32_t row, std::int32_t channel, std::size_t width = 0, bool pad = true ) const;

	std::vector<std::string> get_ctls() const;

	std::string ctl_get( const std::string & ctl ) const;
	void ctl_set( const std::string & ctl, const std::string & value );

	// remember to add new functions to both C and C++ interfaces and to increase OPENMPT_API_VERSION_MINOR

}; // class module

} // namespace openmpt

#endif // LIBOPENMPT_HPP
