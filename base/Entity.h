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

// Base class for 3D entities (objects)
//
#ifndef _Entity_h
#define _Entity_h

#include "World.h"

class Entity
{
public:
    Entity( Entity * parent, World * world, float x, float y, float z, float w, float h, float d );
    virtual ~Entity();

    void xyz_get( float * x, float * y, float * z );
    void whd_get( float * w, float * h, float * d );
    void center_get( float * x, float * y, float * z );

    float distance( Entity * other, bool include_y = true );              // these measure from the centers
    float manhattan_distance( Entity * other, bool include_y = true );

    void visible_set( bool visible );
    bool visible_get( void );

    void changes_set( int changes );
    int  changes_get( void );

    void move_to( float x, float y, float z );  // absolute
    void move_by( float x, float y, float z );  // relative
    void move_in_dir( int dir, float by );      // relative along dir

    Entity * child_first( void );
    Entity * sibling( void );
    void     children_print( void );

protected:
    virtual void geom_set( Vertex * vertex, int vertex_cnt, Triangle * triangle, int triangle_cnt, int changes = GEOM_CHANGES_RARELY );
    virtual void geom_get( Vertex ** vertex, int * vertex_cnt, Triangle ** triangle, int * triangle_cnt );
    virtual void geom_remove( void );
    virtual void geom_changed( void );

    virtual void input( World * world,
                        float   mouse_x,
                        float   mouse_y,
                        bool    mouse_left_click,
                        bool    mouse_right_click,
                        bool    mouse_drag,
                        bool    mouse_scroll,
                        int     mouse_scroll_amount,
                        bool    key_press,
                        int     key );

private:
    class Impl;
    Impl * impl;
};

#endif
