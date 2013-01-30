/*
 * vertexbuffer.h
 *
 *  Created on: Jan 29, 2013
 *      Author: Jurgen
 */

#ifndef VERTEXBUFFER_H_
#define VERTEXBUFFER_H_

#include "vector.h"
#include "allocator.h"
#include "entitypool.h"

#include <vector>

#include <boost/shared_ptr.hpp>

class VertexBuffer
{
    typedef std::vector< Vector, Allocator<Vector> > VertexArray;

    boost::shared_ptr<MemoryPool> m_MemoryPool;

    VertexArray m_NormalArray;
    VertexArray m_ColorArray;
    VertexArray m_TextureArray;
    VertexArray m_VertexArray;
public:
    VertexBuffer();

    ~VertexBuffer();

    float* GetNormalBuffer();

    float* GetColorBuffer();

    float* GetTextureBuffer();

    float* GetVertexBuffer();
};

#endif /* VERTEXBUFFER_H_ */
