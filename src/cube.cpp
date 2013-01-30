
/*
 * cube.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "cube.h"
#include "err.h"

// cube ///////////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

// normal array
GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v6-v5-v4

                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7
};

// vertex coords array for glDrawArrays() =====================================
// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
// array is 108 floats (36 * 3 = 108).
GLfloat vertices[]  = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,      // v0-v1-v2 (front)
                       -1,-1, 1,   1,-1, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1,-1, 1,   1,-1,-1,      // v0-v3-v4 (right)
                        1,-1,-1,   1, 1,-1,   1, 1, 1,      // v4-v5-v0

                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,      // v4-v7-v6 (back)
                       -1, 1,-1,   1, 1,-1,   1,-1,-1,      // v6-v5-v4

                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,      // v1-v6-v7 (left)
                       -1,-1,-1,  -1,-1, 1,  -1, 1, 1,      // v7-v2-v1

                        1, 1, 1,   1, 1,-1,  -1, 1,-1,      // v0-v5-v6 (top)
                       -1, 1,-1,  -1, 1, 1,   1, 1, 1,      // v6-v1-v0

                       -1,-1,-1,   1,-1,-1,   1,-1, 1,      // v7-v4-v3 (bottom)
                        1,-1, 1,  -1,-1, 1,  -1,-1,-1,      // v3-v2-v7
};
// color array
GLfloat colors[]    = { 1, 1, 1, 1,   1, 1, 0, 1,   1, 0, 0, 1,      // v0-v1-v2 (front)
                        1, 0, 0, 1,   1, 0, 1, 1,   1, 1, 1, 1,      // v2-v3-v0

                        1, 1, 1, 1,   1, 0, 1, 1,   0, 0, 1, 1,      // v0-v3-v4 (right)
                        0, 0, 1, 1,   0, 1, 1, 1,   1, 1, 1, 1,      // v4-v5-v0

                        0, 0, 1, 1,   0, 0, 0, 1,   0, 1, 0, 1,      // v4-v7-v6 (back)
                        0, 1, 0, 1,   0, 1, 1, 1,   0, 0, 1, 1,      // v6-v5-v4

                        1, 1, 0, 1,   0, 1, 0, 1,   0, 0, 0, 1,      // v1-v6-v7 (left)
                        0, 0, 0, 1,   1, 0, 0, 1,   1, 1, 0, 1,      // v7-v2-v1

                        1, 1, 1, 1,   0, 1, 1, 1,   0, 1, 0, 1,      // v0-v5-v6 (top)
                        0, 1, 0, 1,   1, 1, 0, 1,   1, 1, 1, 1,      // v6-v1-v0

                        0, 0, 0, 1,   0, 0, 1, 1,   1, 0, 1, 1,      // v7-v4-v3 (bottom)
                        1, 0, 1, 1,   1, 0, 0, 1,   0, 0, 0, 1,      // v3-v2-v7
};
GLfloat texCoords[] = { 1,1,  0,1,  0,0,      // v0-v1-v2 (front)
                        0,0,  1,0,  1,1,      // v2-v3-v0

                        0,1,  0,0,  1,0,      // v0-v3-v4 (right)
                        1,0,  1,1,  0,1,      // v4-v5-v0

                        0,0,  1,0,  1,1,      // v4-v7-v6 (back)
                        1,1,  0,1,  0,0,      // v6-v5-v4

                        1,1,  0,1,  0,0,      // v1-v6-v7 (left)
                        0,0,  1,0,  1,1,      // v7-v2-v1

                        1,0,  1,1,  0,1,      // v0-v5-v6 (top)
                        0,1,  0,0,  1,0,      // v6-v1-v0

                        1,1,  0,1,  0,0,      // v7-v4-v3 (bottom)
                        0,0,  1,0,  1,1,      // v3-v2-v7
};

Cube::Cube()
    : m_VboID(0)
{
    GetRenderState()->SetFlag( BLEND_COLOR_F );
}

Cube::Cube( std::vector<BrushPtr> assetList )
    : m_VboID(0)
    , m_Assets( assetList )
{
    GetRenderState()->SetFlag( BLEND_COLOR_F );
}

Cube::~Cube()
{
    if ( m_VboID > 0 ) {
        glDeleteBuffers(1, &m_VboID);
    }
}

bool Cube::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    bool hasVBO = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );

    if ( m_Assets.size() ) {
        for (auto& asset : m_Assets ) {
            TexturePtr texture( new Texture );
            texture->Load( *asset.get() );
            m_Textures.push_back( texture );
        }
        GetRenderState()->ClearFlag( BLEND_COLOR_F );
    }

    glGenBuffers(1, &m_VboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors)+sizeof(texCoords), 0, GL_STATIC_DRAW_ARB);
    std::size_t offset(0);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
    offset += sizeof(vertices);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals), normals);                // copy normals after vertices
    offset += sizeof(normals);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors);  // copy colors after normals
    if ( m_Textures.size() ) {
        offset += sizeof(texCoords);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(texCoords), texCoords);  // copy colors after normals
    }
    return true;
}

void Cube::DoRender( ) throw(std::exception)
{
    int normalArrayEnabled;
    int colorArrayEnabled;
    int vertexArrayEnabled;
    int texCoordArrayEnabled;

    // enable vertex arrays
    glGetIntegerv( GL_NORMAL_ARRAY, &normalArrayEnabled );
    if ( !normalArrayEnabled )  {
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    glGetIntegerv( GL_COLOR_ARRAY, &colorArrayEnabled );
    if ( !colorArrayEnabled && (GetRenderState()->GetFlags() & BLEND_COLOR_F) ) {
        glEnableClientState(GL_COLOR_ARRAY);
    }
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    glGetIntegerv( GL_TEXTURE_COORD_ARRAY, &texCoordArrayEnabled );
    if ( m_Textures.size() && !texCoordArrayEnabled ) {
        if (!texCoordArrayEnabled) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    int blend_enabled;
    glGetIntegerv(GL_BLEND, &blend_enabled);

    // Render with VBO - if available
    glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
    // before draw, specify vertex and index arrays with their offsets
    std::size_t offset(0);
    glVertexPointer(3, GL_FLOAT, 0, (void*)offset); offset += sizeof(vertices);
    glNormalPointer(   GL_FLOAT, 0, (void*)offset); offset += sizeof(normals);
    glColorPointer (4, GL_FLOAT, 0, (void*)offset);
    if ( m_Textures.size()) {
        m_Textures[0]->Enable();
        offset += sizeof(texCoords);
        glTexCoordPointer(2, GL_FLOAT, 0, (void*)offset );
        if (blend_enabled) {
            glDisable( GL_BLEND );
        }
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( m_Textures.size() ) {
        m_Textures[0]->Disable();
        if (!texCoordArrayEnabled) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (blend_enabled) {
            glEnable( GL_BLEND );
        }
    }
    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }
    if (!colorArrayEnabled)   {
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (!normalArrayEnabled) {
        glDisableClientState(GL_NORMAL_ARRAY);
    }

}
