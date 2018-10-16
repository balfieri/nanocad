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

// Config - holds configuration information 
//
#ifndef _Config_h
#define _Config_h

class Config 
{
public:
    Config( int argc, const char * argv[] );

    // window/graphics
    //
    char *       win_name;
    int          win_w;
    int          win_h;
    int          win_off_x;
    int          win_off_y;
    bool         win_fullscreen;
    bool         win_ortho;
    float        win_eye[3];
    float        win_view[3];
    float        win_up[3];
    float        win_fov_y;
    float        win_near_z;
    float        win_far_z;
    float        win_perspective_fudge_factor;
    bool         win_mouse_motion_enabled;
    int          win_batch_cnt;
    int          win_batch_geom_cnt;
    int          win_batch_vertex_cnt;
    int          win_batch_triangle_cnt;
    bool         win_capture_enabled;
    int          win_overlay_text_color;
    int          win_overlay_text_height;
    int          win_overlay_text_width;
    float        win_overlay_text_line_width;
    float        win_overlay_text_scale_factor;
    void *       win_overlay_text_font;
    bool         win_view_print;

    int          texid_background;
    int          texid_unseen;
};

#endif
