/*
 * cube.cpp
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */


#include "flag.h"
#include "cube.h"
#include "brush.h"
#include <bmp_loader.h>

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
    try {
        // base texture
        BmpBrush* base( new BmpBrush );
        ASSERT( base->Load( "data/Wood.bmp"), "Error loading wood texture" );
        brushes.push_back( BrushPtr(base) );
    } catch ( boost::filesystem::filesystem_error &ex ) {
        throw;
    } catch ( std::ios_base::failure& ex ) {
        THROW( "Error loading texture.\n%s", ex.what() );
    } catch ( std::exception &ex ) {
        throw;
    } catch ( ... ) {
        throw;
    }

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

    if ( m_Assets.size() ) {
        for (auto& asset : m_Assets ) {
            m_Texture = TexturePtr( new Texture );
            m_Texture->Load( *asset.get() );
            break; // no multi texture at this time
        }
        GetRenderState()->ClearFlag( BLEND_COLOR_F );
    }

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
#if 0
        offset += sizeof(vertices);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);                // copy normals after vertices
        offset += sizeof(normals);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors);  // copy colors after normals
#endif
//        if ( m_Texture ) {
//            float *texCoords = (float*)&m_VertexBuffer[0];
//            glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_TexCoordBuffer.size(), texCoords);  // copy colors after normals
//        }
    }
    r = true;

    return r;
}

void Flag::DoRender() throw(std::exception)
{
#if _HAS_NORMALS_
    // enable vertex arrays
    int normalArrayEnabled;
    glGetIntegerv( GL_NORMAL_ARRAY, &normalArrayEnabled );
    if ( !normalArrayEnabled )  {
        glEnableClientState(GL_NORMAL_ARRAY);
    }
#endif
#if _HAS_COLOR_ARRAY_
    int colorArrayEnabled;
    glGetIntegerv( GL_COLOR_ARRAY, &colorArrayEnabled );
    if ( !colorArrayEnabled && (GetRenderState()->GetFlags() & BLEND_COLOR_F) ) {
        glEnableClientState(GL_COLOR_ARRAY);
    }
#endif

    int vertexArrayEnabled;
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    int m_TexCoordArrayEnabled;
    glGetIntegerv( GL_TEXTURE_COORD_ARRAY, &m_TexCoordArrayEnabled );
    if ( m_Texture && !m_TexCoordArrayEnabled ) {
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
//    glNormalPointer(   GL_FLOAT, 0, (void*)offset); offset += sizeof(normals);
//    glColorPointer (4, GL_FLOAT, 0, (void*)offset);
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
#if _HAS_NORMALS_
    glNormalPointer(  GL_FLOAT, 0, normals);
#endif
#if _HAS_COLOR_ARRAY_
    glColorPointer(4, GL_FLOAT, 0, colors);
#else
//    glColor4f( 0.0f, 0.4f, 1.0f, 0.8f );
#endif
    if ( m_Texture) {
        m_Texture->Enable();
        float *texCoords = (float*)&m_VertexBuffer[1];
        // only use u/v coords, skip t/s - stride is from n[0] + offset = n[1]
        glTexCoordPointer( 2, GL_FLOAT, m_Stride*sizeof(Vector), texCoords );
    }

    int *indices = &m_IndexArray[0];
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, indices );

#endif
    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }
    if ( m_Texture ) {
        m_Texture->Disable();
        if (!m_TexCoordArrayEnabled) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (blend_enabled) {
            glEnable( GL_BLEND );
        }
    }
#if _HAS_NORMALS_
    if (!colorArrayEnabled)   {
        glDisableClientState(GL_COLOR_ARRAY);
    }
#endif
#if _HAS_COLOR_ARRAY_
    if (!normalArrayEnabled) {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
#endif
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
