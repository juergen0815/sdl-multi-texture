/*
 * dds_loader.cpp
 *
 *  Created on: Jan 27, 2013
 *      Author: Jurgen
 */

#include "dds_loader.h"

#include <err.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace bfs  = boost::filesystem;

/************************************************************************/
/* THIS CODE IS BASED ON GIMPS DDS IMPORT PLUGIN                        */
/************************************************************************/

// ASC: "DDS "
#define DDS_MAGIC 0x20534444

#ifdef __APPLE__
#define MAKE_4LE32(a,b,c,d) \
(((unsigned int)a      ) | \
 ((unsigned int)b <<  8) | \
 ((unsigned int)c << 16) | \
 ((unsigned int)d << 24))
#else
#define MAKE_4LE32(a,b,c,d) \
(((unsigned int)a      ) | \
 ((unsigned int)b <<  8) | \
 ((unsigned int)c << 16) | \
 ((unsigned int)d << 24))
#endif

static const int _DXT1(MAKE_4LE32( 'D','X','T','1' ));
static const int _DXT3(MAKE_4LE32( 'D','X','T','3' ));
static const int _DXT5(MAKE_4LE32( 'D','X','T','5' ));
static const int _ATI1(MAKE_4LE32( 'A','T','I','1' ));
static const int _ATI2(MAKE_4LE32( 'A','T','I','2' ));

struct Format
{
    int m_RShift;
    int m_GShift;
    int m_BShift;
    int m_AShift;

    unsigned int m_RMask;
    unsigned int m_GMask;
    unsigned int m_BMask;
    unsigned int m_AMask;
};

#define MAKE_COLOR( d, r,g,b, fmt ) \
d = ((r << fmt.m_RShift) | (g << fmt.m_GShift) | (b << fmt.m_BShift)) & ~fmt.m_AMask;

#define DDSD_CAPS                  0x00000001
#define DDSD_HEIGHT                0x00000002
#define DDSD_WIDTH                 0x00000004
#define DDSD_PITCH                 0x00000008
#define DDSD_PIXELFORMAT           0x00001000
#define DDSD_MIPMAPCOUNT           0x00020000
#define DDSD_LINEARSIZE            0x00080000
#define DDSD_DEPTH                 0x00800000

#define DDPF_ALPHAPIXELS           0x00000001
#define DDPF_ALPHA                 0x00000002
#define DDPF_FOURCC                0x00000004
#define DDPF_PALETTEINDEXED8       0x00000020
#define DDPF_RGB                   0x00000040
#define DDPF_LUMINANCE             0x00020000

#define DDSCAPS_COMPLEX            0x00000008
#define DDSCAPS_TEXTURE            0x00001000
#define DDSCAPS_MIPMAP             0x00400000

#define DDSCAPS2_CUBEMAP           0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000
#define DDSCAPS2_VOLUME            0x00200000

enum DDS_COMPRESSION_TYPE
{
    DDS_COMPRESS_NONE = 0,
    DDS_COMPRESS_BC1,        /* DXT1  */
    DDS_COMPRESS_BC2,        /* DXT3  */
    DDS_COMPRESS_BC3,        /* DXT5  */
    DDS_COMPRESS_BC3N,       /* DXT5n */
    DDS_COMPRESS_BC4,        /* ATI1  */
    DDS_COMPRESS_BC5,        /* ATI2  */
    DDS_COMPRESS_AEXP,       /* DXT5  */
    DDS_COMPRESS_YCOCG,      /* DXT5  */
    DDS_COMPRESS_YCOCGS,     /* DXT5  */
    DDS_COMPRESS_MAX
};

// Pack! Mapped from file. Do not pad!
#pragma pack( push, 2 )

struct DdsPixelFormat
{
    unsigned int m_Size;
    unsigned int m_Flags;
    union {
        char AsChar[4];
        int  AsInt;
    } m_FourCC;
    unsigned int m_BitsPerPixel;
    unsigned int m_RMask;
    unsigned int m_GMask;
    unsigned int m_BMask;
    unsigned int m_AMask;
};

struct DdsCapsT
{
    unsigned int m_Caps1;
    unsigned int m_Caps2;
    unsigned int reserved[2];
};

struct DdsHeader
{
    unsigned int m_Magic; // four bytes "DDS " or 0x20534444 (LE)
    unsigned int m_Size;
    unsigned int m_Flags;
    unsigned int m_Height;
    unsigned int m_Width;
    unsigned int m_BufferSize;
    unsigned int m_Depth;
    unsigned int m_NumMipMaps;
    unsigned char reserved[4 * 11];
    DdsPixelFormat m_PixelFormat;
    DdsCapsT       m_Caps;
    unsigned int reserved2;
};
typedef boost::shared_ptr<DdsHeader> DdsHeaderPtr;

#pragma pack( pop )

static unsigned char MaskToColorShift(unsigned int mask)
{
    unsigned char i = 0;

    if(!mask) return(0);
    while(!((mask >> i) & 1)) ++i;
    return(i);
}

class DdsLoader
{
#pragma pack( push, 2 )
    DdsHeader            m_DdsHeader;    // must copy. We'll update some flags
#pragma pack( pop )
    const unsigned char *m_Data;
public:
    DdsLoader( const  char* dataBuffer ) throw( std::exception )
        : m_DdsHeader( *((DdsHeader*)dataBuffer) )
    {
        ASSERT( VerifyHeader(), "File is not in DDS format or format is not supported!" );
        m_Data = (const unsigned char*)dataBuffer + sizeof(DdsHeader);
    }

    virtual ~DdsLoader()
    {
    }

    DdsHeader& GetHeader() { return m_DdsHeader; }

    const unsigned char* LoadPixels( size_t size, int line = 0 )
    {
        // m_DdsHeader.m_BufferSize == pitch
        const unsigned char* pix = m_Data + line * m_DdsHeader.m_BufferSize;
        return pix;
//        m_DdsFile.seekg( pos, std::ios::beg );
//        int t(0);
//        do {
//            t += (int)m_DdsFile.read( buffer, (std::streamsize)size ).gcount();
//            if ( m_DdsFile.bad() || m_DdsFile.eof() || m_DdsFile.fail() ) {
//                return -1;
//            }
//        } while ( t != (int)size ) ;
//        return t;
    }

private:
    bool VerifyHeader( )
    {
        if ( m_DdsHeader.m_Magic == DDS_MAGIC )
        {
            if ( (m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC) &&
                (std::memcmp( m_DdsHeader.m_PixelFormat.m_FourCC.AsChar, "DX10", 4 ) == 0) )
            {
                // DX10 not supported!
                ASSERT( 0, "DX10 not supported!" );
                return false;
            }

            if ( (m_DdsHeader.m_Flags & DDSD_PITCH) == (m_DdsHeader.m_Flags & DDSD_LINEARSIZE) )
            {
                if(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC) {
                    m_DdsHeader.m_Flags |= DDSD_LINEARSIZE;
                } else {
                    m_DdsHeader.m_Flags |= DDSD_LINEARSIZE;
                }
            }
            // we support DXT1, DXT3, DXT5, ATI1 & 2 compression
            int fourcc = m_DdsHeader.m_PixelFormat.m_FourCC.AsInt;
            if(( m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC ) &&
               ( fourcc != _DXT1 && fourcc != _DXT3 && fourcc != _DXT5 &&
                 fourcc != _ATI1 && fourcc != _ATI2 )
               )
            {
                ASSERT( 0, "Unsupported codec %d (only DXT1, DXT3, DXT5, ATI1 & 2 compression is supported) ", fourcc );
                return false;
            }

            if( m_DdsHeader.m_PixelFormat.m_Flags & DDPF_RGB)
            {
                if((m_DdsHeader.m_PixelFormat.m_BitsPerPixel !=  8) &&
                   (m_DdsHeader.m_PixelFormat.m_BitsPerPixel != 16) &&
                   (m_DdsHeader.m_PixelFormat.m_BitsPerPixel != 24) &&
                   (m_DdsHeader.m_PixelFormat.m_BitsPerPixel != 32))
                {
                    ASSERT( 0, "Invalid or unsupported bits per pixels (%d)", m_DdsHeader.m_PixelFormat.m_BitsPerPixel );
                    return false;
                }
            }
            else if(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_LUMINANCE)
            {
                if((m_DdsHeader.m_PixelFormat.m_BitsPerPixel !=  8) &&
                   (m_DdsHeader.m_PixelFormat.m_BitsPerPixel != 16))
                {
                    ASSERT( 0, "Invalid or unsupported bits per pixels (%d)", m_DdsHeader.m_PixelFormat.m_BitsPerPixel );
                    return false;
                }
                m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_RGB;
            }
            else if ( m_DdsHeader.m_PixelFormat.m_Flags & DDPF_PALETTEINDEXED8 )
            {
                m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_RGB;
            }

            if(!(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_RGB) &&
               !(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_ALPHA) &&
               !(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC) &&
               !(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_LUMINANCE))
            {
//                     Console::err("DdsLoader: Unknown pixel format! Taking a guess, expect trouble!");

                switch ( fourcc )
                {
                case _DXT1:
                case _DXT3:
                case _DXT5:
                case _ATI1:
                case _ATI2:
                    m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_FOURCC;
                    break;
                default:
                    switch(m_DdsHeader.m_PixelFormat.m_BitsPerPixel)
                    {
                    case 8:
                        if (m_DdsHeader.m_PixelFormat.m_Flags & DDPF_ALPHAPIXELS) {
                            m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_ALPHA;
                        } else {
                            m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_LUMINANCE;
                        }
                        break;
                    case 16:
                    case 24:
                    case 32:
                        m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_RGB;
                        break;
                    default:
                        return false;
                    }
                    break;
                }
            }

            if(m_DdsHeader.m_PixelFormat.m_Flags & DDPF_PALETTEINDEXED8)
            {
                ASSERT( 0, "Palette in DDS not supported" );
                // do this later.
                char* palette = new char[ 256 * 4 ];
//                m_DdsFile.read( palette, 256 * 4 );
                delete [] palette;
//                     d.palette = g_malloc(256 * 4);
//                     fread(d.palette, 1, 256 * 4, fp);
//                     for(i = j = 0; i < 768; i += 3, j += 4)
//                     {
//                         d.palette[i + 0] = d.palette[j + 0];
//                         d.palette[i + 1] = d.palette[j + 1];
//                         d.palette[i + 2] = d.palette[j + 2];
//                     }
//                     gimp_image_set_colormap(image, d.palette, 256);
            }

            if ( m_DdsHeader.m_BufferSize == 0 ) {
                if ( m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC )
                {
                    m_DdsHeader.m_BufferSize = ((m_DdsHeader.m_Width + 3) >> 2) * ((m_DdsHeader.m_Height + 3) >> 2);
                    if ( m_DdsHeader.m_PixelFormat.m_FourCC.AsChar[3] == '1')
                        m_DdsHeader.m_BufferSize *= 8;
                    else
                        m_DdsHeader.m_BufferSize *= 16;
                }
                else
                {
                    m_DdsHeader.m_BufferSize = m_DdsHeader.m_Width * m_DdsHeader.m_Height * (m_DdsHeader.m_PixelFormat.m_BitsPerPixel >> 3);
                }
            }

            if ( m_DdsHeader.m_PixelFormat.m_Flags & DDPF_FOURCC )
            {
                if ( m_DdsHeader.m_PixelFormat.m_FourCC.AsChar[0] == 'D' ) {
                    m_DdsHeader.m_PixelFormat.m_Flags |= DDPF_ALPHAPIXELS;
                }
            }

            return true;
        }
        return false;
    }
};

DdsBrush::DdsBrush()
{
    m_Width = 0;
    m_Height = 0;
    m_BytesPerPixel = 0; /* 3:RGB, 4:RGBA */
    m_Pixels = nullptr;
}

DdsBrush::~DdsBrush()
{
    if ( m_Pixels ) {
        delete [] m_Pixels;
    }
}

/**
 * mmap a BMP brush into memory. Does not flip bytes or anything.
 * Pixels are raw const char* from file!
 */
bool DdsBrush::Load( const char* filename ) throw(std::exception)
{
    bool result = false;
    if (!m_FileHandle.is_open() ) {
        this->m_Pixels = nullptr;
        m_FileHandle.open( filename, std::ios_base::binary | std::ios_base::in );
        if ( m_FileHandle.is_open() && m_FileHandle.size() >= sizeof(DdsHeader) ) {
            DdsLoader ddsLoader( m_FileHandle.const_data() );
            DdsHeader& hdr = ddsLoader.GetHeader();

            // DXT1/3/5 has alpha channel
            int bpp = 0;

            unsigned int width(hdr.m_Width);
            unsigned int height(hdr.m_Height);

            size_t size;
            int compression = DDS_COMPRESS_NONE;
            if ( hdr.m_PixelFormat.m_Flags & DDPF_FOURCC )
            {
                unsigned int w = (width  + 3) >> 2;
                unsigned int h = (height + 3) >> 2;

                switch (hdr.m_PixelFormat.m_FourCC.AsInt)
                {
                case _DXT1: compression = DDS_COMPRESS_BC1; bpp = 4; break;
                case _DXT3: compression = DDS_COMPRESS_BC2; bpp = 4; break;
                case _DXT5: compression = DDS_COMPRESS_BC3; bpp = 4; break;
                case _ATI1: compression = DDS_COMPRESS_BC4; bpp = 1; break;
                case _ATI2: compression = DDS_COMPRESS_BC5; bpp = 3; break;
                }

                size = w * h;
                if ( compression == DDS_COMPRESS_BC1 || compression == DDS_COMPRESS_BC4 ) {
                    size *= 8;
                } else {
                    size *= 16;
                }
                if ( width  < 4 ) width  = 4;
                if ( height < 4 ) height = 4;

            } else {
                // <1 on unsigned ???
                if ( width  < 1 ) width  = 1;
                if ( height < 1 ) height = 1;
                size = hdr.m_BufferSize;

                bpp = hdr.m_PixelFormat.m_BitsPerPixel >> 3;
            }

            // only care about 2 && 4 byte RGBA textures for now
            ASSERT( ( bpp == 4 || bpp == 2), "Unsupported bit depth. Only 2 and 4 bytes per pixels are supporte" );


            // source DDS descriptor (pixel data are in DDS format)
            // destination surface descriptor (decoded DDS) - we will keep that! This is our
            // target surface used to blit later

            // no cube map, no volume map
            if( ( hdr.m_Caps.m_Caps2 & (DDSCAPS2_CUBEMAP|DDSCAPS2_VOLUME)) == 0 )
            {
                // read flat data
                if ( hdr.m_PixelFormat.m_Flags & DDPF_FOURCC )
                {
                    // compressed
                    const unsigned char* pix = ddsLoader.LoadPixels( size, 0 );
                    if ( pix )
                    {
                        Format format =
                        {
                            0, 8, 16, 24,
                            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
//                            24, 16, 8, 0,   // shift
//                            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff // mask
                        };
                        m_Width  = width;
                        m_Height = height;
                        m_BytesPerPixel = bpp; // must be 4!!
                        m_Pixels = new char[ m_Width * m_BytesPerPixel * m_Height ];
                        switch ( compression )
                        {
                        case DDS_COMPRESS_BC1: // DXT1
                            m_FormatString = "DXT1";
                            {
                                unsigned int amask = format.m_AMask;
                                for ( unsigned int y = 0; y < height; y += 4 ) {
                                    for ( unsigned int x = 0; x < width; x += 4 )
                                    {
                                        unsigned int *dest = (unsigned int*)((m_Pixels) + (y * width + x) * bpp);
#pragma pack( push, 2 )
                                        // only works in LE mode!
                                        struct s_dxt1_pixel {
                                            unsigned short c0;   // r5g6b5 color 0
                                            unsigned short c1;   // r5g6b5 color 1
                                            unsigned long  clut; // 4x4 2bit color lut
                                        } *dxt1 = (s_dxt1_pixel*)pix;
#pragma pack( pop )
                                        unsigned long colorbits = dxt1->clut;
                                        // color block
                                        unsigned int c0 = (unsigned int)dxt1->c0; // R5G6B5
                                        unsigned int c1 = (unsigned int)dxt1->c1; // R5G6B5
                                        unsigned int color_table[4] = { amask, amask, amask, 0 };

                                        // no point in calculating remaining pixels if colors are 0
                                        if ( c0 > 0 || c1 > 0 )
                                        {
                                            // r5g6b5 -> r8g8b8
                                            int r0 = (c0 & 0xF800) >> 11; r0 = (r0 << 3 | r0 >> 2 );
                                            int g0 = (c0 & 0x07E0) >> 5;  g0 = (g0 << 2 | g0 >> 4);
                                            int b0 = (c0 & 0x001F);       b0 = (b0 << 3 | b0 >> 2);

                                            int r1 = (c1 & 0xF800) >> 11; r1 = (r1 << 3 | r1 >> 2 );
                                            int g1 = (c1 & 0x07E0) >> 5;  g1 = (g1 << 2 | g1 >> 4);
                                            int b1 = (c1 & 0x001F);       b1 = (b1 << 3 | b1 >> 2);

                                            MAKE_COLOR( color_table[0], r0, g0, b0, format );
                                            color_table[0] |= amask;
                                            MAKE_COLOR( color_table[1], r1, g1, b1, format );
                                            color_table[1] |= amask;

                                            if ( c0 > c1 ) {
                                                MAKE_COLOR( color_table[2], (2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, format );
                                                MAKE_COLOR( color_table[3], (r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, format );
                                                color_table[2] |= amask;
                                                color_table[3] |= amask;
                                            } else {
                                                r0 = (r0 + r1 + 1) >> 1;
                                                g0 = (g0 + g1 + 1) >> 1;
                                                b0 = (b0 + b1 + 1) >> 1;
                                                MAKE_COLOR( color_table[2], r0, g0, b0, format );
                                                color_table[2] |= amask;
                                                // color_table[3] = 0x00000000; - already set
                                            }
                                        }
                                        for ( int j = 0; j < 4; j++ )
                                        {
                                            unsigned int *d = dest + j * width;
                                            for ( int i = 0; i < 4; i++ )
                                            {
                                                unsigned int ci = (colorbits & 0x03); colorbits >>= 2;
                                                *d++ = color_table[ ci ];
                                            }
                                        }
                                        pix += 8; // advance 128 bits ( = 16 * 8 )
                                    }
                                }
                            }
                            result = true;
                            break;
                        case DDS_COMPRESS_BC2: // DXT3
                            m_FormatString = "DXT3";
                            {
                                for ( unsigned int y = 0; y < height; y += 4 ) {
                                    for ( unsigned int x = 0; x < width; x += 4 )
                                    {
                                        unsigned int *dest = (unsigned int*)((m_Pixels) + (y * width + x) * bpp);
#pragma pack( push, 2 )
                                        // only works in LE mode!
                                        struct s_dxt3_pixel {
                                            unsigned long long alpha; // 4x4 4 bit alpha channel;
                                            unsigned short c0;   // r5g6b5 color 0
                                            unsigned short c1;   // r5g6b5 color 1
                                            unsigned long  clut; // 4x4 2bit color lut
                                        } *dxt3 = (s_dxt3_pixel*)pix;
#pragma pack( pop )
                                        // mask out first to bytes (a0 & a1)
                                        unsigned long long alphabits = dxt3->alpha;
                                        unsigned long      colorbits = dxt3->clut;

                                        // color block
                                        unsigned int c0 = (unsigned int)dxt3->c0; // R5G6B5
                                        unsigned int c1 = (unsigned int)dxt3->c1; // R5G6B5
                                        unsigned int color_table[4] = { 0, 0, 0, 0 };
                                        // no point in calculating remaining pixels if colors are 0
                                        if ( c0 > 0 || c1 > 0 )
                                        {
                                            // r5g6b5 -> r8g8b8
                                            int r0 = (c0 & 0xF800) >> 11; r0 = (r0 << 3 | r0 >> 2);
                                            int g0 = (c0 & 0x07E0) >> 5;  g0 = (g0 << 2 | g0 >> 4);
                                            int b0 = (c0 & 0x001F);       b0 = (b0 << 3 | b0 >> 2);

                                            int r1 = (c1 & 0xF800) >> 11; r1 = (r1 << 3 | r1 >> 2);
                                            int g1 = (c1 & 0x07E0) >> 5;  g1 = (g1 << 2 | g1 >> 4);
                                            int b1 = (c1 & 0x001F);       b1 = (b1 << 3 | b1 >> 2);

                                            MAKE_COLOR( color_table[0], r0, g0, b0, format );
                                            MAKE_COLOR( color_table[1], r1, g1, b1, format );

                                            MAKE_COLOR( color_table[2], (2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, format );
                                            MAKE_COLOR( color_table[3], (r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, format );
                                        }

                                        for ( int j = 0; j < 4; j++ )
                                        {
                                            unsigned int *d = dest + j * width;
                                            for ( int i = 0; i < 4; i++ )
                                            {
                                                unsigned int alpha( (alphabits & 0x0f) * 17 ); alphabits >>= 4;
                                                unsigned int ci = colorbits & 0x03; colorbits >>= 2;
                                                *d++ = color_table[ ci ] | alpha;
                                            }
                                        }
                                        pix += 16; // advance 128 bits ( = 16 * 8 )
                                    }
                                }
                            }
                            result = true;
                            break;
                        case DDS_COMPRESS_BC3: // DXT5
                            m_FormatString = "DXT5";
                            {
                                for ( unsigned int y = 0; y < height; y += 4 ) {
                                    for ( unsigned int x = 0; x < width; x += 4 )
                                    {
                                        unsigned int *dest = (unsigned int*)((m_Pixels) + (y * width + x) * bpp);
#pragma pack( push, 2 )
                                        // only works in LE mode!
                                        struct s_dxt5_pixel {
                                            union {
                                                struct {
                                                    unsigned char a0; // first alpha byte
                                                    unsigned char a1; // second alpha byte
                                                } alpha;
                                                unsigned long long alut; // 4x4 3 bit alpha lut (containing a0&a1!)
                                            };
                                            unsigned short c0;   // r5g6b5 color 0
                                            unsigned short c1;   // r5g6b5 color 1
                                            unsigned long  clut; // 4x4 2bit color lut
                                        } *dxt5 = (s_dxt5_pixel*)pix;
#pragma pack( pop )
                                        unsigned int a0 = (unsigned int)dxt5->alpha.a0;
                                        unsigned int a1 = (unsigned int)dxt5->alpha.a1;
                                        // mask out first to bytes (a0 & a1)
                                        unsigned long long alphabits = (dxt5->alut >> 16);
                                        unsigned long      colorbits = dxt5->clut;

                                        // color block
                                        unsigned int c0 = (unsigned int)dxt5->c0; // R5G6B5
                                        unsigned int c1 = (unsigned int)dxt5->c1; // R5G6B5

                                        unsigned int color_table[4] = { 0, 0, 0, 0 };
                                        // no point in calculating remaining pixels if colors are 0
                                        if ( c0 > 0 || c1 > 0 )
                                        {
                                            // r5g6b5 -> r8g8b8
                                            int r0 = (c0 & 0xF800) >> 11; r0 = (r0 << 3 | r0 >> 2);
                                            int g0 = (c0 & 0x07E0) >> 5;  g0 = (g0 << 2 | g0 >> 4);
                                            int b0 = (c0 & 0x001F);       b0 = (b0 << 3 | b0 >> 2);

                                            int r1 = (c1 & 0xF800) >> 11; r1 = (r1 << 3 | r1 >> 2);
                                            int g1 = (c1 & 0x07E0) >> 5;  g1 = (g1 << 2 | g1 >> 4);
                                            int b1 = (c1 & 0x001F);       b1 = (b1 << 3 | b1 >> 2);

                                            MAKE_COLOR( color_table[0], r0, g0, b0, format );
                                            MAKE_COLOR( color_table[1], r1, g1, b1, format );

                                            MAKE_COLOR( color_table[2], (2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, format );
                                            MAKE_COLOR( color_table[3], (r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, format );
                                        }

                                        // Alpha table
                                        unsigned int alpha_table[8];
                                        alpha_table[0] = a0 << format.m_AShift & format.m_AMask;
                                        alpha_table[1] = a1 << format.m_AShift & format.m_AMask;
                                        if ( a0 > a1 ) {
                                            alpha_table[2] = (((6*a0 + 1*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[3] = (((5*a0 + 2*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[4] = (((4*a0 + 3*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[5] = (((3*a0 + 4*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[6] = (((2*a0 + 5*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[7] = (((1*a0 + 6*a1) / 7) << format.m_AShift) & format.m_AMask ;
                                        } else {
                                            alpha_table[2] = (((4*a0 + 1*a1) / 5) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[3] = (((3*a0 + 2*a1) / 5) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[4] = (((2*a0 + 3*a1) / 5) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[5] = (((1*a0 + 4*a1) / 5) << format.m_AShift) & format.m_AMask ;
                                            alpha_table[6] = 0;
                                            alpha_table[7] = format.m_AMask ;
                                        }

                                        for ( int j = 0; j < 4; j++ )
                                        {
                                            unsigned int *d = dest + j * width;
                                            for ( int i = 0; i < 4; i++ )
                                            {
                                                // I might need to pre shift those bits
                                                int ai = (int)(alphabits & 0x07); alphabits >>= 3;
                                                int ci = (colorbits & 0x03); colorbits >>= 2;
                                                // write back pixel
                                                *d++ = color_table[ ci ] | alpha_table[ai] ;
                                            }
                                        }
                                        pix += 16; // advance 128 bits ( = 16 * 8 )
                                    }
                                }
                            }
                            result = true;
                            break;
                        }
                    }
                }
            }
            else if( (hdr.m_PixelFormat.m_Flags & (DDPF_RGB|DDPF_ALPHA|DDPF_ALPHAPIXELS)) != 0 )
            {
                m_FormatString = "DDS Uncompressed";

                Format fmt = { MaskToColorShift( hdr.m_PixelFormat.m_RMask ),
                               MaskToColorShift( hdr.m_PixelFormat.m_GMask ),
                               MaskToColorShift( hdr.m_PixelFormat.m_BMask ),
                               MaskToColorShift( hdr.m_PixelFormat.m_AMask ),

                               hdr.m_PixelFormat.m_RMask ,
                               hdr.m_PixelFormat.m_GMask ,
                               hdr.m_PixelFormat.m_BMask ,
                               hdr.m_PixelFormat.m_AMask };

                // not sure why there would be a DDSD_PITCH without DDSD_LINEARSIZE - change later.
                if ( hdr.m_Flags & DDSD_LINEARSIZE )
                {
                    m_Width  = width;
                    m_Height = height;
                    m_BytesPerPixel = bpp;
                    // we must convert these to RGBA
                    m_Pixels = new char [ m_Width * m_BytesPerPixel * m_Height ];
                    const unsigned char* pix = ddsLoader.LoadPixels( size, 0 );
                    std::memcpy( const_cast<char*>(m_Pixels), pix, size );
                    result = true;
                }
                else if ( hdr.m_Flags & DDSD_PITCH )
                {
                    ASSERT( hdr.m_Flags & DDSD_PITCH, "Per line decoder not implemented!");
//                    m_Width  = width;
//                    m_Height = (hdr.m_Flags & DDSD_LINEARSIZE) ? height : 1;
//                    m_BytesPerPixel = bpp;

                    //                fmt.m_Depth  = hdr.m_PixelFormat.m_BitsPerPixel;
                    //                fmt.m_Pitch  = width * bpp;

                    // this will need some work because we will need to convert pixels...too much for demo code....just bail for now

                    // from pei::Engine
//                    m_Surface = SurfacePtr( new pei::Surface( width, height ));
//                    pei::Blitter blitter( pei::PixOpPtr( new pei::PixOpCopySrcAlpha( ddsLineBuffer->GetFormat(), m_Surface->GetFormat() )) );
//                     load one line at the time
//                    unsigned int y = 0;
//                    ddsLineBuffer->Lock();
//                    while ((y < height) && loader.LoadPixels( (char*)ddsLineBuffer->GetPixels(), size, y ) == size )
//                    {
//                        blitter.Blit( ddsLineBuffer, m_Surface, 0, y++, width, 1 );
//                    }
                    result = false;
                }
            }
        } else {
            ASSERT( 0, "File open error! Invalid file or file size! (%s)", filename );
        }
    }
    return result;
}


