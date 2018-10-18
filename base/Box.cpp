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
#include "Box.h"
#include "Sys.h"

class Box::Impl
{
public:
    bool     for_outer;
    Box *    other;

    Vertex   vertex[24];
    Triangle triangle[12];
};

Box::Box( Entity * parent, World * world, bool for_outer, 
          float x, float y, float z, float w, float h, float d,
          int texid_top, int texid_sides, int texid_bottom, int changes )
    : Entity( parent, world, x, y, z, w, h, d )
{
    impl = new Box::Impl;

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
    if ( d > 0.0f ) {
        z += d;
        d = -d;
    }

    //-----------------------------------------------------
    // Construct geometry for inner or outer cube faces with
    // appropriate normals.  In order to get the normals correct,
    // we need a separate set of vertices for each face, so
    // 6x4=24 in total.  Each face has two triangles, which
    // can use two of the same vertices of the face.  So 12
    // triangles in total.
    //
    // Always initially set up things for outer faces:
    //     front, right, back, left, top, bottom
    //
    // Each face has vertexes labeled (for outer):
    //
    //       3      2
    //      
    //       0      1
    //
    //     First  triangle is: 0 2 3
    //     Second triangle is: 2 0 1
    //
    // For inner, we simply reverse the normals and change the 
    // triangle orders as follows:
    //
    //     First  triangle is: 1 3 2
    //     Second triangle is: 3 1 0
    //-----------------------------------------------------
    impl->for_outer = for_outer;
    impl->other = nullptr;

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
        v->position[Z] = z;
        v->normal[X]   = 0.0;
        v->normal[Y]   = 0.0;
        v->normal[Z]   = for_outer ? 1.0 : -1.0;

        v->texid       = texid_sides;
        v->texcoord[0] = x + x_add;
        v->texcoord[1] = y + y_add;
    }


    // right
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float y_add = (i == 0 || i == 1) ? 0 : h;
        float z_add = (i == 0 || i == 3) ? 0 : d;
        v->position[X] = x + w;
        v->position[Y] = y + y_add;
        v->position[Z] = z + z_add;
        v->normal[X]   = for_outer ? 1.0 : -1.0;
        v->normal[Y]   = 0.0;
        v->normal[Z]   = 0.0;

        v->texid       = texid_sides;
        v->texcoord[0] = z + z_add;
        v->texcoord[1] = y + y_add;
    }

    // back
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float x_add = (i == 0 || i == 3) ? w : 0;
        float y_add = (i == 0 || i == 1) ? 0 : h;
        v->position[X] = x + x_add;
        v->position[Y] = y + y_add;
        v->position[Z] = z + d;
        v->normal[X]   = 0.0;
        v->normal[Y]   = 0.0;
        v->normal[Z]   = for_outer ? -1.0 : 1.0;

        v->texid       = texid_sides;
        v->texcoord[0] = x + x_add;
        v->texcoord[1] = y + y_add;
    }

    // left
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float y_add = (i == 0 || i == 1) ? 0 : h;
        float z_add = (i == 0 || i == 3) ? d : 0;
        v->position[X] = x;
        v->position[Y] = y + y_add;
        v->position[Z] = z + z_add;
        v->normal[X]   = for_outer ? -1.0 : 1.0;
        v->normal[Y]   = 0.0;
        v->normal[Z]   = 0.0;

        v->texid       = texid_sides;
        v->texcoord[0] = z + z_add;
        v->texcoord[1] = y + y_add;
    }

    // top
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float x_add = (i == 0 || i == 3) ? 0 : w;
        float z_add = (i == 0 || i == 1) ? 0 : d;
        v->position[X] = x + x_add;
        v->position[Y] = y + h;
        v->position[Z] = z + z_add;
        v->normal[X]   = 0.0;
        v->normal[Y]   = for_outer ? 1.0 : -1.0;
        v->normal[Z]   = 0.0;

        v->texid       = texid_top;
        v->texcoord[0] = x + x_add;
        v->texcoord[1] = z + z_add;
    }

    // bottom
    //
    for( int i = 0; i < 4; i++, v++ )
    {
        float x_add = (i == 0 || i == 3) ? w : 0;
        float z_add = (i == 0 || i == 1) ? 0 : d;
        v->position[X] = x + x_add;
        v->position[Y] = y;
        v->position[Z] = z + z_add;
        v->normal[X]   = 0.0;
        v->normal[Y]   = for_outer ? -1.0 : 1.0;
        v->normal[Z]   = 0.0;

        v->texid       = texid_bottom;
        v->texcoord[0] = x + x_add;
        v->texcoord[1] = z + z_add;
    }

    // all pairs of triangles look the same in terms of local vertex order within face
    //
    Triangle * t = impl->triangle;
    unsigned short off = 0;
    for( int i = 0; i < 6; i++ )
    {
        if ( for_outer ) {
            t->v0 = off + 0;
            t->v1 = off + 1;
            t->v2 = off + 2;
            t++;

            t->v0 = off + 0;
            t->v1 = off + 2;
            t->v2 = off + 3;
            t++;
        } else {
            t->v0 = off + 1;
            t->v1 = off + 0;
            t->v2 = off + 3;
            t++;

            t->v0 = off + 1;
            t->v1 = off + 3;
            t->v2 = off + 2;
            t++;
        }
        off += 4;
    }

    //-----------------------------------------------------
    // Now add it to the world.
    //-----------------------------------------------------
    this->geom_set( impl->vertex, 24, impl->triangle, 12 );

    for( int i = 0; i < 24; i++ ) 
    {
        v = &impl->vertex[i];
        if ( i == 0 ) dprintf( "Box: x=%f y=%f z=%f w=%f h=%f d=%f\n", x, y, z, w, h, d );
        dprintf( "    vertex[%d] = [%f, %f, %f]\n", i, v->position[X], v->position[Y], v->position[Z] );
    }
}

Box::~Box()
{
    this->geom_remove(); 
    delete impl;
    impl = nullptr;
}

void Box::texid_set( int texid_top, int texid_sides, int texid_bottom )
{
    Vertex * v = impl->vertex;

    if ( texid_sides == -1 ) {
        texid_sides = texid_top;
    }
    if ( texid_bottom == -1 ) {
        texid_bottom = texid_top;
    }

    for( int s = 0; s < 6; s++ )
    {
        for( int i = 0; i < 4; i++, v++ ) 
        {
            v->texid = (s < 4)  ? texid_sides :
                       (s == 4) ? texid_top : texid_bottom;
        }
    }

    this->geom_changed();
}

void Box::inner_set( Box * inner )
{
    dassert( impl->for_outer );
    impl->other = inner;
}

Box * Box::inner_get( void )
{
    dassert( impl->for_outer );
    return impl->other;
}

void Box::outer_set( Box * outer )
{
    dassert( !impl->for_outer );
    impl->other = outer;
}

Box * Box::outer_get( void )
{
    dassert( !impl->for_outer );
    return impl->other;
}
