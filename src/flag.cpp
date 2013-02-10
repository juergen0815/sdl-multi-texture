/*
 * cube.cpp
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */


#include "flag.h"
#include "cube.h"
#include "brush.h"
#include "brushloader.h"

#include <cmath>

#include <boost/filesystem.hpp>

const int columns = 120;
const int rows    = 120;

struct PoolDeleter
{
    void operator()(void const *p) const
    {
        // called when shared_ptr goes out of scope - that is last container releases link to pool
        EntityPool::DestroyPool( (MemoryPool*)p );
    }
};

Flag::Flag( const std::vector< BrushPtr >& assets )
    : m_VboID(-1)
    , m_Assets( assets )
    , m_MemoryPool( EntityPool::CreatePool<Vector>( 0 ), PoolDeleter() )
    , m_Stride(2)// store two vectors per vertex
    , m_VertexBuffer( m_MemoryPool )    // use the same memory pool for vertex and texture coords
    , m_TimeEllapsed(0)
    , m_Speed(1.0)
{
    // we might just want to create this in DoInitialize - and throw away the data we don't need locally

    // allocate memory buffers for vertex and texture coord
    m_VertexBuffer.resize( columns*rows*m_Stride );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( (rows-1) * (columns-1) * 3 * 2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    int looper(0);
    // width x height is always a quad, not a rect
    const float width_2  = 5.0f; // 4.4f - for columns = 45
    const float height_2 = 5.0f;

    // generate vertex array
    const float xstep = (2*width_2)/columns; // mesh sub divider - 0.2f
    const float ystep = (2*height_2)/rows; // mesh sub divider - 0.2f
    const float amp  = 0.85f; // "height" of wave
    const float numWaves = 16.0f; // num of sin loops (or waves)
    auto vit = m_VertexBuffer.begin();

    // I think we need an additional row/column to finish this mesh ??
    for ( float y = 0; y < rows; ++y )
    {
        for ( float x = 0; x < columns; ++x )
        {
            Vector& vertex = *vit; ++vit;
            vertex[ Vector::X ] = x * xstep - width_2; // -4.4 ... +4.4
            vertex[ Vector::Y ] = y * ystep - height_2; // -4.4 ... +4.4
            // maybe I should shift this for each row, huh, norm x to "length" of column (0.0 - 1.0)
            vertex[ Vector::Z ] = std::sin( (x/columns) * numWaves ) * amp; // make z a big "wavy"

            // calc texture positions
            Vector& texCoord = *vit; ++vit;
            texCoord[ Vector::U ] = x/(columns-1);
            texCoord[ Vector::V ] = y/(rows-1);

            // this needs work: we use a row * col vertex and texture array
            // to extract triangles, the index array needs to be calculated appropriately
            //        0  1  2...n
            //        +--+--+...
            //        |\ |\ |
            //        | \| \|
            //        +--+--+
            // n*y +  0' 1' 2'...(n+1)*y

            // e.g. t[0] = { 0,1,1'} { 1',0',1 } ...

            // skip last column/row - already indexed
            if ( x < (columns-1) && y < (rows-1) ) {
                // vertices don't need to be set just yet. We just index them here

                // top tri
                int
                idx = int(x + 0 + columns*y);     m_IndexArray[ looper++ ] = idx;  // 0x0
                idx = int(x + 1 + columns*y);     m_IndexArray[ looper++ ] = idx;  // 1x0
                idx = int(x + 0 + columns*(y+1)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

                // bottom tri
                idx = int(x + 1 + columns*y);     m_IndexArray[ looper++ ] = idx; // 0x0
                idx = int(x + 0 + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
                idx = int(x + 1 + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
                idx = 0;
            }
        }
    }

    int pitch  = columns*m_Stride; // pair of Vertex + TexCoord
    int x = columns/2*m_Stride;
    int y = rows/2;
    auto mid = m_VertexBuffer[ x + y * pitch ]; // stride = sizeof(Vector) - we have a pair here, Vertex+Tex

    std::vector< BrushPtr > brushes;
    brushes.push_back( LoadBrush<BmpBrush>("Wood.bmp") );

    m_Child = EntityPtr( new Cube( brushes ) );
    m_Child->GetRenderState()->Translate( mid, Vector(1.0f, 1.0f, 1.0f) );
    // this entity renders
    AddEntity( m_Child, 0 );
}

Flag::~Flag()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    if ( m_VboID > 0 ) {
        glDeleteBuffers(1, &m_VboID);
    }
}

bool Flag::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    bool r(false);

    bool hasMultiTexture  = glewGetExtension("GL_ARB_multitexture");
    ASSERT( hasMultiTexture, "No multitexture support!" );
    int numTextureUnits;
    glGetIntegerv( GL_MAX_TEXTURE_UNITS, &numTextureUnits );
    ASSERT( MAX_TEXTURES <= numTextureUnits, "Not enough texture units available." );
    ASSERT( MAX_TEXTURES <= m_Assets.size(), "Not enough textures attached! At least %d textures required.", MAX_TEXTURES );

    for (auto& asset : m_Assets ) {
        TexturePtr texture( new Texture );
        texture->Load( *asset.get() );
        m_Textures.push_back( texture );
    }
    GetRenderState()->ClearFlag( BLEND_COLOR_F );

    bool hasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );
    if ( hasVBO ) {
        glGenBuffers(1, &m_VboID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector)*m_VertexBuffer.size(), 0, GL_STATIC_DRAW_ARB);
        std::size_t offset(0);
        // copy vertices starting from 0 offest - holds both, vertex and texture array
        float *vertices = (float*)&m_VertexBuffer[0];
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_VertexBuffer.size(), vertices);
        offset += sizeof(Vector)*m_VertexBuffer.size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(int)*m_IndexArray.size(), &m_IndexArray[0] );
        offset += sizeof(int)*m_IndexArray.size();
    }
    r = true;

    return r;
}

void Flag::DoRender() throw(std::exception)
{

    int vertexArrayEnabled;
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    int m_TexCoordArrayEnabled;
    glGetIntegerv( GL_TEXTURE_COORD_ARRAY, &m_TexCoordArrayEnabled );
    if ( m_Textures.size() && !m_TexCoordArrayEnabled ) {
        if (!m_TexCoordArrayEnabled) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    int blend_enabled;
    glGetIntegerv(GL_BLEND, &blend_enabled);

#ifdef _USE_VBO_
    glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
    // before draw, specify vertex and index arrays with their offsets
    std::size_t offset(0);
    glVertexPointer(3, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset); offset += sizeof(vertices);
    if ( m_Texture) {
        m_Texture->Enable();
        offset += sizeof(Vector);
        glTexCoordPointer(2, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset );
        if (blend_enabled) {
            glDisable( GL_BLEND );
        }
    }
    // use draw indices
    int *indices = &m_IndexArray[0];
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, indices );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    // before draw, specify vertex arrays
    float *vertices = (float*)&m_VertexBuffer[0];
    glVertexPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), vertices);
#endif
    // Base texture. No special treatment. Just draw it
    if ( m_Textures[BASE_TEXTURE] ) {
        glClientActiveTexture(GL_TEXTURE0);
        float *texCoords = (float*)&m_VertexBuffer[1];
        // only use u/v coords, skip t/s - stride is from n[0] + offset = n[1]
        glTexCoordPointer( 2, GL_FLOAT, m_Stride*sizeof(Vector), texCoords );

        glActiveTexture(GL_TEXTURE0);
        m_Textures[BASE_TEXTURE]->Enable();
    }
    // Lightmap. Simple RGB blend it into previous texture
    if ( m_Textures[LIGHT_MAP] ) {
        glClientActiveTexture(GL_TEXTURE1);
        float *texCoords = (float*)&m_VertexBuffer[1];
        // only use u/v coords, skip t/s - stride is from n[0] + offset = n[1]
        glTexCoordPointer( 2, GL_FLOAT, m_Stride*sizeof(Vector), texCoords );

        glActiveTexture(GL_TEXTURE1);
        m_Textures[LIGHT_MAP]->Enable();

        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_BLEND);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    }
    int *indices = &m_IndexArray[0];
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, indices );

    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }

    if ( m_Textures[BASE_TEXTURE] ) {
        m_Textures[BASE_TEXTURE]->Disable();
    }
    if ( m_Textures[LIGHT_MAP] ) {
        m_Textures[LIGHT_MAP]->Disable();
    }
    if (!m_TexCoordArrayEnabled) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

}

void Flag::DoUpdate( float ticks ) throw(std::exception)
{
    m_TimeEllapsed += ticks;
    if ( m_TimeEllapsed*m_Speed > 16.67 )
    {
        m_TimeEllapsed = 0;
        // size must be > 2
        auto first = m_VertexBuffer[0]; // backup first vertex
        for ( auto vit = m_VertexBuffer.begin(); vit != m_VertexBuffer.end();  ) {
            auto& vertex = *vit; vit += m_Stride;
            vertex[ Vector::Z ] = (*vit)[ Vector::Z ];
        }
        auto& last = m_VertexBuffer[ columns*rows*m_Stride-m_Stride];
        last[ Vector::Z ] = first[ Vector::Z ];

        // This is one example why we are using a custom Vector array instead of simple floats.
        // A vector is a Vertex with a size of float[4] and can overlay a simple vertex array
        // But we have all luxury a vector has, e.g. normalizing, dot and cross product, etc.

        // use mid point of mesh
        int pitch  = columns*m_Stride; // pair of Vertex + TexCoord
        int x = columns/2*m_Stride;
        int y = rows/2;
        auto mid = m_VertexBuffer[ x + y * pitch ]; // stride = 2*Vector - we have a pair here, Vertex+Tex
        // let it "swim"
        m_Child ->GetRenderState()->GetMatrix().LoadIdentity().Translate( mid );
    }
}
