// Copyright (c) 2017-2019 Robert A. Alfieri
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

#include "ConfigViz.h"
#include "Color.h"
#include "Sys.h"
#include "math.h"

ConfigViz::ConfigViz( int argc, const char * argv[] )
    : Config( argc, argv )
{
    // default values
    //
    this->viz_path = 0;
    this->viz_last = 0;
    this->texid_background = Color::rgb( "black" );

    //----------------------------------------------------------------
    // process command-line args
    //----------------------------------------------------------------
    const char * view = "top";
    for( int i = 1; i < argc; i++ )
    {
        if ( strcmp( argv[i], "-viz_path" ) == 0 ) {
            this->viz_path = argv[++i];
        } else if ( strcmp( argv[i], "-viz_last" ) == 0 ) {
            this->viz_last = atoi( argv[++i] ); 
        } else if ( strcmp( argv[i], "-viz_view" ) == 0 ) {
            view = argv[++i];
        }
    }

    //----------------------------------------------------------------
    // Change view position.
    // Need to save this stuff in files.
    //----------------------------------------------------------------
    if ( strcmp( view, "front" ) == 0 ) {
        this->win_eye[0]  = 39.37;
        this->win_eye[1]  = 16.4;
        this->win_eye[2]  = 364.0;
        this->win_view[0] = 39.37;
        this->win_view[1] = 16.4;
        this->win_view[2] = 363.0;
        this->win_fov_y   = 78.71;
    } else if ( strcmp( view, "left" ) == 0 ) {
        this->win_eye[0]  = -202.94;
        this->win_eye[1]  = 164.4;
        this->win_eye[2]  = -9.026284;
        this->win_view[0] = -201.936417;
        this->win_view[1] = 164.4;
        this->win_view[2] = -9.017079;
        this->win_fov_y   = 78.71;
    } else if ( strcmp( view, "backright" ) == 0 ) { 
        this->win_eye[0]  = 4783.8;
        this->win_eye[1]  = 364.4;
        this->win_eye[2]  = 400.212;
        this->win_view[0] = 4783.9;
        this->win_view[1] = 363.8;
        this->win_view[2] = 399.215;
        this->win_fov_y   = 78.71;
    } else if ( strcmp( view, "right" ) == 0 ) {
        this->win_eye[0]  = 120.0;
        this->win_eye[1]  = 2.4;
        this->win_eye[2]  = 295.0;
        this->win_view[0] = 68.0;
        this->win_view[1] = 2.4;
        this->win_view[2] = 295.32;
        this->win_fov_y   = 6.6;
    } else {
        this->win_eye[0]  = 1.2;
        this->win_eye[1]  = 45.2;
        this->win_eye[2]  = 300.0;
        this->win_view[0] = 1.2;
        this->win_view[1] = 32.5;
        this->win_view[2] = 299.0;
        this->win_fov_y   = 11.71;
    }
    //this->win_mouse_motion_enabled = true;
    
}

ConfigViz::~ConfigViz()
{
}
