/*
 * texture.cpp
 *
 *  Created on: Jan 19, 2013
 *      Author: Jurgen
 */

#include "texture.h"

#include <GL/glew.h>

Texture::Texture()
    : m_TextID( -1 )
    , m_Width(0)
    , m_Height(0)
    , m_TextureFilter(GL_LINEAR)
    , m_WrapMode(GL_REPEAT)
{
}

Texture::~Texture()
{
    if ( m_TextID > -1 ) {
        glDeleteTextures(1,(GLuint*)&m_TextID);
    }
}

void Texture::SetFilter( int filter )
{
    try {
        // need to figure out if this is called from the current context ??? - or disallow calling from main thread???
        if ( m_TextID > -1 ) {
            /* Linear Filtering */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_TextureFilter );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_TextureFilter );
        }
    }
    // we need to handle OGL exceptions somehow
    catch (...)
    {

    }
}

void Texture::SetWrapMode( int clampMode )
{
    try {
        // TODO: use flags to set wrap mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);

    }
    catch ( ... )
    {

    }
}

bool Texture::Allocate( int width, int height, int bpp /*= 4*/ ) throw(std::exception)
{
    bool r(false);
    m_Width  = width;
    m_Height = height;
    if ( m_TextID <= 0) {
        glGenTextures( 1, (GLuint*)&m_TextID );
        GL_ASSERT( m_TextID > 0, "Error generating texture!" );
    }
    if ( m_TextID > 0 )
    {
        glBindTexture( GL_TEXTURE_2D, m_TextID );

        /* Linear Filtering */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_TextureFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_TextureFilter );

        // TODO: use flags to set wrap mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);

        // TODO: use flags to switch mip maps on/off
        glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

        GLint type;
        switch ( bpp )
        {
        case 1: type = GL_LUMINANCE; break; // could be GL_ALPHA - not supported. Use LUMINANCE_ALPHA
        case 2: type = GL_LUMINANCE_ALPHA; break;
        case 3: type = GL_RGB; break;
        case 4: type = GL_RGBA; break;
        default: ASSERT( 0, "Invalid Bytes per Pixel (%d). Must be 1, 2, 3 or 4.", bpp); break;
        }
        // generate pixel buffer in vmem
        glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);
        r = true;
    }
    return r;
}

void Texture::Load( const char* pixels, int width, int height, int bpp, int type ) throw(std::exception)
{
    ASSERT( (bpp == 1 || bpp == 2 || bpp == 3 || bpp == 4), "Pixel data must be 1, 2, 3 or 4 bytes per pixel!" );
    if ( width > m_Width || height > m_Height ) {
        Allocate( width, height, bpp );
    }
    Enable();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, type, GL_UNSIGNED_BYTE, pixels);
}

void Texture::Load( const Brush& brush ) throw(std::exception)
{
    ASSERT( brush.m_Pixels && ( brush.m_BytesPerPixel == 3 || brush.m_BytesPerPixel == 4 ),
            "Invalid brush for texture!" );
    Load( brush.m_Pixels, brush.m_Width, brush.m_Height, brush.m_BytesPerPixel, brush.m_BytesPerPixel == 3 ? GL_BGR : GL_BGRA );
}

unsigned int Texture::GetTextureId() const
{
    return m_TextID;
}

void Texture::Bind() const
{
    ASSERT( m_TextID > -1, "Invalid Texture ID" );

    /* Typical Texture Generation Using Data From The Bitmap */
    glBindTexture( GL_TEXTURE_2D, m_TextID );
}

void Texture::Enable() const
{
    glEnable( GL_TEXTURE_2D );
    Bind();
}

void Texture::Disable() const
{
    glDisable( GL_TEXTURE_2D );
}

