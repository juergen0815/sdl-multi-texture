/*
 * err.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef ERR_H_
#define ERR_H_

#include <exception>
#include <stdexcept>
#include <cstdio>
#include <boost/assert.hpp>

const char* glErrMessage( int code );

void ShowWindowsError( const char* msg, unsigned long err, const char* header = "Windows Error" );

void ShowError( const char* msg, const char* header = "Error" );

#define THROW( ... ) \
{                                           \
    char msg[ 256 ];                     \
    snprintf( msg, 256, __VA_ARGS__ );   \
    char buffer[ 480 ];                     \
    std::snprintf( buffer, 480, "%s\nIn Function: %s\nFile: %s\nLine %d", msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__ );   \
    throw std::runtime_error( buffer );     \
}

#define ASSERT( cond, ... ) if ( !(cond) ) THROW( __VA_ARGS__ );

#define GL_ASSERT( cond, ... ) \
    if ( !(cond) ) {              \
        char b1[ 256 ];                     \
        std::snprintf( b1, 256, __VA_ARGS__ );   \
        char b2[ 480 ];                     \
        int code(glGetError());             \
        std::snprintf( b2, 480, "%s\n-----------------------------\n" \
                                "OpenGL Error: %d\n%s\n" \
                                "\nIn Function: %s\nFile: %s\nLine: %d", b1, code, glErrMessage(code), BOOST_CURRENT_FUNCTION, __FILE__, __LINE__ ); \
        throw std::runtime_error( b2 );     \
    }

#ifdef _VERBOSE
#define INFO( header, ... ) \
{           \
    char msg[ 256 ];                     \
    snprintf( msg, 256, __VA_ARGS__ );   \
    ShowError( msg, header ); \
}
#else
#define INFO( header, ... )
#endif

#endif /* ERR_H_ */
