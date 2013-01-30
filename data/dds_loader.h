/*
 * dds_loader.h
 *
 *  Created on: Jan 27, 2013
 *      Author: Jurgen
 */

#ifndef DDS_LOADER_H_
#define DDS_LOADER_H_

#include <brush.h>

#include <string>

#include <boost/iostreams/device/mapped_file.hpp>
namespace bios = boost::iostreams;

class DdsBrush : public Brush
{
    bios::mapped_file m_FileHandle;
    std::string       m_FormatString;
public:
    DdsBrush();

    ~DdsBrush();

    /**
     * mmap a BMP brush into memory. Does not flip bytes or anything.
     * Pixels are raw const char* from file!
     */
    bool Load( const char* filename ) throw(std::exception);

};


#endif /* DDS_LOADER_H_ */
