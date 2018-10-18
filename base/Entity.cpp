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
#include "Entity.h"
#include "Sys.h"
#include "Misc.h"

class Entity::Impl
{
public:
    World  * world;
    int      geom_hdl;
    Vertex * vertex;
    int      vertex_cnt;
    Triangle*triangle;
    int      triangle_cnt;
    Entity * parent;
    Entity * child_first;
    Entity * sibling;
    float    x, y, z;
    float    w, h, d;
};

Entity::Entity( Entity * parent, World * world, float x, float y, float z, float w, float h, float d )
{
    impl = new Entity::Impl();

    impl->world = world;
    impl->geom_hdl = -1;
    impl->vertex = nullptr;
    impl->vertex_cnt = 0;
    impl->triangle = nullptr;
    impl->triangle_cnt = 0;
    impl->parent = parent;
    if ( parent != nullptr ) {
        impl->sibling = parent->impl->child_first;
        impl->parent->impl->child_first = this;
    } else {
        impl->sibling = nullptr;
    }
    impl->child_first = nullptr;
    impl->x = x;
    impl->y = y;
    impl->z = z;
    impl->w = w;
    impl->h = h;
    impl->d = d;
}

Entity::~Entity()
{
    // children first
    //
    Entity * sibling = nullptr;
    for( Entity * c = impl->child_first; c != nullptr; c = sibling )
    {
        sibling = c->impl->sibling;
        printf( "~Entity() child\n" );
        delete c;
    }
    dassert( impl->child_first == nullptr );

    // remove this node from parent
    //
    if ( impl->parent != nullptr ) {
        Entity ** child_ptr_ptr;
        Entity *  child_ptr;
        for( child_ptr_ptr = &impl->parent->impl->child_first, child_ptr = *child_ptr_ptr; 
             child_ptr != this; 
             child_ptr_ptr = &child_ptr->impl->sibling, child_ptr = *child_ptr_ptr )
        {
        }
        *child_ptr_ptr = impl->sibling;
        printf( "~Entity() remove from parent\n" );
        impl->sibling = nullptr;
        impl->parent = nullptr;
    }

    // now we can delete this node
    //
    printf( "~Entity() remove geom\n" );
    this->geom_remove();
    delete impl;
    impl = nullptr;
}

void Entity::xyz_get( float * x, float * y, float * z )
{
    *x = impl->x;
    *y = impl->y;
    *z = impl->z;
}

void Entity::whd_get( float * w, float * h, float * d )
{
    *w = impl->w;
    *h = impl->h;
    *d = impl->d;
}

void Entity::center_get( float * x, float * y, float * z )
{
    *x = impl->x + impl->w/2.0f;
    *y = impl->y + impl->h/2.0f;
    *z = impl->z + impl->d/2.0f;
}

float Entity::distance( Entity * other, bool include_y )
{
    float x0, x1, y0, y1, z0, z1;

    this->center_get( &x0, &y0, &z0 );
    other->center_get( &x1, &y1, &z1 );
    if ( !include_y ) {
        y0 = 0.0f;
        y1 = 0.0f;
    }

    return sqrtf( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) + (z1-z0)*(z1-z0) );
}

float Entity::manhattan_distance( Entity * other, bool include_y )
{
    float x0, x1, y0, y1, z0, z1;

    this->center_get( &x0, &y0, &z0 );
    other->center_get( &x1, &y1, &z1 );
    if ( !include_y ) {
        y0 = 0.0f;
        y1 = 0.0f;
    }

    return fabsf( x0 - x1 ) + fabsf( y0 - y1 ) + fabsf( z0 - z1 );
}

void Entity::geom_set( Vertex * vertex, int vertex_cnt, Triangle * triangle, int triangle_cnt, int changes )
{
    this->geom_remove();
    impl->vertex = vertex;
    impl->vertex_cnt = vertex_cnt;
    impl->triangle = triangle;
    impl->triangle_cnt = triangle_cnt;
    impl->geom_hdl = impl->world->geom_add( vertex, vertex_cnt, triangle, triangle_cnt, changes );
}

void Entity::geom_get( Vertex ** vertex, int * vertex_cnt, Triangle ** triangle, int * triangle_cnt )
{
    *vertex = impl->vertex;
    *vertex_cnt = impl->vertex_cnt;
    *triangle = impl->triangle;
    *triangle_cnt = impl->triangle_cnt;
}

void Entity::geom_remove( void )
{
    if ( impl->geom_hdl != -1 ) {
        impl->world->geom_remove( impl->geom_hdl );
        impl->vertex = nullptr;
        impl->vertex_cnt = 0;
        impl->triangle = nullptr;
        impl->triangle_cnt = 0;
        impl->geom_hdl = 0;
    }
}

void Entity::geom_changed( void )
{
    impl->world->geom_changed( impl->geom_hdl );
}

void Entity::visible_set( bool visible )
{
    if ( impl->geom_hdl != -1 ) {
        impl->world->geom_visible_set( impl->geom_hdl, visible );
    }
    for( Entity * child = this->child_first(); child != nullptr; child = child->sibling() )
    {
        child->visible_set( visible );
    }
}

bool Entity::visible_get( void )
{
    return impl->world->geom_visible_get( impl->geom_hdl );
}

void Entity::changes_set( int changes )
{
    if ( impl->geom_hdl != -1 ) {
        impl->world->geom_changes_set( impl->geom_hdl, changes );
    }
    for( Entity * child = this->child_first(); child != nullptr; child = child->sibling() )
    {
        child->changes_set( changes );
    }
}

int Entity::changes_get( void )
{
    return impl->world->geom_changes_get( impl->geom_hdl );
}

void Entity::move_to( float x, float y, float z )
{
    dprintf( "move_to: x=%f y=%f z=%f\n", x, y, z );
    float x_change = x - impl->x;
    float y_change = y - impl->y;
    float z_change = z - impl->z;
    impl->x = x;
    impl->y = y;
    impl->z = z;

    for( int i = 0; i < impl->vertex_cnt; i++ ) 
    {
        impl->vertex[i].position[0] += x_change;
        impl->vertex[i].position[1] += y_change;
        impl->vertex[i].position[2] += z_change;
    }
    this->geom_changed(); 
}

void Entity::move_by( float x, float y, float z )
{
    this->move_to( impl->x + x, impl->y + y, impl->z + z );
}

void Entity::move_in_dir( int dir, float by )
{
    float x_change = (dir == DIR_X_NEG) ? -by : (dir == DIR_X_POS) ? by : 0.0f;
    float y_change = (dir == DIR_Y_NEG) ? -by : (dir == DIR_Y_POS) ? by : 0.0f;
    float z_change = (dir == DIR_Z_NEG) ? -by : (dir == DIR_Z_POS) ? by : 0.0f;

    this->move_to( impl->x + x_change, impl->y + y_change, impl->z + z_change );
}

Entity * Entity::child_first( void )
{
    return impl->child_first;
}

Entity * Entity::sibling( void )
{
    return impl->sibling;
}

void Entity::children_print( void )
{
    printf ( "Children:\n" );
    for( Entity * child = this->child_first(); child != nullptr; child = child->sibling() )
    {
        printf( "    child x=%f y=%f z=%f\n", child->impl->x, child->impl->y, child->impl->z );
    }
}

void Entity::input( World*world,
                    float mouse_x,
                    float mouse_y,
                    bool  mouse_left_click,
                    bool  mouse_right_click,
                    bool  mouse_drag,
                    bool  mouse_scroll,
                    int   mouse_scroll_amount,
                    bool  key_press,
                    int   key )
{
    for( Entity * c = impl->child_first; c != nullptr; c = c->impl->sibling ) 
    {
        if ( c->visible_get() ) {
            c->input( world, 
                      mouse_x, mouse_y, 
                      mouse_left_click, mouse_right_click, mouse_drag, mouse_scroll, mouse_scroll_amount, 
                      key_press, key );
        }
    }
}
