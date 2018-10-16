// Copyright (c) 2017-2018 Robert A. Alfieri
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

#ifndef _Viz_h
#define _Viz_h

#include "ConfigViz.h"
#include "World.h"
#include "Sys.h"

class Viz : public World
{
public:
    Viz( ConfigViz * config, Sys * sys );
    ~Viz();

protected:
    virtual void frame_begin( float wall_clock_ms );
    virtual void resize_event( double w, double h );
    virtual void motion_event( double x, double y );
    virtual void key_event( int key, int action, int modifiers );
    virtual void button_event( int button, int action, int modifiers );

private:
    class Impl;
    Impl * impl;
};

#endif
