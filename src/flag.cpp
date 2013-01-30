/*
 * cube.cpp
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */

#if 0
#include "config.h"
#include "wave_mesh.h"

#include <render/tex_surface.h>

namespace wave {

	static GLfloat points[45][45][3];    // the array for the points on the grid of our "wave"

	class VertexTarget : public pei::render::VertexArrayRenderTarget
	{
	public:
		void AllocateVertexBuffer( unsigned int num )
		{
			m_Vertices.resize( num );
		}
		GLfloat* GetVertexBuffer() { return &m_Vertices.at(0); }

		GLfloat* GetTexCoordBuffer() { return &m_TextureCoords.at(0); }
	};

	WaveMesh::WaveMesh()
		: amp(4)
		, xrot(0)
    	, yrot()
    	, zrot()
		, m_TimeEllapsed(0)
		, m_Speed(0.75)
	{
		VertexTarget *rt = new VertexTarget;
		SetRenderTarget( pei::RenderTargetInterfacePtr(rt) );
	    float float_x, float_y; // loop counters.

	    for(float_x = 0.0f; float_x < 9.0f; float_x +=  0.2f )	{
			for(float_y = 0.0f; float_y < 9.0f; float_y += 0.2f)		{
				points[ (int) (float_x*5) ][ (int) (float_y*5) ][0] = float_x - 4.4f;
				points[ (int) (float_x*5) ][ (int) (float_y*5) ][1] = float_y - 4.4f;
				points[ (int) (float_x*5) ][ (int) (float_y*5) ][2] = (float) (sin( ( (float_x*5*8)/360 ) * 3.14159 * amp ));
			}
	    }

		// indices and texCoords remain static, only vertices will be updated
	    std::vector<unsigned int> indexArray;
	    int indexOffset(0);
	    int x, y;
		unsigned int numVertices(0);
	    float float_xb, float_yb;
	    for (x=0; x<44; x++) {
			for (y=0; y<44; y++) {
				float_x  = (float) (x)/44;
				float_y  = (float) (y)/44;
				float_xb = (float) (x+1)/44;
				float_yb = (float) (y+1)/44;

				texCoords.push_back(float_x );  texCoords.push_back( float_y);
				texCoords.push_back(float_x );  texCoords.push_back( float_yb);
				texCoords.push_back(float_xb ); texCoords.push_back( float_yb);
				texCoords.push_back(float_xb ); texCoords.push_back( float_y);

				// must restart poly for proper texture mapping...optimize this
				indexArray.push_back(0+numVertices);
				indexArray.push_back(1+numVertices);
				indexArray.push_back(3+numVertices);
				indexArray.push_back(3+numVertices);
				indexArray.push_back(1+numVertices);
				indexArray.push_back(2+numVertices);
				// use TRIANGLES, cannot use TRI_STRIPS - triangle getting merged into 1 draw call, strips don't
				rt->AddPolygon( GL_TRIANGLES, 6, indexOffset );
				indexOffset += 6;
				numVertices += 4;
			}
	    }
		rt->SetTexCoords(texCoords.size(), &texCoords.at(0) );
		rt->SetIndices( indexArray.size(), &indexArray.at(0));
		rt->AllocateVertexBuffer( numVertices*3 );
		SetColor( pei::Color( 1.0,1.0,1.0 ));
		RequestStateChanged( true );
	}

	WaveMesh::~WaveMesh()
	{
	}

	pei::SurfacePtr WaveMesh::SetSurface(pei::SurfacePtr s, int n)
	{
		pei::Drawable::SetSurface( s, n );
#if 0
		// read back texture
		pei::Texture2DSurfacePtr tex = boost::dynamic_pointer_cast< pei::Texture2DSurface >( GetSurface(0) );
		if ( tex ) {
			// We must re calc texture coords for each "frame" we attach to the cube - for non pow2 surfaces!
//			int numCoords = texCoords.size();
			VertexTarget *rt = (VertexTarget*)(GetRenderTarget().get());
			GLfloat *texCoords = rt->GetTexCoordBuffer();
			const pei::TexSurface::TexCoords &triCoords = tex->GetCoords();

			int n(0);
			float x,y;
		    float tx0, tx1, ty0, ty1;
		    float dx = (triCoords.MaxX-triCoords.MinX)/44;
		    float dy = (triCoords.MaxY-triCoords.MinY)/44;

		    for (x = triCoords.MinX; x < triCoords.MaxX; x += dx ) {
				for (y = triCoords.MinY; y < triCoords.MaxY; y += dy) {
					tx0 = x;
					ty0 = y;
					tx1 = x + dx;
					ty1 = y + dy;

					texCoords[ n++ ] = tx0; texCoords[ n++ ] = ty0;
					texCoords[ n++ ] = tx0; texCoords[ n++ ] = ty1;
					texCoords[ n++ ] = tx1; texCoords[ n++ ] = ty1;
					texCoords[ n++ ] = tx1; texCoords[ n++ ] = ty0;
				}
		    }
		}
#endif
		return s;
	}

	void WaveMesh::OnUpdateAnimation( const pei::RenderProfilePtr& profile, double time )
	{
		m_TimeEllapsed += time;
	    if ( m_TimeEllapsed*m_Speed > 16.67 ) {
	    	m_TimeEllapsed = 0;
			int x, y;
			for (y = 0; y <45; y++) {
				points[44][y][2] = points[0][y][2];
			}

			for( x = 0; x < 44; x++ ) {
				for( y = 0; y < 45; y++) {
					points[x][y][2] = points[x+1][y][2];
				}
			}
			// no point in updating if vertices don't change
			RequestStateChanged( true );

//		    xrot +=0.3f;
//		    yrot +=0.2f;
//		    zrot +=0.4f;
		    SetRotation( GetXRotation(), GetYRotation() + 0.2f , GetZRotation() );
	    }
	}

	void WaveMesh::OnUpdateState( const pei::RenderProfilePtr& profile )
	{
		if ( MustRefreshState( ) )
		{
			VertexTarget *rt = (VertexTarget*)(GetRenderTarget().get());
			GLfloat *vertexArray = rt->GetVertexBuffer();
			int x, y;
			int idx(0);
			for (x=0; x<44; x++) {
				for (y=0; y<44; y++) {
					vertexArray[idx++] = ( points[x][y][0] );
					vertexArray[idx++] = ( points[x][y][1] );
					vertexArray[idx++] = ( points[x][y][2] );

					vertexArray[idx++] = ( points[x][y+1][0] );
					vertexArray[idx++] = ( points[x][y+1][1] );
					vertexArray[idx++] = ( points[x][y+1][2] );

					vertexArray[idx++] = ( points[x+1][y+1][0] );
					vertexArray[idx++] = ( points[x+1][y+1][1] );
					vertexArray[idx++] = ( points[x+1][y+1][2] );

					vertexArray[idx++] = ( points[x+1][y][0] );
					vertexArray[idx++] = ( points[x+1][y][1] );
					vertexArray[idx++] = ( points[x+1][y][2] );
				}
			}
		}
	}

	void WaveMesh::OnDraw( const pei::RenderProfilePtr& profile, const pei::SurfacePtr& buffer, const pei::RenderParam& param  )
	{
		pei::Drawable::OnDraw( profile, buffer, param );
	}

}
#endif
