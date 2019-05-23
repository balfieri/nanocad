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

// Config - holds configuration information (can be per-world or universe)
//
#include "Config.h"
#include "Color.h"
#include "Sys.h"
#include "math.h"

Config::Config( int argc, const char * argv[] )
{
    this->win_name  = strdup( "" );
    this->win_w     = 1920; 
    this->win_h     = 1080;
    this->win_off_x = 400;
    this->win_off_y = 400;
    this->win_fullscreen = false;
    this->win_ortho = false;
    this->win_lookfrom[0] = 0.0; 
    this->win_lookfrom[1] = 0.0;
    this->win_lookfrom[2] = 100.f;
    this->win_lookat[0] = win_lookfrom[0];
    this->win_lookat[1] = win_lookfrom[1];
    this->win_lookat[2] = this->win_lookfrom[2] - 1.00f;
    this->win_vup[0]   = 0.0f;
    this->win_vup[1]   = 1.0f;
    this->win_vup[0]   = 0.0f;
    this->win_vfov = 8.0f; 
    this->win_near_z = 0.01f;
    this->win_far_z = 10000.0f;
    this->win_perspective_fudge_factor = 1.0f;   
    this->win_mouse_motion_enabled = true;
    this->win_batch_cnt = 16*1024;
    this->win_batch_geom_cnt = 1024;
    this->win_batch_vertex_cnt = this->win_batch_geom_cnt * 16;
    this->win_batch_triangle_cnt = this->win_batch_vertex_cnt / 2;
    this->win_capture_enabled = false;
    this->win_overlay_text_color = Color::rgb( "gray" );
    this->win_overlay_text_scale_factor = 0.2;
    this->win_overlay_text_height = 42;
    this->win_overlay_text_width = 27;
    this->win_overlay_text_line_width = 3.0f;
    this->win_overlay_text_font = nullptr;
    this->win_view_print = false;

    // hard code texids for now
    // we don't really have textures yet, just phony colors
    //
    this->texid_background      = Color::rgb( "black" );
    this->texid_unseen          = Color::rgb( "black" );

    // process command-line args
    //
    for( int i = 1; i < argc; i++ )
    {
        if ( strcmp( argv[i], "-win_fullscreen" ) == 0 ) {
            this->win_fullscreen = true;
        } else if ( strcmp( argv[i], "-win_capture" ) == 0 ) {
            this->win_capture_enabled = true;
        } else if ( strcmp( argv[i], "-win_lookfrom_z" ) == 0 ) {
            this->win_lookfrom[2] = atof( argv[++i] );
            this->win_lookat[2] = this->win_lookfrom[2] - 0.5f;
        } else if ( strcmp( argv[i], "-win_view_print" ) == 0 ) {
            this->win_view_print = true;
        } else if ( strcmp( argv[i], "-win_ortho" ) == 0 ) {
            this->win_ortho = true;
        }
    }
}
