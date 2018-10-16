// Copyright (c) 2014-2017 Robert A. Alfieri
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
#include "Rectangle.h"
#include "Sys.h"

class Rectangle::Impl
{
public:
    Vertex   vertex[4];
    Triangle triangle[2];
};

Rectangle::Rectangle( Entity * parent, World * world, 
                      float x, float y, float w, float h,
                      int texid, int changes )
    : Entity( parent, world, x, y, 0.0, w, h, 0.0 )
{
    impl = new Rectangle::Impl;

    //-----------------------------------------------------
    // Temporary Hack to make code below happy.
    //-----------------------------------------------------
    if ( w < 0.0f ) {
        x += w;
        w = -w;
    }
    if ( h < 0.0f ) {
        y += h;
        h = -h;
    }

    //-----------------------------------------------------
    // Construct geometry for the rectangle.
    //-----------------------------------------------------
    const int X = 0;
    const int Y = 1;
    const int Z = 2;

    Vertex *   v = impl->vertex;

    // front
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float x_add = (i == 0 || i == 3) ? 0 : w;
        float y_add = (i == 0 || i == 1) ? 0 : h;
        v->position[X] = x + x_add;
        v->position[Y] = y + y_add;
        v->position[Z] = 0.0;
        v->normal[X]   = 0.0;
        v->normal[Y]   = 0.0;
        v->normal[Z]   = 1.0;

        v->texid       = texid;
        v->texcoord[0] = x + x_add;
        v->texcoord[1] = y + y_add;
    }

    // all pairs of triangles look the same in terms of local vertex order within face
    //
    Triangle * t = impl->triangle;
    t->v0 = 0;
    t->v1 = 1;
    t->v2 = 2;
    t++;

    t->v0 = 0;
    t->v1 = 2;
    t->v2 = 3;

    //-----------------------------------------------------
    // Now add it to the world.
    //-----------------------------------------------------
    this->geom_set( impl->vertex, 4, impl->triangle, 2 );

    for( int i = 0; i < 4; i++ ) 
    {
        Vertex * v = &impl->vertex[i];
        if ( i == 0 ) dprintf( "Rectangle: x=%f y=%f w=%f h=%f\n", x, y, w, h );
        dprintf( "    vertex[%d] = [%f, %f]\n", i, v->position[X], v->position[Y] );
    }
}

Rectangle::~Rectangle()
{
    this->geom_remove(); 
    delete impl;
    impl = 0;
}

void Rectangle::texid_set( int texid ) 
{
    Vertex * v = impl->vertex;

    for( int i = 0; i < 4; i++, v++ ) 
    {
        v->texid = texid;
    }

    this->geom_changed();
}
