// Copyright (c) 2014-2019 Robert A. Alfieri
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

// All the heavy lifting happens here.  We eventually want to push all of it down to the GPU using
// compute and graphics shaders.
//
#include "Sys.h"
#include "Misc.h"

class Batch
{
public:
    int                 hdl;            // hdl for Sys

    Geom *              geom;           // array of Geom
    int                 geom_alloc;     // Geom structs allocated
    int                 geom_used;      // Geom structs used
    bool                geom_changed;   // any geom changed?

    int                 vertex_alloc;   // number of vertexes allocated in batch
    int                 vertex_used;    // number of vertexes used in batch
    int                 triangle_alloc; // number of triangles allocated in batch
    int                 triangle_used;  // number of triangles used in batch
};

class World::Impl
{
public:
    Config * config;
    Sys * sys;

    bool  use_ortho;
    float fov_y;
    float near_z;
    float far_z;
    float eye[3];
    float view[3];
    float up[3];
    float D[3];

    float motion_x;
    float motion_y;
    bool * button_pressed;

    Batch ** batch; 
    int      batch_alloc;
    int      batch_used;

    int      text2d_cnt;
    int      text2d_alloc;
    Text2D * text2d_text;

    void     view_print( const char * from );
    void     view_translate( float x, float y, float z );
    void     view_turn_left_right( float a );
    void     view_tilt_up_down( float a );
};

World::World( Config * config, Sys * sys, float w, float h, float d )
{
    impl = new Impl();
    impl->config = config;
    impl->sys = sys;

    //------------------------------------------------------------
    // Initialize variables for mouse and keyboard
    //------------------------------------------------------------
    impl->motion_x = 0;
    impl->motion_y = 0;
    impl->button_pressed = new bool[SYS_BUTTON_COUNT];
    for( int i = 0; i < SYS_BUTTON_COUNT; i++ ) impl->button_pressed[i] = false;

    //------------------------------------------------------------
    // Initialize camera perspective
    //------------------------------------------------------------
    impl->use_ortho = config->win_ortho;
    impl->fov_y = config->win_fov_y;
    impl->near_z = config->win_near_z;
    impl->far_z = config->win_far_z;

    for( int i = 0; i < 3; i++ ) 
    {
        impl->eye[i]  = config->win_eye[i];  // where is the eye?
        impl->view[i] = config->win_view[i]; // what point is the eye looking at?
        impl->up[i]   = config->win_up[i];   // which way is up?
    }
    vec_sub( impl->D, impl->view, impl->eye );
    vec_normalize( impl->D, impl->D );
    vec_add( impl->view, impl->eye, impl->D );
    impl->view_print( "World" );

    //------------------------------------------------------------
    // No batches yet.
    //------------------------------------------------------------
    impl->batch = nullptr;
    impl->batch_alloc = 0;
    impl->batch_used = 0;

    //------------------------------------------------------------
    // No 2D text yet.
    //------------------------------------------------------------
    impl->text2d_cnt = 0;
    impl->text2d_alloc = 256;
    impl->text2d_text = new Text2D[impl->text2d_alloc];
}

World::~World()
{
    //------------------------------------------------------------
    // Delete all batches
    //------------------------------------------------------------
    impl = nullptr;
}

Config * World::config_get( void )
{
    return impl->config;
}

//------------------------------
// VIEW
//------------------------------
void World::view_get( float * fov_y, float * near_z, float * far_z,
                      float eye[], float view[], float up[] )
{
    *fov_y  = impl->fov_y;
    *near_z = impl->near_z;
    *far_z  = impl->far_z;
    for( int i = 0; i < 3; i++ ) 
    {
        eye[i]  = impl->eye[i];
        view[i] = impl->view[i];
        up[i]   = impl->up[i];
    }
    impl->view_print( "view_get" );
}

void World::view_set( float fov_y, float near_z, float far_z,
                      float eye[], float view[], float up[] )
{
    impl->fov_y  = fov_y;
    impl->near_z = near_z;
    impl->far_z  = far_z;
    for( int i = 0; i < 3; i++ ) 
    {
        impl->eye[i]  = eye[i];
        impl->view[i] = view[i];
        impl->up[i]   = up[i];
    }
    vec_sub( impl->D, impl->view, impl->eye );
    vec_normalize( impl->D, impl->D );
    vec_add( impl->view, impl->eye, impl->D );
    impl->view_print( "view_set" );

    impl->sys->force_redraw();                                        
}

//------------------------------
// GEOMETRY
//------------------------------
int World::geom_add( Vertex * vertex, int vertex_cnt, Triangle * triangle, int triangle_cnt, int changes )
{
    //------------------------------------------------------
    // might need a new Batch
    //
    // TODO: need to pay attention to changes
    //------------------------------------------------------
    if ( impl->batch == 0 ) {
        impl->batch = new Batch*[impl->config->win_batch_cnt];
        impl->batch_alloc = impl->config->win_batch_cnt;
        impl->batch_used = 0;
    }

    int bi = impl->batch_used - 1;
    if ( bi < 0 || 
         (impl->batch[bi]->geom_used + 1) > impl->batch[bi]->geom_alloc ||
         (impl->batch[bi]->vertex_used + vertex_cnt) > impl->batch[bi]->vertex_alloc ||
         (impl->batch[bi]->triangle_used + triangle_cnt) > impl->batch[bi]->triangle_alloc ) {

        impl->batch_used++;
        dassert( impl->batch_used <= impl->batch_alloc );

        bi++;
        Batch * batch = new Batch;
        impl->batch[bi] = batch;

        batch->geom = new Geom[impl->config->win_batch_geom_cnt];
        batch->geom_alloc = impl->config->win_batch_geom_cnt;
        batch->geom_used = 0;
        batch->geom_changed = false;

        batch->vertex_alloc   = impl->config->win_batch_vertex_cnt;
        batch->vertex_used    = 0;
        batch->triangle_alloc = impl->config->win_batch_triangle_cnt;
        batch->triangle_used  = 0;

        batch->hdl = impl->sys->batch_alloc( batch->vertex_alloc, batch->triangle_alloc );
    }

    Batch * batch = impl->batch[bi];

    //------------------------------------------------------
    // add new Geom to Batch
    //------------------------------------------------------
    dassert( batch->geom_used < batch->geom_alloc );
    Geom * geom = &batch->geom[batch->geom_used];
    batch->geom_changed = true;
    geom->valid = true;
    geom->visible = true;
    geom->changes = changes;
    geom->changed = true;
    geom->vertex = vertex;
    geom->vertex_cnt = vertex_cnt;
    geom->triangle = triangle;
    geom->triangle_cnt = triangle_cnt;
    batch->vertex_used += vertex_cnt;
    batch->triangle_used += triangle_cnt;

    return (bi << 16) | batch->geom_used++;
}

bool World::geom_visible_get( int hdl )
{
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];
    int geom_index  = hdl & 0xffff;
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    return batch->geom[geom_index].visible;
}

void World::geom_visible_set( int hdl, bool visible )
{
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];
    int geom_index  = hdl & 0xffff;
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    if ( batch->geom[geom_index].visible != visible ) {
        batch->geom[geom_index].visible = visible;
        this->geom_changed( hdl );
    }
}

int World::geom_changes_get( int hdl )
{
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];
    int geom_index  = hdl & 0xffff;
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    return batch->geom[geom_index].changes;
}

void World::geom_changes_set( int hdl, int changes )
{
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];
    int geom_index  = hdl & 0xffff;
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    if ( batch->geom[geom_index].changes != changes ) {
        batch->geom[geom_index].changes = changes;
        this->geom_changed( hdl );
    }
}

void World::geom_changed( int hdl )
{
    //------------------------------------------------------
    // mark geometry as changed
    //------------------------------------------------------
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];

    int geom_index  = hdl & 0xffff;
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    batch->geom[geom_index].changed = 1;
    batch->geom_changed = 1;
    impl->sys->force_redraw();
}

void World::geom_remove( int hdl )
{
    //------------------------------------------------------
    // mark geometry as no longer valid
    // mark it changed, too, which is the signal for deleting
    //------------------------------------------------------
    int batch_index = hdl >> 16;
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];
    dassert( batch != nullptr );

    int geom_index  = hdl & 0xffff;
    dprintf( "remove: bi=%d gi=%d\n", batch_index, geom_index );
    dassert( geom_index < batch->geom_used );
    dassert( batch->geom[geom_index].valid );
    batch->geom[geom_index].valid = 0;
    batch->geom[geom_index].changed = 1;
    batch->geom_changed = 1;
}


//------------------------------
// 2D TEXT
//------------------------------
void World::text2d_set( int x, int y, int h, int rgb, int str_cnt, const char * str[] )
{
    dassert( str_cnt <= impl->text2d_alloc );
    impl->text2d_cnt = str_cnt;
    for( int i = 0; i < str_cnt; i++ ) 
    {
        impl->text2d_text[i].x   = x;
        impl->text2d_text[i].y   = y;
        impl->text2d_text[i].rgb = rgb;
        impl->text2d_text[i].str = str[i];
        y -= h;
    }
    impl->sys->force_redraw();
}

//------------------------------
// VECTORS
//------------------------------
float World::vec_length( const float A[], int dims )
{
    float len = 0.0;
    for( int i = 0; i < dims; i++ )  
    {
        len += A[i]*A[i];
    }
    len = sqrt( len );
    return len;
}

void World::vec_assign( float R[], const float A[], int dims )
{
    for( int i = 0; i < dims; i++ )  
    {
        R[i] = A[i];
    }
}

void World::vec_normalize( float R[], const float A[], int dims )
{
    float len = vec_length( A );
    dassert( len != 0.0 );

    for( int i = 0; i < dims; i++ ) 
    {
        R[i] = A[i] / len;
    }
}

void World::vec_add( float R[], const float A[], const float B[], int dims )
{
    for( int i = 0; i < dims; i++ )  
    {
        R[i] = A[i] + B[i];
    }
}

void World::vec_sub( float R[], const float A[], const float B[], int dims )
{
    for( int i = 0; i < dims; i++ )  
    {
        R[i] = A[i] - B[i];
    }
}

void World::vec_scale( float R[], const float A[], const float B[], int dims )
{
    for( int i = 0; i < dims; i++ ) 
    {
        R[i] = A[i] * B[i];
    }
}

void World::vec_scale( float R[], const float A[], const float c, int dims )
{
    for( int i = 0; i < dims; i++ ) 
    {
        R[i] = A[i] * c;
    }
}

void World::vec_cross( float R[], const float A[], const float B[], int dims )
{
    dassert( dims == 3 );
    R[0] = A[1]*B[2]-A[2]*B[1];
    R[1] = A[2]*B[0]-A[0]*B[2];
    R[2] = A[0]*B[1]-A[1]*B[0];
}

//------------------------------
// PRIVATE FUNCTIONS
//------------------------------
void World::frame_render( float wall_clock_ms )
{
    //------------------------------------------------------
    // do beginning work that may change the frame
    //------------------------------------------------------
    this->frame_begin( wall_clock_ms );

    //------------------------------------------------------
    // begin frame draw
    //------------------------------------------------------
    impl->sys->draw_begin( impl->use_ortho,
                           impl->fov_y, impl->near_z, impl->far_z,
                           impl->eye,   impl->view,   impl->up );

    //------------------------------------------------------
    // draw all batches
    //------------------------------------------------------
    for( int b = 0; b < impl->batch_used; b++ )
    {
        //------------------------------------------------------
        // now do the actual draw
        //------------------------------------------------------
        Batch * batch = impl->batch[b];
        impl->sys->draw_batch( batch->hdl, batch->geom, batch->geom_used, batch->geom_changed );
        batch->geom_changed = 0;
    }

    //------------------------------------------------------
    // draw any 2d overlay text
    //------------------------------------------------------
    impl->sys->draw_text2d( impl->text2d_text, impl->text2d_cnt );

    //------------------------------------------------------
    // end frame draw
    //------------------------------------------------------
    impl->sys->draw_end();
}

void World::frame_begin( float wall_clock_ms )
{
    dassert( 0 && "oops, running default frame_begin" );
}

//--------------------------------------------------------------
// INPUT
//--------------------------------------------------------------
void World::resize_event( double w, double h )
{
}

void World::motion_event( double x, double y )
{
    float change_x = x - impl->motion_x;
    float change_y = y - impl->motion_y;

    impl->motion_x = x;
    impl->motion_y = y;

    if ( change_x < -0.001 || change_x > 0.001 ) {
        //--------------------------------------------------------------
        // Turn left or right.
        //--------------------------------------------------------------
        float a = -change_x / 100.0;
        impl->view_turn_left_right( a );
    }

    if ( change_y < -0.001 || change_y > 0.001 ) {
        //--------------------------------------------------------------
        // Tilt up or down.
        //--------------------------------------------------------------
        float a = change_y / 100.0;
        impl->view_tilt_up_down( a );
    }
}

void World::key_event( int key, int action, int modifiers )
{
    if ( action != SYS_ACTION_PRESS ) {
        return;
    }

    const float MOVEMENT = 0.04;
    switch( key )
    {
        case 'Q':
            impl->sys->quit( 0 );
            break;

        // Minecraft style movement
        //
        case 'a':
            impl->view_translate( 10.0*MOVEMENT/2.0f, 0.0, 0.0 );
            break;

        case 'A':
            impl->view_translate( 100.0*MOVEMENT/2.0f, 0.0, 0.0 );
            break;

        case 'd':
            impl->view_translate( -10.0*MOVEMENT/2.0f, 0.0, 0.0 );
            break;

        case 'D':
            impl->view_translate( -100.0*MOVEMENT/2.0f, 0.0, 0.0 );
            break;

        case 'w':
            impl->view_translate( 0.0, 0.0, -10.0*MOVEMENT );
            break;

        case 'W':
            impl->view_translate( 0.0, 0.0, -100.0*MOVEMENT );
            break;

        case 's':
            impl->view_translate( 0.0, 0.0, 10.0*MOVEMENT );
            break;

        case 'S':
            impl->view_translate( 0.0, 0.0, 100.0*MOVEMENT );
            break;

        case ' ':
            if ( (modifiers & (SYS_MOD_SHIFT|SYS_MOD_CTRL)) == 0 ) {
                impl->view_translate( 0.0, 10.0*MOVEMENT, 0.0 );
            } else {
                impl->view_translate( 0.0, 100.0*MOVEMENT, 0.0 );
            }
            break;

        case '\t':
            impl->view_translate( 0.0, -10.0*MOVEMENT, 0.0 );
            break;

        case '`':
            impl->view_translate( 0.0, -100.0*MOVEMENT, 0.0 );
            break;

        case 'h':
            motion_event( impl->motion_x-1.0, impl->motion_y );
            break;

        case 'H':
            motion_event( impl->motion_x-10.0, impl->motion_y );
            break;

        case 'l':
            motion_event( impl->motion_x+1.0, impl->motion_y );
            break;

        case 'L':
            motion_event( impl->motion_x+10.0, impl->motion_y );
            break;

        case 'k':
            motion_event( impl->motion_x, impl->motion_y-1.0 );
            break;

        case 'K':
            motion_event( impl->motion_x, impl->motion_y-60.0 );
            break;

        case 'j':
            motion_event( impl->motion_x, impl->motion_y+1.0 );
            break;

        case 'J':
            motion_event( impl->motion_x, impl->motion_y+60.0 );
            break;

        case '+':
            impl->fov_y /= 1.10;  // zoom in
            impl->view_print( "zoom in" );
            impl->sys->force_redraw();
            break;

        case '-':
            impl->fov_y *= 1.10;  // zoom out
            impl->view_print( "zoom out" );
            impl->sys->force_redraw();
            break;

        case '_':
            impl->sys->toggle_fullscreen();
            impl->sys->force_redraw();
            break;

        default:
            // ignore
            break;
    }
}

void World::button_event( int button, int action, int modifiers )
{
    bool is_pressed = action == SYS_ACTION_PRESS;
    impl->button_pressed[button] = is_pressed;

    // select object when pressed
}

void World::scroll_event( int scroll, int action, int modifiers )
{
}

void World::Impl::view_print( const char * from )
{
    if ( !config->win_view_print ) return;

    float len = vec_length( D );
    float diff[3]; 
    vec_sub( diff, view, eye );
    printf( "%s(): D=[%f, %f, %f] D_len=%f eye=[%f, %f, %f] view=[%f, %f, %f] diff=[%f, %f, %f] fov_y=%f\n", from, D[0], D[1], D[2], len, eye[0], eye[1], eye[2], view[0], view[1], view[2], diff[0], diff[1], diff[2], fov_y );
}

void World::Impl::view_translate( float x, float y, float z )
{
    //------------------------------------------------------------
    // This is messed up right now, so we're going to take
    // care of the different cases.
    //------------------------------------------------------------
    if ( y != 0.0 ) {
        // easy case
        //
        eye[1]  += y;
        view[1] += y;
    } else if ( z != 0.0 ) {
        // move along D
        //
        eye[0]  += -z * D[0];
        eye[2]  += -z * D[2];
        view[0] += -z * D[0];
        view[2] += -z * D[2];
    } else if ( x != 0.0 ) {
        // move perpendicular
        //
        eye[0]  += x  * D[2];
        eye[2]  += -x * D[0];
        view[0] += x  * D[2];
        view[2] += -x * D[0];
    }
    view_print( "view_translate" );

    sys->force_redraw();
}


void World::Impl::view_turn_left_right( float a )
{
    //------------------------------------------------------------
    // Rotate around Y axis.
    //------------------------------------------------------------
    float D_new[3];

    float sin_a = sin( a );
    float cos_a = cos( a );
    dprintf( "a=%f sin_a=%f cos_a=%f\n", a, sin_a, cos_a );
    D_new[0] = D[2] * sin_a + D[0] * cos_a;
    D_new[1] = 0.0f;
    D_new[2] = D[2] * cos_a - D[0] * sin_a;
    vec_normalize( D, D_new );
    view[0] = eye[0] + D[0];
    view[2] = eye[2] + D[2];
    view_print( "view_turn_left_right" );

    sys->force_redraw();
}

void World::Impl::view_tilt_up_down( float a )
{
    //------------------------------------------------------------
    // Tilt up/down by simply changing the view Y value.
    //------------------------------------------------------------
    view[1] += a;
    view_print( "view_tilt_up_down" );

    sys->force_redraw();
}
