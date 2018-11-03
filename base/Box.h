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
#ifndef _Box_h
#define _Box_h

#include "Entity.h"

class Box : public Entity
{
public:
    Box( Entity * parent, World * world, bool for_outer, 
         float x, float y, float z, float w, float h, float d,
         int texid_top, int texid_sides, int texid_bottom, int changes = GEOM_CHANGES_RARELY );
    ~Box();

    // Change texid(s)
    //
    void        texid_set( int texid_top, int texid_sides = -1, int texid_bottom = -1 );

    // Each Box may have an inner and outer version of itself with different world, geometry, etc.
    // These optional methods are used to go from the inner or outer Box from the other.
    //
    void        inner_set( Box * inner );
    Box *       inner_get( void );

    void        outer_set( Box * outer );
    Box *       outer_get( void );

private:
    class Impl;
    Impl * impl;
};

#endif
