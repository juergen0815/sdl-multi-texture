/*
 * app.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "err.h"
#include "app.h"
#include "renderer.h"

#include "viewport.h"
#include "camera.h"
#include "cube.h"

#include <bmp_loader.h>
#include <dds_loader.h>
#include <tga_loader.h>

#include <SDL/SDL.h>

#include <GL/glew.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

// based on NeHe: http://nehe.gamedev.net/tutorial/bump-mapping,_multi-texturing_&_extensions/16009/

App::App()
    : m_Worker(new Renderer)
    , m_Joystick(nullptr)
{
}

App::~App()
{
    if ( m_Joystick ) SDL_JoystickClose(m_Joystick);
}

bool OnHandleCubeKeyEvent( const SDL_Event& event, EntityPtr c1, EntityPtr c2 )
{
    ASSERT( c1 && c2, "No entities in event handler!" );

    bool processed(false);
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
            c1->GetRenderState()->Rotate( {0, -10, 0 } );
            c2->GetRenderState()->Rotate( {0, -10, 0 } );
            processed = true;
            break;
        case SDLK_RIGHT:
            c1->GetRenderState()->Rotate( {0, 10, 0 } );
            c2->GetRenderState()->Rotate( {0, 10, 0 } );
            processed = true;
            break;
        case SDLK_UP:
            c1->GetRenderState()->Rotate( {-10, 0, 0 } );
            c2->GetRenderState()->Rotate( {-10, 0, 0 } );
            processed = true;
            break;
        case SDLK_DOWN:
            c1->GetRenderState()->Rotate( {10, 0, 0 } );
            c2->GetRenderState()->Rotate( {10, 0, 0 } );
            processed = true;
            break;
        case SDLK_HOME:
            c1->GetRenderState()->GetMatrix().LoadIdentity();
            c1->GetRenderState()->Translate( Vector(-7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
            c2->GetRenderState()->GetMatrix().LoadIdentity();
            c2->GetRenderState()->Translate( Vector(+7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
            processed = true;
            break;
        default:
            break;
        }
        break;
    case SDL_JOYBUTTONDOWN:
        switch ( event.jbutton.button ) {
        case JOY_BUTTONS::START:
            c1->GetRenderState()->GetMatrix().LoadIdentity();
            c1->GetRenderState()->Translate( Vector(-7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
            c2->GetRenderState()->GetMatrix().LoadIdentity();
            c2->GetRenderState()->Translate( Vector(+7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
            processed = true;
            break;
        default:
            break;
        }
        break;
    case SDL_JOYBUTTONUP:
        switch ( event.jbutton.button ) {
        case 0:
            break;
        default:
            break;
        }
        break;
    case SDL_JOYHATMOTION:
        switch ( event.jhat.value )
        {
        case SDL_HAT_LEFT:
            c1->GetRenderState()->Rotate( {0, -10, 0 } );
            c2->GetRenderState()->Rotate( {0, -10, 0 } );
            processed = true;
            break;
        case SDL_HAT_RIGHT:
            c1->GetRenderState()->Rotate( {0,  10, 0 } );
            c2->GetRenderState()->Rotate( {0,  10, 0 } );
            processed = true;
            break;
        case SDL_HAT_UP:
            c1->GetRenderState()->Rotate( {-10, 0, 0 } );
            c2->GetRenderState()->Rotate( {-10, 0, 0 } );
            processed = true;
            break;
        case SDL_HAT_DOWN:
            c1->GetRenderState()->Rotate( { 10, 0, 0 } );
            c2->GetRenderState()->Rotate( { 10, 0, 0 } );
            processed = true;
            break;
        }
        break;
    default:
        break;
    }
    return processed;
}

void App::InitScene( int width, int height )
{
    Renderer* renderer = dynamic_cast<Renderer*>(m_Worker.get());
    BOOST_ASSERT(renderer);
    renderer->Init();

    std::vector< BrushPtr > brush1;
    try {
        // base texture
        TgaBrush* base( new TgaBrush );
        ASSERT( base->Load( "data/Fieldstone.tga"), "Error loading base texture" );
        brush1.push_back( BrushPtr(base) );

        BmpBrush* normal( new BmpBrush );
        ASSERT( normal->Load( "data/FieldstoneNoisy.bmp"), "Error loading normal maps" );
        brush1.push_back( BrushPtr(normal) );

        // TODO: add 8 bit support for BMP loader
        DdsBrush* alpha( new DdsBrush );
        ASSERT( alpha->Load( "data/ScratchMetal.dds"), "Error loading specular maps" );
        brush1.push_back( BrushPtr(alpha) );

    } catch ( boost::filesystem::filesystem_error &ex ) {
        throw;
    } catch ( std::ios_base::failure& ex ) {
        THROW( "Error loading texture.\n%s", ex.what() );
    } catch ( std::exception &ex ) {
        throw;
    } catch ( ... ) {
        throw;
    }

    std::vector< BrushPtr > brush2;
    try {
        // base texture
        DdsBrush* base( new DdsBrush );
        ASSERT( base->Load( "data/MetalDecoA.dds"), "Error loading base texture" );
        brush2.push_back( BrushPtr(base) );

        BmpBrush* normal( new BmpBrush );
        ASSERT( normal->Load( "data/rocks1.bmp"), "Error loading normal maps" );
        brush2.push_back( BrushPtr(normal) );

//        BmpBrush* alpha( new BmpBrush );
//        ASSERT( alpha->Load( "data/MetalDecoB_SPEC.bmp"), "Error loading specular maps" );
//        brush2.push_back( BrushPtr(alpha) );
    } catch ( boost::filesystem::filesystem_error &ex ) {
        throw;
    } catch ( std::ios_base::failure& ex ) {
        THROW( "Error loading texture.\n%s", ex.what() );
    } catch ( std::exception &ex ) {
        throw;
    } catch( ... ) {
        throw;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Compose our scene

    // Add a viewport
    EntityPtr viewport(new Viewport(width, height));
    // this entity renders
    renderer->AddEntity(viewport);
    // listen to resize events
    m_EntityEventHandlerList.push_back( viewport );

    // Add the camera
    EntityPtr camera(new Camera(m_Joystick));
    // this entity handles events
    m_EntityEventHandlerList.push_back( camera );
    // this entity renders
    viewport->AddEntity(camera, 0);

    EntityPtr cube( new Cube( brush1 ) );
    cube->GetRenderState()->Translate( Vector(-7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
    cube->GetRenderState()->Rotate( Vector(0.0f, 0.0f, 0.0f ) );
    // this entity renders
    camera->AddEntity(cube, 20 );

    EntityPtr cube2( new Cube( brush2 ) );
    cube2->GetRenderState()->Translate( Vector(+7.0, 0, 10), Vector(2.0f, 2.0f, 2.0f) );
    cube2->GetRenderState()->Rotate( Vector(0.0f, 0.0f, 0.0f ) );
    // this entity renders
    camera->AddEntity(cube2, 20 );

    // some custom event handler
    m_EventHandlerList.push_back( boost::bind( &OnHandleCubeKeyEvent, _1, cube, cube2 ) );

}

void App::Init(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0)
    {
        THROW( "Failed to initialize SDL video system! SDL Error: %s\n", SDL_GetError());
    }
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    int stencilSize(8);
    if (SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilSize) == -1)
    {
        THROW("Error setting stencil size to %d! SDL Error:  %s\n", stencilSize, SDL_GetError());
    }
    // enable multi sampling
    if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) == -1)
    {
        THROW("Error enabling multi sampling! SDL Error: %s\n", SDL_GetError());
    }
    int numSampleBuffers(8); // test what's the max AA. test 8xMSAA
    if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, numSampleBuffers) == -1)
    {
        THROW("Error setting number (%d) of AA buffer! SDL Error: %s\n", numSampleBuffers, SDL_GetError());
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int vsync = 1;  // 0 = novsync
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);

    SDL_WM_SetCaption("SDLFW", NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    //  SDL_WM_SetIcon( pei::SDL::SurfaceRef( pei::LogoBrush ), NULL );
    //	SDL_ShowCursor(SDL_DISABLE);
    //	SDL_EnableUNICODE(1);

    int numJoysticks = SDL_NumJoysticks();
    if ( numJoysticks > 0 ) {
        m_Joystick = SDL_JoystickOpen(0);
        SDL_JoystickEventState(SDL_ENABLE);
    }

}

int App::Run()
{
    int r(0);

    // somebody must attach a worker
    BOOST_ASSERT( m_Worker);

    int width(960);
    int height(540);
    SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL|SDL_RESIZABLE);
    if (screen == NULL)
    {
        THROW("Unable to set %dx%d video! SDL Error: %s\n", width, height, SDL_GetError());
    }

    InitScene(width, height);

    // Run our worker thread
    boost::thread worker(boost::bind(&Worker::Run, m_Worker));
    Renderer* renderer = static_cast<Renderer*>(m_Worker.get());

    bool running(true);
    SDL_Event event;
    do
    {
        int eventsPending(0);
        SDL_WaitEvent(&event);
        do
        {
            bool processed( renderer->HandleEvent( event ) );
            for ( auto entity = m_EntityEventHandlerList.begin(); entity != m_EntityEventHandlerList.end(); )
            {
                processed |= (*entity)->HandleEvent(event);
                if (processed) {
                    break;
                }
                // Remove from event handler as well if marked for deletion
                if ( (*entity)->AreFlagsSet( Entity::F_DELETE ) ) {
                    entity = m_EntityEventHandlerList.erase( entity );
                    continue;
                }
                ++entity;
            }
            if (!processed)
            {
                for ( auto eventHandler : m_EventHandlerList ) {
                    processed |= eventHandler( event );
                    if (processed) {
                        break;
                    }
                }
            }
            if (!processed)
            {
                switch (event.type)
                {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_JOYBUTTONUP:
                    switch ( event.jbutton.button ) {
                    case JOY_BUTTONS::START:
                        running = false;
                        break;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                }
            }
            if (running)
            {
                eventsPending = SDL_PollEvent(&event);
            }
        } while (eventsPending > 0 && running);
    } while (running);

    // Need to clear this list before renderer destroys entities
    m_EventHandlerList.clear();

    m_Worker->Terminate();
    worker.join();

    return r;
}

