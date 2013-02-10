sdl-multi-texture

Author:

	Jurgen Schober
	
Date:
   
	January, 2013
	
Short:
  
	Example bump mapping/multi textures with OpenGL.

Description:

	Based on sdl-vertex-animation this example uses code from previous sdl-xx-examples I've written.
	
	We use a base texture and a second one as a light map simply blending it over the base texture.
	
	No shaders are used. Stock OGL 2.x
	
	All sdl-xx-examples are written in C++11 using MinGW gcc 4.6 and are Windows only. I'm using
	Eclipse Juno as Development IDE.

Libs used:

	boost_thread
	boost_system
	boost_filesystem
	boost_iostreams
	glew
	+ OpenGL

License:

	Use as is. No license other then the ones included with third party libraries are required.

Compiler used:

	MinGW with Eclipse Juno. Windows only. Linux might just work, MacOS will need some work due to 
	the fact OSX needs to run the render loop in the main loop. This example runs a render thread decoupled
	from the main thread.

Have fun
Jurgen
