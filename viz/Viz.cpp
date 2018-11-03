// Copyright (c) 2017-2019 Robert A. Alfieri
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 

#include "ConfigViz.h"
#include "Viz.h"
#include "Color.h"
#include "Sys.h"
#include "Misc.h"
#include "Node.h"
#include "Box.h"

#undef dprintf
#define dprintf if ( 0 ) printf

//--------------------------------------------
// Internal Implementation Structure
//--------------------------------------------
class Viz::Impl
{
public:
    ConfigViz *         config;
    Sys *               sys;

    //------------------------------------------------------------
    // Viz Info
    //------------------------------------------------------------
    NodeIO *            viz_nodeio;                                                     // handle on parser for viz file
    List *              viz_list;                                                       // listof things to visualize
    Entity **           viz_entities;                                                   // allocated World entities
    Hash                viz_id_to_entity_index;                                         // maps id to index into viz_entrities
    int                 viz_last;                                                       // draw everything through this position in list

    int                 id_line;
    int                 id_kind;
    int                 id_shape;
    int                 id_color;
    int                 id_x;
    int                 id_y;
    int                 id_z;
    int                 id_w;
    int                 id_h;
    int                 id_d;
    int                 id_index;

    //------------------------------------------------------------
    // GUI
    //------------------------------------------------------------
    bool                gui_recompute;                  // true to force recompute of entire window content
    int                 gui_mouse_x;                    // current mouse X
    int                 gui_mouse_y;                    // current mouse Y

    void                gui_init( void );               // one-time iniialization
};

//----------------------------------------------------------------
// Initialization
//----------------------------------------------------------------
Viz::Viz( ConfigViz * config, Sys * sys ) 
        : World( config, sys, 1000.0f, 1000.0f, 1000.0f )
{
    impl = new Viz::Impl();

    impl->config = config;
    impl->sys = sys;
    sys->world_set( this );

    //----------------------------------------------------------------
    // read in viz_file
    //----------------------------------------------------------------
    if ( !impl->config->viz_path ) error( "no -viz_path supplied" );
    impl->viz_nodeio = new NodeIO( impl->config->viz_path );
    impl->viz_list = impl->viz_nodeio->list_parse();

    //----------------------------------------------------------------
    // Get fast ids for hash field names that we really care about.
    //----------------------------------------------------------------
    impl->id_line  = Hash::str_to_id( "line" ); 
    impl->id_kind  = Hash::str_to_id( "kind" ); 
    impl->id_shape = Hash::str_to_id( "shape" );
    impl->id_color = Hash::str_to_id( "color" );
    impl->id_x     = Hash::str_to_id( "x" );
    impl->id_y     = Hash::str_to_id( "y" );
    impl->id_z     = Hash::str_to_id( "z" );
    impl->id_w     = Hash::str_to_id( "w" );
    impl->id_h     = Hash::str_to_id( "h" );
    impl->id_d     = Hash::str_to_id( "d" );
    impl->id_index = Hash::str_to_id( "index" );

    //----------------------------------------------------------------
    // Prep the visualization.
    // Initial time is set to 0.
    // Create objects.
    // Anything after the initial time is marked invisible.
    //----------------------------------------------------------------
    int len = impl->viz_list->length();
    impl->viz_last = impl->config->viz_last;
    if ( impl->viz_last < 0 ) impl->viz_last = 0;
    if ( impl->viz_last >= len ) impl->viz_last = len - 1;
    impl->viz_entities = new Entity*[len];
    for( int i = 0; i < len; i++ )
    {
        impl->viz_entities[i] = nullptr;
    }
    //printf( "Setting up shapes for %d viz_list entries...\n", len );
    for( int i = 0; i < len; i++ ) 
    {
        //if ( (i % 1000) == 0 ) printf( "%d\n", i );
        Hash * obj = impl->viz_list->hp( i );
        nStr kind = obj->s( impl->id_kind );
        bool is_visible = i <= impl->viz_last;
        if ( strcmp( kind, "geom" ) == 0 ) {
            // shape
            Hash * shape = obj->hp( impl->id_shape ); 
            nStr shape_kind = shape->s( impl->id_kind );
            if ( strcmp( shape_kind, "box" ) == 0 ) {
                nFlt x = shape->f( impl->id_x );
                nFlt y = shape->f( impl->id_y );
                nFlt z = shape->f( impl->id_z );
                nFlt w = shape->f( impl->id_w );
                nFlt h = shape->f( impl->id_h );
                nFlt d = shape->f( impl->id_d );
                nStr color_str = shape->s( impl->id_color );
                int texid = Color::rgb( color_str );
                impl->viz_entities[i] = new Box( 0, this, true, x, y, z, w, h, d, texid, texid, texid );
                impl->viz_entities[i]->visible_set( is_visible );
            } else {
                printf( "ERROR: unknown shape kind '%s'\n", shape_kind );
                my_exit( 1 );
            }
        } else if ( strcmp( kind, "hide" ) == 0 ) {
            // hide existing shape
            //
            int index = obj->i( impl->id_index );
            dassert( impl->viz_entities[index] != NULL );
            impl->viz_entities[index]->visible_set( false );
        } else if ( strcmp( kind, "unhide" ) == 0 ) {
            // unhide existing shape
            //
            int index = obj->i( impl->id_index );
            dassert( impl->viz_entities[index] != NULL );
            impl->viz_entities[index]->visible_set( true );
        } else {
            printf( "ERROR: unknown kind '%s'\n", kind );
            my_exit( 1 );
        }
    }
}

//----------------------------------------------------------------
// Destructor
//----------------------------------------------------------------
Viz::~Viz()
{
    delete impl->viz_nodeio;
    impl->viz_nodeio = nullptr;
    delete impl;
    impl = nullptr;
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//
// GUI
//
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
void Viz::Impl::gui_init( void )
{
    //----------------------------------------------------------------
    // Nothing visible in the window currently.
    //----------------------------------------------------------------
    gui_recompute = true;
}

void Viz::frame_begin( float wall_clock_ms )
{
    //----------------------------------------------------------------
    // See if we need to recompute the window
    //----------------------------------------------------------------
    if ( impl->gui_recompute ) {
    }

    //----------------------------------------------------------------
    // Have SYS redraw the frame from scratch.
    //----------------------------------------------------------------
    impl->sys->force_redraw();
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
//
// NAVIGATION
//
//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------

//----------------------------------------------------------------
// Window resize event
//----------------------------------------------------------------
void Viz::resize_event( double w, double h )
{
    impl->gui_recompute = true;

    World::resize_event( w, h );
}

//----------------------------------------------------------------
// Mouse moved event
//----------------------------------------------------------------
void Viz::motion_event( double x, double y )
{
    //----------------------------------------------------------------
    // Update current mouse position
    //----------------------------------------------------------------
    impl->gui_mouse_x = int( x + 0.5 );
    impl->gui_mouse_y = int( y + 0.5 );

    //----------------------------------------------------------------
    // Let World do what it wants.
    //----------------------------------------------------------------
    World::motion_event( x, y );
}

//----------------------------------------------------------------
// Mouse button pressed/released event
//----------------------------------------------------------------
void Viz::button_event( int button, int action, int modifiers )
{
    //----------------------------------------------------------------
    // Let World handle it
    //----------------------------------------------------------------
    World::button_event( button, action, modifiers );
}

//----------------------------------------------------------------
// Key pressed/released event
//----------------------------------------------------------------
void Viz::key_event( int key, int action, int modifiers )
{
    if ( action != SYS_ACTION_PRESS ) {
        return;
    }

    switch( key )
    {
        case '>':
        case '}':
        case ']':
        case '$': 
            {
                int cnt = (key == '>') ? 1   :
                          (key == '}') ? 10  :
                          (key == ']') ? 100 : 100000000;
                for( int i = 0; i < cnt && impl->viz_last != (impl->viz_list->length()-1); i++ ) 
                {
                    impl->viz_last += 1;
                    Hash   * obj     = impl->viz_list->hp( impl->viz_last );
                    Entity * entity  = impl->viz_entities[impl->viz_last];
                    if ( entity != NULL ) {
                        entity->visible_set( true );
                    } else {
                        int index = obj->i( impl->id_index );
                        entity = impl->viz_entities[index];
                        bool is_hide = strcmp( obj->s( impl->id_kind ), "hide" ) == 0;
                        entity->visible_set( !is_hide );
                    }
                    printf( "%s\n", obj->s( impl->id_line ) );
                }
                break;
            }

        case '<':
        case '{':
        case '[':
        case '0':
            {
                int cnt = (key == '<') ? 1   :
                          (key == '{') ? 10  :
                          (key == '[') ? 100 : 100000000;
                for( int i = 0; i < cnt && impl->viz_last != 0; i++ )
                {
                    Hash   * obj     = impl->viz_list->hp( impl->viz_last );
                    Entity * entity  = impl->viz_entities[impl->viz_last];
                    if ( entity != NULL ) {
                        entity->visible_set( false );
                    } else {
                        int index = obj->i( impl->id_index );
                        entity = impl->viz_entities[index];
                        bool is_hide = strcmp( obj->s( impl->id_kind ), "hide" ) == 0;
                        entity->visible_set( is_hide );
                    }
                    impl->viz_last -= 1;
                    printf( "%s\n", obj->s( impl->id_line ) );
                }
                break;
            }

        case '.':
            printf( "%s\n", impl->viz_list->hp( impl->viz_last )->s( impl->id_line ) );
            break;

        default:
        {
            //----------------------------------------------------------------
            // Let World handle it
            //----------------------------------------------------------------
            World::key_event( key, action, modifiers );
        }
    }
}
