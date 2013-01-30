/*
 * cube.h
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */

#ifndef MESH_H
#define MESH_H

#if 0
#include <render/drawable.h>

#include <vector>

namespace wave
{
	class WaveMesh : public pei::Drawable
	{
	    std::vector<GLfloat> texCoords;
	    double 	 amp;
	    float 	 xrot, yrot, zrot;
	    double 	 m_TimeEllapsed;
	    double	 m_Speed;
	public:
		WaveMesh();

		virtual ~WaveMesh();

		virtual pei::SurfacePtr SetSurface( pei::SurfacePtr s, int n = 0 );

	protected:
		virtual void OnUpdateAnimation( const pei::RenderProfilePtr& profile, double time );

        virtual void OnUpdateState( const pei::RenderProfilePtr& profile );

		virtual void OnDraw( const pei::RenderProfilePtr& profile, const pei::SurfacePtr& buffer, const pei::RenderParam& param  );
	};

}

#endif

#endif /* MESH_H */

