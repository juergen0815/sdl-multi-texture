/*
 * texture.h
 *
 *  Created on: Jan 19, 2013
 *      Author: Jurgen
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "err.h"
#include "brush.h"

#include <GL/glew.h>

#include <boost/shared_ptr.hpp>

class Texture
{
protected:
    int m_TextID;
    int m_Width, m_Height;
    int m_TextureFilter;
    int m_WrapMode;
public:
    Texture();

    ~Texture();

    int GetWidth() const { return m_Width; }

    int GetHeight() const { return m_Height; }

    bool Allocate( int width, int height, int bpp = 4 ) throw(std::exception);

    void Load( const char* pixels, int width, int height, int bpp = 4, int type = GL_RGBA ) throw(std::exception);

    void Load( const Brush& brush ) throw(std::exception);

    unsigned int GetTextureId() const;

    void Bind() const;

    void Enable() const;

    void Disable() const;

    void SetFilter( int filter );

    void SetWrapMode( int clampMode );
};

typedef boost::shared_ptr<Texture> TexturePtr;


#endif /* TEXTURE_H_ */
