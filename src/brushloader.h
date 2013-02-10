/*
 * brushloader.h
 *
 *  Created on: Feb 10, 2013
 *      Author: Jurgen
 */

#ifndef BRUSHLOADER_H_
#define BRUSHLOADER_H_

#include "err.h"

#include <bmp_loader.h>
#include <dds_loader.h>
#include <tga_loader.h>

#include <boost/filesystem.hpp>

template < class T >
BrushPtr LoadBrush( const char* name ) throw(std::exception)
{
    BrushPtr brushPtr;
    try {
        T* brush( new T );
        std::string file("data/");
        file += name;
        ASSERT( brush->Load( file.c_str() ), "Error loading base texture" );
        brushPtr = BrushPtr(brush);
    } catch ( boost::filesystem::filesystem_error &ex ) {
        THROW( "Error loading texture '%s'.\n%s", name, ex.what() );
    } catch ( std::ios_base::failure& ex ) {
        THROW( "Error loading texture '%s'.\n%s", name, ex.what() );
    } catch ( std::exception &ex ) {
        std::string what;
        what = ex.what();
        what += "'";
        what += name;
        what += "'";
        throw std::runtime_error( what.c_str() );
    }
    return brushPtr;
}


#endif /* BRUSHLOADER_H_ */
