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
#ifndef _Arrow_h
#define _Arrow_h

#include "Entity.h"

class Arrow : public Entity
{
public:
    Arrow( Entity * parent, World * world, 
           float x, float y, float z,           // center of arrow
           float w, float h, float d,           // width, height (length), and depth
           float rotation,                      // about center, 0 .. 2*PI
           int   texid,
           int   changes = GEOM_CHANGES_RARELY );
    ~Arrow();

    // keep tail the same; change w, h, and d
    //
    void whd_set( float w, float h, float d );

    // rotation radians go from 0 to 2*PI (can also be negative)
    // this is about the center for now
    //
    void rotation_set( float rotation );

private:
    class Impl;
    Impl * impl;
};

#endif
