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
#include "Arrow.h"
#include "Sys.h"

class Arrow::Impl
{
public:
    float    rotation;
    Vertex   vertex[24];
    Triangle triangle[12];
};

Arrow::Arrow( Entity * parent, World * world, 
              float x, float y, float z, float w, float h, float d, float rotation,
              int texid, 
              int changes )
    : Entity( parent, world, x, y, z, w, h, d )
{
    impl = new Arrow::Impl;

    const int X = 0;
    const int Y = 1;
    const int Z = 2;

    //-----------------------------------------------------
    // Reset rotation
    //-----------------------------------------------------
    this->rotation_set( rotation );

    //-----------------------------------------------------
    // Now add it to the world.
    //-----------------------------------------------------
    this->geom_set( impl->vertex, 24, impl->triangle, 12, changes );

    for( int i = 0; i < 24; i++ ) 
    {
        Vertex * v = &impl->vertex[i];
        if ( i == 0 ) dprintf( "Arrow: x=%f y=%f z=%f w=%f h=%f d=%f\n", x, y, z, w, h, d );
        dprintf( "    vertex[%d] = [%f, %f, %f]\n", i, v->position[X], v->position[Y], v->position[Z] );
    }
}

Arrow::~Arrow()
{
    this->geom_remove(); 
    delete impl;
    impl = 0;
}

void Arrow::whd_set( float w, float h, float d )
{
}

void Arrow::rotation_set( float rotation )
{
    impl->rotation = rotation;
}
