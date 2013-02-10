/*
 * template.cpp
 *
 *  Created on: Jan 28, 2013
 *      Author: Jurgen
 */

#include "vector.h"

#include <GL/glew.h>

const float MAX_EMBOSS = 0.01f;

// sample code from NeHe
// http://nehe.gamedev.net/tutorial/bump-mapping,_multi-texturing_&_extensions/16009/

#if 0
// Sets Up The Texture-Offsets
// n : Normal On Surface. Must Be Of Length 1
// c : Current Vertex On Surface
// l : Lightposition
// s : Direction Of s-Texture-Coordinate In Object Space (Must Be Normalized!)
// t : Direction Of t-Texture-Coordinate In Object Space (Must Be Normalized!)
void SetUpBumps(GLfloat *n, GLfloat *c, GLfloat *l, GLfloat *s, GLfloat *t) {
    GLfloat v[3];                               // Vector From Current Position To Light
    GLfloat lenQ;                               // Used To Normalize
    // Calculate v From Current Vertex c To Lightposition And Normalize v
    v[0]=l[0]-c[0];
    v[1]=l[1]-c[1];
    v[2]=l[2]-c[2];
    // -js- normal projection is missing...
    lenQ=(GLfloat) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0]/=lenQ;
    v[1]/=lenQ;
    v[2]/=lenQ;
    // Project v Such That We Get Two Values Along Each Texture-Coordinate Axis
    c[0]=(s[0]*v[0]+s[1]*v[1]+s[2]*v[2])*MAX_EMBOSS;
    c[1]=(t[0]*v[0]+t[1]*v[1]+t[2]*v[2])*MAX_EMBOSS;
}
#endif

Vector SetUpBumps( const Vector& surfaceNormal, const Vector& vertex, const Vector& lightPos, const Vector& s, const Vector& t )
{
    Vector v( lightPos - vertex );
    v.Normalize();

    // s and t vectors are the the directional union vectors - not texture coords - or trasformed into object space
    return Vector( s.Dot( v )*MAX_EMBOSS, t.Dot( v )*MAX_EMBOSS, 0, 0 );
}


// Texture setup for one face

//TODO: Don't use quads
GLfloat data[]= {
    // FRONT FACE
    0.0f, 0.0f,     -1.0f, -1.0f, +1.0f, // texCoord - vertex
    1.0f, 0.0f,     +1.0f, -1.0f, +1.0f, // texCoord - vertex
    1.0f, 1.0f,     +1.0f, +1.0f, +1.0f, // texCoord - vertex
    0.0f, 1.0f,     -1.0f, +1.0f, +1.0f, // texCoord - vertex
};

Vector vertexArray[] =
{
    { -1.0f, -1.0f, +1.0f },
    { +1.0f, -1.0f, +1.0f },
    { +1.0f, +1.0f, +1.0f },
    { -1.0f, +1.0f, +1.0f },
};

Vector texArray[] =
{
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f }
};

Vector normalArray[] =
{
    { 0, 0, 1 }
};

void SetFace() {
#if 0
    // Front Face
    n[0]=0.0f;
    n[1]=0.0f;
    n[2]=1.0f;
    s[0]=1.0f;
    s[1]=0.0f;
    s[2]=0.0f;
    t[0]=0.0f;
    t[1]=1.0f;
    t[2]=0.0f;
    for (i=0; i<4; i++) {
        c[0]=data[5*i+2];
        c[1]=data[5*i+3];
        c[2]=data[5*i+4];
        SetUpBumps(n,c,l,s,t);
        glTexCoord2f(data[5*i]+c[0], data[5*i+1]+c[1]);
        glVertex3f(data[5*i+2], data[5*i+3], data[5*i+4]);
    }
#endif

}
