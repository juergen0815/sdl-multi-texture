/*
 * cube.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef CUBE_H_
#define CUBE_H_

#include "entity.h"
#include "brush.h"
#include "texture.h"

#include <GL/glew.h>

class Cube : public Entity
{
public:
    enum
    {
        BLEND_COLOR_F = 1<<RenderState::USER_B
    };
private:
	GLuint m_VboID;

protected:
	std::vector<BrushPtr>    m_Assets;
	std::vector<TexturePtr>  m_Textures;
public:
	Cube();

	Cube( std::vector<BrushPtr> assetList );

	virtual ~Cube();
protected:
	virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

	virtual void DoRender() throw(std::exception);

	virtual void DoUpdate( float ticks ) throw(std::exception) {}

};


#endif /* CUBE_H_ */
