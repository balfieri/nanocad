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

// OpenGL 2.1 and GLUT Based Implementation
//
#include <string.h>
#include <sys/time.h>
#ifndef offsetof
#define offsetof( st, f ) __builtin_offsetof( st, f )
#endif

#include "Sys.h"

#ifdef GLUT_ONLY
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#define ATTRIB_POSITION 0
#define ATTRIB_NORMAL   1
#define ATTRIB_TEXID    2
#define ATTRIB_TEXCOORD 3

#ifdef EMULATE_BUFFERS
//
// EMULATE Buffer-Related Functions (makes it a little easier to debug in immediate mode)
//
void GenBuffers( GLsizei n, GLuint * buffers );
void BindBuffer( GLenum target, GLuint buffer );
void BufferData( GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage );
void EnableVertexAttribArray( GLuint index );
void DisableVertexAttribArray( GLuint index );
void VertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer );
void DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid * indices );
#else
#define GenBuffers			glGenBuffers
#define BindBuffer			glBindBuffer
#define BufferData			glBufferData
#define EnableVertexAttribArray		glEnableVertexAttribArray
#define DisableVertexAttribArray	glDisableVertexAttribArray
#define VertexAttribPointer		glVertexAttribPointer
#define DrawElements			glDrawElements
#endif

Sys * sys = nullptr;

class Batch 
{
public:
    unsigned            vbo_hdl;        // vertex buffer object handle
    Vertex *            vbo;            // vertices
    unsigned int        vbo_alloc;      // vertices allocated 
    unsigned int        vbo_used;       // vertices used

    unsigned            ibo_hdl;        // index buffer object handle
    Triangle *          ibo;            // indexes
    unsigned int        ibo_alloc;      // indexes allocated 
    unsigned int        ibo_used;       // indexes used
};

class Sys::Impl 
{
public:
    Config *            config;         // configuration info
    World *             world;          // current world

    int                 win_w;
    int                 win_h;

    Batch **            batch; 
    int                 batch_alloc;
    int                 batch_used;

    bool                capture_enabled;// whether to capture frames to file
    FILE *              capture_stream; // pipe to video compression program
    unsigned int *      capture_buff;   // capture buffer for glReadBuffer() results

    static void render_event( void )
    {
        if ( sys->impl->world == 0 ) return;

        struct timeval tv;
        gettimeofday( &tv, 0 );
        double wall_clock_ms = double(tv.tv_sec)*1000.0 + double(tv.tv_usec)/1000.0;
        static double wall_clock_ms_first = 0.0;
        if ( wall_clock_ms_first <= 0.01 ) {
            wall_clock_ms_first = wall_clock_ms;
        }

        double diff = wall_clock_ms - wall_clock_ms_first;
        sys->impl->world->frame_render( float(diff) );
    }

    static void resize_event( int w, int h )
    {
        sys->impl->win_w = w;
        sys->impl->win_h = h;

        if ( sys->impl->world == 0 ) return;

        sys->impl->world->resize_event( double(w), double(h) );
    }

    static int modifiers( void )
    {
        int glut_mods = glutGetModifiers();
        int mods = ((glut_mods & GLUT_ACTIVE_SHIFT) ? SYS_MOD_SHIFT : 0) |
                   ((glut_mods & GLUT_ACTIVE_CTRL)  ? SYS_MOD_CTRL  : 0) |
                   ((glut_mods & GLUT_ACTIVE_ALT)   ? SYS_MOD_ALT   : 0);
        return mods;
    }

    static void motion_event( int x, int y )
    {
        if ( sys->impl->world == 0 || !sys->impl->config->win_mouse_motion_enabled ) return;

        sys->impl->world->motion_event( double(x), double(y) );
    }

    static void key_event( unsigned char key, int x, int y )
    {
        if ( sys->impl->world == 0 ) return;

        int mods = modifiers();
        //motion_event( x, y );
        sys->impl->world->key_event( key, SYS_ACTION_PRESS,   mods );
        sys->impl->world->key_event( key, SYS_ACTION_RELEASE, mods );
    }

    static void special_key_event( int key, int x, int y )
    {
        if ( sys->impl->world == 0 ) return;

        // translate from glut defines to our defins
        //
        switch( key ) 
        {
            case GLUT_KEY_F1: 		key = SYS_KEY_F1;		break;
            case GLUT_KEY_F2: 		key = SYS_KEY_F2;		break;
            case GLUT_KEY_F3: 		key = SYS_KEY_F3;		break;
            case GLUT_KEY_F4: 		key = SYS_KEY_F4;		break;
            case GLUT_KEY_F5: 		key = SYS_KEY_F5;		break;
            case GLUT_KEY_F6: 		key = SYS_KEY_F6;		break;
            case GLUT_KEY_F7: 		key = SYS_KEY_F7;		break;
            case GLUT_KEY_F8: 		key = SYS_KEY_F8;		break;
            case GLUT_KEY_F9: 		key = SYS_KEY_F9;		break;
            case GLUT_KEY_F10: 		key = SYS_KEY_F10;		break;
            case GLUT_KEY_F11: 		key = SYS_KEY_F11;		break;
            case GLUT_KEY_F12: 		key = SYS_KEY_F12;		break;
            case GLUT_KEY_LEFT: 	key = SYS_KEY_LEFT;		break;
            case GLUT_KEY_UP: 		key = SYS_KEY_UP;		break;
            case GLUT_KEY_RIGHT: 	key = SYS_KEY_RIGHT;		break;
            case GLUT_KEY_DOWN: 	key = SYS_KEY_DOWN;		break;
            case GLUT_KEY_PAGE_UP: 	key = SYS_KEY_PAGE_UP;		break;
            case GLUT_KEY_PAGE_DOWN: 	key = SYS_KEY_PAGE_DOWN;	break;
            case GLUT_KEY_HOME: 	key = SYS_KEY_HOME;		break;
            case GLUT_KEY_END: 		key = SYS_KEY_END;		break;
            case GLUT_KEY_INSERT: 	key = SYS_KEY_INSERT;		break;
            default:                    key = SYS_KEY_UNKNOWN;		break;
        }
        int mods = modifiers();
        motion_event( x, y );
        sys->impl->world->key_event( key, SYS_ACTION_PRESS,   mods );
        sys->impl->world->key_event( key, SYS_ACTION_RELEASE, mods );
    }

    static void button_event( int glut_button, int state, int x, int y )
    {
        if ( sys->impl->world == 0 ) return;

        motion_event( x, y );
        int mods   = modifiers();
        if ( glut_button < 3 ) {
            // MOUSE BUTTON
            //
            int button = (glut_button == GLUT_LEFT_BUTTON)   ? SYS_BUTTON_LEFT   :
                         (glut_button == GLUT_MIDDLE_BUTTON) ? SYS_BUTTON_MIDDLE : SYS_BUTTON_RIGHT;
            int action = (state == GLUT_DOWN) ? SYS_ACTION_PRESS : SYS_ACTION_RELEASE;
            sys->impl->world->button_event( button, action, mods );
        } else if (glut_button < 7 ) { 
            // SCROLL
            //
            int scroll = (glut_button == 3 || glut_button == 4) ? SYS_SCROLL_VERTICAL : SYS_SCROLL_HORIZONTAL;
            int action = (glut_button == 3) ? SYS_ACTION_SCROLL_UP    :
                         (glut_button == 4) ? SYS_ACTION_SCROLL_DOWN  :
                         (glut_button == 5) ? SYS_ACTION_SCROLL_LEFT  : SYS_ACTION_SCROLL_RIGHT;
            sys->impl->world->scroll_event( scroll, action, mods );
        } else {
            dassert( 0 && "unknown glut mouse button" );
        }
    }
};


Sys::Sys( Config * config )
{
    //--------------------------------------------------------
    // There's really only one Sys instance stored in sys.
    //--------------------------------------------------------
    impl = new Sys::Impl();
    sys = this;
    sys->impl = impl;
    impl->config = config;
    impl->world = nullptr;

    //--------------------------------------------------------
    // Initialize GLUT and create the window.
    //--------------------------------------------------------
    int argc = 0;
    char ** argv = nullptr;
    glutInit( &argc, argv ); 

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    impl->win_w = config->win_w;
    impl->win_h = config->win_h;
    glutInitWindowSize( impl->win_w, impl->win_h );
    glutInitWindowPosition( config->win_off_x, config->win_off_y );
    glutCreateWindow( config->win_name );
    if ( config->win_fullscreen ) glutFullScreen();

    //----------------------------------------------------------
    // Set up callbacks
    //----------------------------------------------------------
    glutReshapeFunc( Sys::Impl::resize_event );
    glutDisplayFunc( Sys::Impl::render_event );

    glutMotionFunc( Sys::Impl::motion_event );
    glutPassiveMotionFunc( Sys::Impl::motion_event );
    glutKeyboardFunc( Sys::Impl::key_event );
    glutSpecialFunc( Sys::Impl::special_key_event );
    glutMouseFunc( Sys::Impl::button_event );

    //------------------------------------------------------------
    // No batches yet.
    //------------------------------------------------------------
    impl->batch = nullptr;
    impl->batch_alloc = 0;
    impl->batch_used = 0;

    //------------------------------------------------------------
    // Issue one-time OGL commands.
    //------------------------------------------------------------
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_MULTISAMPLE );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_POLYGON_SMOOTH );
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glHint( GL_LINE_SMOOTH_HINT,    GL_NICEST );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    int texid = config->texid_background;
    float r = float( (texid >> 16) & 0xff ) / 255.0f;
    float g = float( (texid >>  8) & 0xff ) / 255.0f;
    float b = float( (texid >>  0) & 0xff ) / 255.0f;
    glClearColor( r, g, b, 0.0f );

    impl->capture_enabled = config->win_capture_enabled;
    if ( config->win_capture_enabled ) {
        //------------------------------------------------------------
        // Set up frame capture to ffmpeg.
        //------------------------------------------------------------
        char cmd[256];
        sprintf( cmd, "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s %dx%d -i - "
                      "-threads 0 -preset fast -y -crf 21 -vf vflip capture.mp4", impl->win_w, impl->win_h );
        impl->capture_stream = popen( cmd, "wb" );  // pipe in binary write mode
        dassert( impl->capture_stream != nullptr && "could not open pipe to ffmpeg" );
        impl->capture_buff = new unsigned int[ impl->win_w * impl->win_h ];
    }
}

void Sys::world_set( World * world )
{
    impl->world = world;
}

World * Sys::world_get( void )
{
    return impl->world;
}

int Sys::batch_alloc( int vertex_cnt, int triangle_cnt )
{
    //----------------------------------------------------------
    // allocate batch structure
    //----------------------------------------------------------
    if ( impl->batch == 0 ) {
        impl->batch = new Batch*[1024];
        impl->batch_alloc = 1024;
        impl->batch_used = 0;
    }
    dassert( impl->batch_used < impl->batch_alloc ); 

    int batch_index = impl->batch_used++;
    Batch * batch = new Batch;
    impl->batch[batch_index] = batch;

    //----------------------------------------------------------
    // allocate one vertex buffer object and one index buffer object
    //----------------------------------------------------------
    GenBuffers( 1, &batch->vbo_hdl );
    batch->vbo = new Vertex[vertex_cnt];
    batch->vbo_alloc = vertex_cnt;
    batch->vbo_used = 0;

    GenBuffers( 1, &batch->ibo_hdl );
    batch->ibo = new Triangle[triangle_cnt];
    batch->ibo_alloc = triangle_cnt;
    batch->ibo_used = 0;

    return batch_index;
}

void batch_free( int batch_hdl )
{
    //----------------------------------------------------------
    // deallocate GPU buffer and Batch structure
    //----------------------------------------------------------
}

void Sys::main_loop( void )
{
    //----------------------------------------------------------
    // transfer control to GLUT main loop
    //----------------------------------------------------------
    glutMainLoop();
    this->quit( 0 );
}

void Sys::quit( int status )
{
    if ( impl->capture_enabled ) {
        pclose( impl->capture_stream );
    }
    my_exit( status );
}

void Sys::force_redraw( void )
{
    dprintf( "force redraw\n" );
    glutPostRedisplay();
}

void Sys::draw_begin( bool use_ortho,
                      float fov_y,  float near_z, float far_z,
                      float eye[],  float view[], float up[] )
{
    //----------------------------------------------------------
    // set up viewport and perspective
    //----------------------------------------------------------
    glViewport( 0, 0, impl->win_w, impl->win_h );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if ( use_ortho ) {
        glOrtho( 0.0f, impl->win_w, 0.0f, impl->win_h, near_z, far_z );
    } else {
        gluPerspective( fov_y, impl->config->win_perspective_fudge_factor*GLfloat(impl->win_w)/GLfloat(impl->win_h), near_z, far_z );
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    if ( !use_ortho ) {
        gluLookAt( eye[0],  eye[1],  eye[2],
                   view[0], view[1], view[2],
                   up[0],   up[1],   up[2] );
    }

    EnableVertexAttribArray( ATTRIB_POSITION );
    EnableVertexAttribArray( ATTRIB_NORMAL );
    EnableVertexAttribArray( ATTRIB_TEXID );
    EnableVertexAttribArray( ATTRIB_TEXCOORD );

    glEnable( GL_LIGHTING );
    glEnable( GL_COLOR_MATERIAL );

    GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 0.5 };
    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );

    //GLfloat mat_emission[] = { 0.5, 0.5, 0.5, 0.5 };
    //glMaterialfv( GL_FRONT, GL_EMISSION, mat_emission );

    GLfloat mat_shininess[] = { 3.0 };
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );

    glEnable( GL_LIGHT0 );
    GLfloat lightAmbient0[] = { 1.0f,   1.0f,   1.0f,    1.0f };
    GLfloat lightColor0[]   = { 1.0f,   1.0f,   1.0f,    1.0f };
    GLfloat lightPos0[]     = { 0.0f, 100.0f, 100.0f, 1000.0f }; 
    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient0 );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightColor0 );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos0 );

    glDisable( GL_LIGHT1 );
}

void Sys::draw_batch( int batch_index, Geom * geom, int geom_cnt, bool geom_changed )
{
    dassert( batch_index < impl->batch_used );
    Batch * batch = impl->batch[batch_index];

    //----------------------------------------------------------
    // bind the vertex and index buffers
    // communicate the vertex attributes
    //----------------------------------------------------------
    BindBuffer( GL_ARRAY_BUFFER, batch->vbo_hdl );
    VertexAttribPointer( ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), reinterpret_cast<GLvoid *>( offsetof( Vertex, position ) ) );
    VertexAttribPointer( ATTRIB_NORMAL,   3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), reinterpret_cast<GLvoid *>( offsetof( Vertex, normal ) ) );
    VertexAttribPointer( ATTRIB_TEXID,    1, GL_INT,   GL_FALSE, sizeof( Vertex ), reinterpret_cast<GLvoid *>( offsetof( Vertex, texid ) ) );
    VertexAttribPointer( ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), reinterpret_cast<GLvoid *>( offsetof( Vertex, texcoord ) ) );

    BindBuffer( GL_ELEMENT_ARRAY_BUFFER, batch->ibo_hdl );

    //----------------------------------------------------------
    // update buffers if geometry has changed
    //----------------------------------------------------------
    if ( geom_changed ) {
        //----------------------------------------------------------
        // for now, redo the whole batch, even parts that aren't changing
        //----------------------------------------------------------
	batch->vbo_used = 0;
	batch->ibo_used = 0;
        for( int i = 0; i < geom_cnt; i++ )
        {
            //----------------------------------------------------------
            // ignore invalid or invisible geometry
            //----------------------------------------------------------
            if ( !geom[i].valid || !geom[i].visible ) continue;

            //----------------------------------------------------------
            // note the vertex offset, then copy all the vertexes into the vbo.
            //----------------------------------------------------------
            unsigned short offset = batch->vbo_used;
            batch->vbo_used += geom[i].vertex_cnt;
            dassert( batch->vbo_used <= batch->vbo_alloc );
            memcpy( &batch->vbo[offset], geom[i].vertex, geom[i].vertex_cnt * sizeof( Vertex ) );

            //----------------------------------------------------------
            // copy the indexes, offset by the vertex offset, into the ibo.
            //----------------------------------------------------------
            dassert( (batch->ibo_used + geom[i].triangle_cnt) <= batch->ibo_alloc );
            unsigned int j;
            Triangle * ibo_ptr;
            Triangle * tri_ptr;
            for( j = 0, ibo_ptr = &batch->ibo[batch->ibo_used], tri_ptr = geom[i].triangle;
                 j < geom[i].triangle_cnt; 
                 j++, ibo_ptr++, tri_ptr++ ) 
            {
                ibo_ptr->v0 = tri_ptr->v0 + offset;
                ibo_ptr->v1 = tri_ptr->v1 + offset;
                ibo_ptr->v2 = tri_ptr->v2 + offset;
            }
            batch->ibo_used += geom[i].triangle_cnt;
        }

        //----------------------------------------------------------
        // copy the entire vbo and ibo onto the gpu
        //----------------------------------------------------------
        BufferData( GL_ARRAY_BUFFER, batch->vbo_used * sizeof( Vertex ), batch->vbo, GL_STATIC_DRAW );
        BufferData( GL_ELEMENT_ARRAY_BUFFER, batch->ibo_used * sizeof( Triangle ), batch->ibo, GL_STATIC_DRAW );
    }

    //----------------------------------------------------------
    // draw all the triangles in the ibo
    //----------------------------------------------------------
    DrawElements( GL_TRIANGLES, 3 * batch->ibo_used, GL_UNSIGNED_SHORT, batch->ibo );
}

void Sys::draw_text2d( Text2D * text, int text_cnt )
{
    //----------------------------------------------------------
    // draw 2D text overlays
    //----------------------------------------------------------
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, impl->win_w, 0.0, impl->win_h );
//  void * font = GLUT_BITMAP_8_BY_13; // impl->config->win_overlay_text_font;
    void * font = GLUT_BITMAP_9_BY_15; // impl->config->win_overlay_text_font;

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity(); 
    float x_scale_factor = impl->config->win_overlay_text_scale_factor;
    float y_scale_factor = x_scale_factor * 1.2f;
    float z_scale_factor = x_scale_factor;
    glScalef( x_scale_factor, y_scale_factor, z_scale_factor );
    if ( font == nullptr ) {
        glLineWidth( impl->config->win_overlay_text_line_width );
    } else {
        glDisable( GL_LIGHTING );
    }

    for( int i = 0; i < text_cnt; i++ )
    {
        int rgb = text[i].rgb;
        float r = float( (rgb >> 16) & 0xff ) / 255.0f;
        float g = float( (rgb >>  8) & 0xff ) / 255.0f;
        float b = float( (rgb >>  0) & 0xff ) / 255.0f;
        glColor3f( r, g, b );
        
        float x = float(text[i].x) / x_scale_factor;
        float y = float(text[i].y) / y_scale_factor;
        float w = float(impl->config->win_overlay_text_width) / x_scale_factor;
        int len = strlen( text[i].str );
        if ( font != nullptr ) {
            glRasterPos2i( int( x ), int( y ) );
        }
        for( int j = 0; j < len; j++ )
        {
            if ( font != nullptr ) {
                glutBitmapCharacter( font, text[i].str[j] ); 
            } else {
                glPushMatrix();
                glTranslatef( x, y, -1.0f );
                glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, text[i].str[j] );
                glPopMatrix();
            }
            x += w;
        }
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Sys::draw_end( void )
{
    //----------------------------------------------------------
    // end drawing this frame => swap buffers
    //----------------------------------------------------------
    DisableVertexAttribArray( ATTRIB_POSITION );
    DisableVertexAttribArray( ATTRIB_NORMAL );
    DisableVertexAttribArray( ATTRIB_TEXID );
    DisableVertexAttribArray( ATTRIB_TEXCOORD );

    glutSwapBuffers();

    if ( impl->capture_enabled ) {
        //----------------------------------------------------------
        // capture this frame
        //----------------------------------------------------------
        glReadPixels( 0, 0, impl->win_w, impl->win_h, GL_RGBA, GL_UNSIGNED_BYTE, impl->capture_buff );
        fwrite( impl->capture_buff, sizeof(unsigned int)*impl->win_w*impl->win_h, 1, impl->capture_stream );
    }
}

void Sys::toggle_fullscreen( void )
{
    Config * config = impl->config;
    config->win_fullscreen = !config->win_fullscreen;
    if ( config->win_fullscreen ) {
        glutFullScreen();
    } else {
        impl->win_w = config->win_w;
        impl->win_h = config->win_h;
        glutPositionWindow( config->win_off_x, config->win_off_y );
        glutReshapeWindow( config->win_w, config->win_h );
    }
}

#ifdef EMULATE_BUFFERS
//
// EMULATE Buffer-Related Functions (makes it a little easier to debug in immediate mode)
//
#define BUFF_MAX 1024
static GLuint           buff_cnt = 0;
static const GLvoid *   buff_ptr[BUFF_MAX] = {nullptr};
static GLuint           buff_used[BUFF_MAX] = {0};
static GLuint           buff_bound[2] = {BUFF_MAX};

// use solid colors for textures for now
//
void GenBuffers( GLsizei n, GLuint * buffers )
{
    while( n > 0 ) 
    {
        *buffers = buff_cnt;
        buffers++;
        buff_cnt++;
        n--;
    }
}

void BindBuffer( GLenum target, GLuint buffer )
{
    dassert( target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER );
    GLuint which = (target == GL_ARRAY_BUFFER) ? 0 : 1;
    buff_bound[which] = buffer;
    dprintf( "BindBuffer: buff_bound[%d]=%d\n", which, buffer );
}

void BufferData( GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage )
{
    GLuint which = (target == GL_ARRAY_BUFFER) ? 0 : 1;
    GLuint size1 = (target == GL_ARRAY_BUFFER) ? sizeof( Vertex ) : sizeof( unsigned short );
    GLuint buffer = buff_bound[which];
    dassert( buffer < BUFF_MAX );

    buff_ptr[buffer] = data;
    buff_used[buffer] = size / size1;
    dprintf( "BufferData: buff_used[%d]=%d\n", buffer, buff_used[buffer] );
}

void EnableVertexAttribArray( GLuint index )
{
    // NOP
}

void DisableVertexAttribArray( GLuint index )
{
    // NOP
}

void VertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer )
{
    // NOP
}

void DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid * indices )
{
    // talk in terms Vertex and Triangle structures and counts
    //
    GLuint vbo_index = buff_bound[0];
    GLuint ibo_index = buff_bound[1];
    dassert( vbo_index < BUFF_MAX );
    dassert( ibo_index < BUFF_MAX );
    dprintf( "DrawElements: count=%d buff_used[%d]=%d\n", count, ibo_index, buff_used[ibo_index] );
    if ( GLuint(count) > buff_used[ibo_index] ) exit( 1 );
    dassert( buff_ptr[ibo_index] == indices && GLuint(count) <= buff_used[ibo_index] );

    const Vertex   * vbo = reinterpret_cast<const Vertex *>( buff_ptr[vbo_index] );
    const GLushort * ibo = reinterpret_cast<const GLushort *>( buff_ptr[ibo_index] ); 

    // draw each triangle, substituting bogus colors for textures for now
    //
    glBegin( GL_TRIANGLES );
        for( int i = 0; i < count; i += 3 )
        {
            for( int j = 0; j < 3; j++, ibo++ ) 
            {
                GLushort vi = *ibo;  
                const Vertex * v = &vbo[vi];
                int texid = v->texid;
                float r = float( (texid >> 16) & 0xff ) / 255.0f;
                float g = float( (texid >>  8) & 0xff ) / 255.0f;
                float b = float( (texid >>  0) & 0xff ) / 255.0f;

		dprintf( "DrawElements: i=%d j=%d vi=%d position=[%f, %f, %f] normal=[%f, %f, %f] rgb=[%f, %f, %f]\n", 
                         i, j, vi,
                         v->position[0], v->position[1], v->position[2], 
                         v->normal[0],   v->normal[1],   v->normal[2], 
                         r, g, b );
                glColor3f( r, g, b );
                glNormal3f( v->normal[0], v->normal[1], v->normal[2] );
                glVertex3f( v->position[0], v->position[1], v->position[2] );
            }
        }
    glEnd();
}

#endif
