/*
 * tga_loader.cpp
 *
 *  Created on: Jan 27, 2013
 *      Author: Jurgen
 */

#include "tga_loader.h"

#include <err.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace bfs  = boost::filesystem;

TgaBrush::TgaBrush()
{
    m_Width = 0;
    m_Height = 0;
    m_BytesPerPixel = 0; /* 3:RGB, 4:RGBA */
    m_Pixels = nullptr;
}

TgaBrush::~TgaBrush()
{

}

/**
 * mmap a BMP brush into memory. Does not flip bytes or anything.
 * Pixels are raw const char* from file!
 */
bool TgaBrush::Load( const char* filename ) throw(std::exception)
{
    // make sure compiler does not add padding
#pragma pack( push, 1 )
    // directly from OpenGL SuperBible Rev5 (GLTools.cpp)
    struct TgaHeader
    {
        uint8_t  identsize;              // Size of ID field that follows header (0)
        uint8_t  colorMapType;           // 0 = None, 1 = paletted
        uint8_t  imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
        uint16_t colorMapStart;          // First colour map entry
        uint16_t colorMapLength;         // Number of colors
        uint8_t  colorMapBits;           // bits per palette entry
        uint16_t xstart;                 // image x origin
        uint16_t ystart;                 // image y origin
        uint16_t width;                  // width in pixels
        uint16_t height;                 // height in pixels
        uint8_t  bits;                   // bits per pixel (8 16, 24, 32)
        uint8_t  descriptor;             // image descriptor
    };
#pragma pack( pop )

    if ( filename && (std::strlen(filename) > 4) && (std::strncmp(&filename[std::strlen(filename)-4],".tga",4) == 0) && !m_FileHandle.is_open() ) {
        this->m_Pixels = nullptr;
        m_FileHandle.open( filename, std::ios_base::binary | std::ios_base::in );
        if ( m_FileHandle.is_open() && m_FileHandle.size() >= sizeof(TgaHeader) ) {
            const TgaHeader* tga = (const TgaHeader*)m_FileHandle.const_data();
            if (  tga->colorMapType == 0 &&
                 (tga->imageType == 0 || tga->imageType == 2) &&
                 (tga->bits == 8 || tga->bits == 24 || tga->bits == 32) )
            {
                this->m_Width  = tga->width;
                this->m_Height = tga->height;
                // 8 bit is luminance
                this->m_BytesPerPixel = (tga->bits >> 3);
                // we map our pixel straight into the mmaped file
                this->m_Pixels = ((const char*)tga) + sizeof(TgaHeader) + tga->identsize;
            } else {
                ASSERT( 0, "Unsupported format. Cannot load TGA (%s)!", filename );
            }
//                ASSERT( 0, "File '%s' is not a TGA format!", filename );
        } else {
            ASSERT( 0, "File open error! Invalid file or file size! (%s)", filename );
        }
    }
    return this->m_Pixels != nullptr;
}



