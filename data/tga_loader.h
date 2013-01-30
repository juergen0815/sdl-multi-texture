/*
 * tga_loader.h
 *
 *  Created on: Jan 27, 2013
 *      Author: Jurgen
 */

#ifndef TGA_LOADER_H_
#define TGA_LOADER_H_

#include <brush.h>

#include <boost/iostreams/device/mapped_file.hpp>
namespace bios = boost::iostreams;

class TgaBrush : public Brush
{
    bios::mapped_file m_FileHandle;
public:
    TgaBrush();

    ~TgaBrush();

    /**
     * mmap a BMP brush into memory. Does not flip bytes or anything.
     * Pixels are raw const char* from file!
     */
    bool Load( const char* filename ) throw(std::exception);

};



#endif /* TGA_LOADER_H_ */
