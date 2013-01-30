/*
 * vertexbuffer.cpp
 *
 *  Created on: Jan 29, 2013
 *      Author: Jurgen
 */

#include "vertexbuffer.h"

struct PoolDeleter
{
    void operator()(void const *p) const
    {
        EntityPool::DestroyPool( (MemoryPool*)p );
    }
};

VertexBuffer::VertexBuffer()
    : m_MemoryPool( EntityPool::CreatePool<Vector>( 32 ), PoolDeleter() )
    , m_NormalArray( m_MemoryPool )
    , m_ColorArray( m_MemoryPool )
    , m_TextureArray( m_MemoryPool )
    , m_VertexArray( m_MemoryPool )
{

}

VertexBuffer::~VertexBuffer()
{
}

float* VertexBuffer::GetNormalBuffer()
{
    return (float*)&m_NormalArray[0];
}

float* VertexBuffer::GetColorBuffer()
{
    return (float*)&m_ColorArray[0];
}

float* VertexBuffer::GetTextureBuffer()
{
    return (float*)&m_TextureArray[0];
}

float* VertexBuffer::GetVertexBuffer()
{
    return (float*)&m_VertexArray[0];
}
