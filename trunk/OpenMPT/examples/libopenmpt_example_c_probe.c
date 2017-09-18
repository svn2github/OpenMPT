/*
 * libopenmpt_example_c_probe.c
 * ----------------------------
 * Purpose: libopenmpt C API probing example
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

/*
 * Usage: libopenmpt_example_c_probe SOMEMODULE ...
 * Returns 0 on successful probing for all files.
 * Returns 1 on failed probing for 1 or more files.
 * Returns 2 on error.
 */

#define LIBOPENMPT_EXAMPLE_PROBE_RESULT_BINARY 1
#define LIBOPENMPT_EXAMPLE_PROBE_RESULT_FLOAT  2

#define LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS 1
#define LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX    2

#define LIBOPENMPT_EXAMPLE_PROBE_STYLE  LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX

#define LIBOPENMPT_EXAMPLE_PROBE_RESULT LIBOPENMPT_EXAMPLE_PROBE_RESULT_FLOAT

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libopenmpt/libopenmpt.h>

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS )
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>
#endif

static void libopenmpt_example_logfunc( const char * message, void * userdata ) {
	(void)userdata;

	if ( message ) {
		fprintf( stderr, "%s\n", message );
	}
}

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )

typedef struct blob_t {
	size_t size;
	void * data;
} blob_t;

static void free_blob( blob_t * blob ) {
	if ( blob ) {
		if ( blob->data ) {
			free( blob->data );
			blob->data = 0;
		}
		blob->size = 0;
		free( blob );
	}
}

#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
static blob_t * load_file( const wchar_t * filename, size_t prefix_size ) {
#else
static blob_t * load_file( const char * filename, size_t prefix_size ) {
#endif
	blob_t * result = 0;

	blob_t * blob = 0;
	FILE * file = 0;
	long tell_result = 0;

	blob = malloc( sizeof( blob_t ) );
	if ( !blob ) {
		goto fail;
	}
	memset( blob, 0, sizeof( blob_t ) );

#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
	file = _wfopen( filename, L"rb" );
#else
	file = fopen( filename, "rb" );
#endif
	if ( !file ) {
		goto fail;
	}

	if ( fseek( file, 0, SEEK_END ) != 0 ) {
		goto fail;
	}

	tell_result = ftell( file );
	if ( tell_result < 0 ) {
		goto fail;
	}
	if ( (unsigned long long)tell_result > SIZE_MAX ) {
		goto fail;
	}
	blob->size = (size_t)tell_result;

	if ( fseek( file, 0, SEEK_SET ) != 0 ) {
		goto fail;
	}

	if ( prefix_size > 0 ) {
		if ( prefix_size > blob->size ) {
			prefix_size = blob->size;
		}
	} else {
		prefix_size = blob->size;
	}

	blob->data = malloc( prefix_size );
	if ( !blob->data ) {
		goto fail;
	}
	memset( blob->data, 0, prefix_size );

	if ( fread( blob->data, 1, prefix_size, file ) != prefix_size ) {
		goto fail;
	}

	result = blob;
	blob = 0;
	goto cleanup;

fail:

	result = 0;

cleanup:

	if ( blob ) {
		free_blob( blob );
		blob = 0;
	}

	if ( file ) {
		fclose( file );
		file = 0;
	}

	return result;
}

#endif

#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
static int probe_file( const wchar_t * filename ) {
#else
static int probe_file( const char * filename ) {
#endif

	int result = 0;
	int mod_err = OPENMPT_ERROR_OK;
#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )
	blob_t * blob = 0;
#elif ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS )
	FILE * file = NULL;
#endif

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )
	size_t prefix_size = openmpt_probe_file_header_get_recommended_size();
	int probe_file_header_result = OPENMPT_PROBE_FILE_HEADER_RESULT_FAILURE;
#endif
#if ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_FLOAT )
	double probability = 0.0;
#endif
#if ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_BINARY )
	int result_binary = 0;
#endif

#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
	if ( wcslen( filename ) == 0 ) {
		fprintf( stderr, "Error: %s\n", "Wrong invocation. Use 'libopenmpt_example_c_probe SOMEMODULE'." );
		goto fail;
	}
#else
	if ( strlen( filename ) == 0 ) {
		fprintf( stderr, "Error: %s\n", "Wrong invocation. Use 'libopenmpt_example_c_probe SOMEMODULE'." );
		goto fail;
	}
#endif

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )
	blob = load_file( filename, prefix_size );
	if ( !blob ) {
		fprintf( stderr, "Error: %s\n", "load_file() failed." );
		goto fail;
	}
#elif ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS )
#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
	file = _wfopen( filename, L"rb" );
#else
	file = fopen( filename, "rb" );
#endif
	if ( !file ) {
		fprintf( stderr, "Error: %s\n", "fopen() failed." );
		goto fail;
	}
#endif

	#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )

		if ( prefix_size > blob->size ) {
			prefix_size = blob->size;
		}
		#if ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_BINARY )
			probe_file_header_result = openmpt_probe_file_header( OPENMPT_PROBE_FILE_HEADER_FLAGS_DEFAULT, blob->data, prefix_size, blob->size, &libopenmpt_example_logfunc, NULL, &openmpt_error_func_default, NULL, &mod_err, NULL );
			result_binary = 0;
			switch ( probe_file_header_result ) {
				case OPENMPT_PROBE_FILE_HEADER_RESULT_SUCCESS:
					result_binary = 1;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_FAILURE:
					result_binary = 0;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_WANTMOREDATA:
					result_binary = 0;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_ERROR:
					result_binary = 0;
					fprintf( stderr, "Error: %s\n", "openmpt_probe_file_header() failed." );
					goto fail;
					break;
				default:
					result_binary = 0;
					fprintf( stderr, "Error: %s\n", "openmpt_probe_file_header() failed." );
					goto fail;
					break;
			}
#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
			fprintf( stdout, "%s - %ls\n", result_binary ? "Success" : "Failure", filename );
#else
			fprintf( stdout, "%s - %s\n", result_binary ? "Success" : "Failure", filename );
#endif
			if ( result_binary ) {
				result = 0;
			} else {
				result = 1;
			}
		#elif ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_FLOAT )
			probe_file_header_result = openmpt_probe_file_header( OPENMPT_PROBE_FILE_HEADER_FLAGS_DEFAULT, blob->data, prefix_size, blob->size, &libopenmpt_example_logfunc, NULL, &openmpt_error_func_default, NULL, &mod_err, NULL );
			probability = 0.0;
			switch ( probe_file_header_result ) {
				case OPENMPT_PROBE_FILE_HEADER_RESULT_SUCCESS:
					probability = 1.0;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_FAILURE:
					probability = 0.0;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_WANTMOREDATA:
					probability = 0.5;
					break;
				case OPENMPT_PROBE_FILE_HEADER_RESULT_ERROR:
					probability = 0.6;
					fprintf( stderr, "Error: %s\n", "openmpt_probe_file_header() failed." );
					goto fail;
					break;
				default:
					probability = 0.6;
					fprintf( stderr, "Error: %s\n", "openmpt_probe_file_header() failed." );
					goto fail;
					break;
			}
#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
			fprintf( stdout, "%s: %f - %ls\n", "Result", probability, filename );
#else
			fprintf( stdout, "%s: %f - %s\n", "Result", probability, filename );
#endif
			if ( probability >= 0.5 ) {
				result = 0;
			} else {
				result = 1;
			}
		#else
			#error "LIBOPENMPT_EXAMPLE_PROBE_RESULT is wrong"
		#endif

	#elif ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS )

		#if ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_BINARY )
			result_binary = ( openmpt_probe_file_header_from_stream( OPENMPT_PROBE_FILE_HEADER_FLAGS_DEFAULT, openmpt_stream_get_file_callbacks(), file, &libopenmpt_example_logfunc, NULL, &openmpt_error_func_default, NULL, &mod_err, NULL ) != OPENMPT_PROBE_FILE_HEADER_RESULT_SUCCESS ) ? 0 : 1;
#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
			fprintf( stdout, "%s - %ls\n", result_binary ? "Success" : "Failure", filename );
#else
			fprintf( stdout, "%s - %s\n", result_binary ? "Success" : "Failure", filename );
#endif
			if ( result_binary ) {
				result = 0;
			} else {
				result = 1;
			}
		#elif ( LIBOPENMPT_EXAMPLE_PROBE_RESULT == LIBOPENMPT_EXAMPLE_PROBE_RESULT_FLOAT )
			probability = openmpt_could_open_probability( openmpt_stream_get_file_callbacks(), file, 0.25, &libopenmpt_example_logfunc, NULL, &openmpt_error_func_default, NULL, &mod_err, NULL );
#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
			fprintf( stdout, "%s: %f - %ls\n", "Result", probability, filename );
#else
			fprintf( stdout, "%s: %f - %s\n", "Result", probability, filename );
#endif
			if ( probability >= 0.5 ) {
				result = 0;
			} else {
				result = 1;
			}
		#else
			#error "LIBOPENMPT_EXAMPLE_PROBE_RESULT is wrong"
		#endif

	#else
		#error "LIBOPENMPT_EXAMPLE_PROBE_STYLE is wrong"
	#endif

	goto cleanup;

fail:

	result = 2;

cleanup:

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_PREFIX )
	if ( blob ) {
		free_blob( blob );
		blob = 0;
	}
#endif

#if ( LIBOPENMPT_EXAMPLE_PROBE_STYLE == LIBOPENMPT_EXAMPLE_PROBE_STYLE_CALLBACKS )
	if ( file ) {
		fclose( file );
		file = 0;
	}
#endif

	return result;
}


#if ( defined( _WIN32 ) || defined( WIN32 ) ) && ( defined( _UNICODE ) || defined( UNICODE ) )
int wmain( int argc, wchar_t * argv[] ) {
#else
int main( int argc, char * argv[] ) {
#endif

	int global_result = 0;

	if ( argc <= 1 ) {
		fprintf( stderr, "Error: %s\n", "Wrong invocation. Use 'libopenmpt_example_c_probe SOMEMODULE ...'." );
		goto fail;
	}

	for ( int i = 1; i < argc; ++i ) {
		int result = probe_file( argv[i] );
		if ( result > global_result ) {
			global_result = result;
		}
	}

	goto cleanup;

fail:

	global_result = 2;

cleanup:

	return global_result;

}
