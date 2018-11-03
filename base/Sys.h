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

// Sys.h - interface to all system dependencies (graphics, mouse, keyboard, files, etc.)
//
#ifndef _Sys_h
#define _Sys_h

#include "Misc.h"
#include "Config.h"
#include "World.h"

// geometry for one object (World.cpp should be the only user)
//
class Geom
{
public:
    bool                valid;
    bool                visible;
    int                 changes;
    bool                changed;
    Vertex *            vertex;
    unsigned int        vertex_cnt;
    Triangle *          triangle;
    unsigned int        triangle_cnt;
};

// text for 2D overlays
//
class Text2D
{
public:
    int                 x;
    int                 y;
    int                 rgb;
    const char *        str;
};

class Sys 
{
public:
    Sys( Config * config );

    void     world_set( World * w );
    World *  world_get( void );

    // called by World to allocate or deallocate a batch
    //
    int      batch_alloc( int vertex_cnt, int triangle_cnt );
    void     batch_free( int batch_hdl );

    // called by main.cpp to transfer control to Sys
    // there is no return from this call
    //
    void     main_loop( void );

    // quit() the process gracefully
    // 
    void     quit( int status );

    // called by World to force redraw
    //
    void     force_redraw( void );

    // called by World to draw frame
    //
    void     draw_begin( bool use_ortho,
                         float fov_y,  float near_z, float far_z,
                         float eye[],  float view[], float up[] );
    void     draw_batch( int batch_hdl, Geom * geom_array, int geom_cnt, bool geom_changed );
    void     draw_text2d( Text2D * text, int text_cnt );
    void     draw_end( void );

    // called by World to toggle fullscreen mode
    //
    void     toggle_fullscreen( void );

private:
    class Impl;
    Impl * impl;
};

//------------------------------------------
// Key Names
//------------------------------------------
#define SYS_KEY_UNKNOWN      -1
#define SYS_KEY_SPECIAL      256
#define SYS_KEY_F1           (SYS_KEY_SPECIAL+1)
#define SYS_KEY_F2           (SYS_KEY_SPECIAL+2)
#define SYS_KEY_F3           (SYS_KEY_SPECIAL+3)
#define SYS_KEY_F4           (SYS_KEY_SPECIAL+4)
#define SYS_KEY_F5           (SYS_KEY_SPECIAL+5)
#define SYS_KEY_F6           (SYS_KEY_SPECIAL+6)
#define SYS_KEY_F7           (SYS_KEY_SPECIAL+7)
#define SYS_KEY_F8           (SYS_KEY_SPECIAL+8)
#define SYS_KEY_F9           (SYS_KEY_SPECIAL+9)
#define SYS_KEY_F10          (SYS_KEY_SPECIAL+10)
#define SYS_KEY_F11          (SYS_KEY_SPECIAL+11)
#define SYS_KEY_F12          (SYS_KEY_SPECIAL+12)
#define SYS_KEY_UP           (SYS_KEY_SPECIAL+13)
#define SYS_KEY_DOWN         (SYS_KEY_SPECIAL+14)
#define SYS_KEY_LEFT         (SYS_KEY_SPECIAL+15)
#define SYS_KEY_RIGHT        (SYS_KEY_SPECIAL+16)
#define SYS_KEY_PAGE_UP      (SYS_KEY_SPECIAL+17)
#define SYS_KEY_PAGE_DOWN    (SYS_KEY_SPECIAL+18)
#define SYS_KEY_HOME         (SYS_KEY_SPECIAL+19)
#define SYS_KEY_END          (SYS_KEY_SPECIAL+20)
#define SYS_KEY_INSERT       (SYS_KEY_SPECIAL+21)

//------------------------------------------
// Button names
//------------------------------------------
#define SYS_BUTTON_LEFT      0
#define SYS_BUTTON_RIGHT     1
#define SYS_BUTTON_MIDDLE    2
#define SYS_BUTTON_COUNT     3

//------------------------------------------
// Scroll names
//------------------------------------------
#define SYS_SCROLL_VERTICAL   0
#define SYS_SCROLL_HORIZONTAL 1

//------------------------------------------
// Actions
//------------------------------------------
#define SYS_ACTION_RELEASE      0
#define SYS_ACTION_PRESS        1

#define SYS_ACTION_SCROLL_LEFT  0
#define SYS_ACTION_SCROLL_RIGHT 1
#define SYS_ACTION_SCROLL_UP    1
#define SYS_ACTION_SCROLL_DOWN  1

//------------------------------------------
// Modifiers (mask)
//------------------------------------------
#define SYS_MOD_SHIFT        0x1
#define SYS_MOD_ALT          0x2
#define SYS_MOD_CTRL         0x4

#endif
