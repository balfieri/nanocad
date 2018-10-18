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
#include "List.h"
#include "Misc.h"

//---------------------------------------
// List Entry
//---------------------------------------
class Entry 
{
public:
    nKind kind;
    union
    {
        nInt    i;
        nFlt    f;
        nStr    s;
        Hash *  hp;
        List *  lp;
    } u;
};

class List::Impl
{
public:
    int     count;              // entries used
    int     alloc_count;        // allocated entries
    Entry * entries;            // array of allocated entries

    inline Entry * get( int i, nKind kind = UNDEF )
    {
        dassert( i >= 0 );
        dassert( i < this->count );
        Entry * e = &this->entries[i];
        dassert( kind == UNDEF || e->kind == kind );
        return e;
    }

    Entry * set( int i )
    {
        dassert( i >= 0 );
        if ( i >= this->alloc_count ) {
            //---------------------------------------
            // Resize Array
            //---------------------------------------
            int new_alloc_count = alloc_count << 1;
            while( new_alloc_count <= i ) 
            {
                new_alloc_count <<= 1;
            }

            Entry * new_entries = new Entry[ new_alloc_count ];
            memcpy( new_entries, this->entries, this->count * sizeof( Entry ) );
            memset( &new_entries[this->count], 0, (new_alloc_count-this->count) * sizeof( Entry ) );
            this->alloc_count = new_alloc_count;
            delete this->entries;
            this->entries = new_entries;
        }

        if ( i >= this->count ) {
            this->count = i+1;
        }

        return &this->entries[i];
    }

    void shift_up( void )
    {
        this->set( this->count );  // causes resize if needed
        memcpy( &this->entries[1], &this->entries[0], (this->count-1) * sizeof( Entry ) );
        // we'll assume that index 0 is set after this call, so no need to do anything here
    }

    void shift_down( void )
    {
        this->count--;
        memcpy( &this->entries[0], &this->entries[1], this->count * sizeof( Entry ) );
    }
};

const int INIT_ENTRY_CNT = 4;

//---------------------------------------
// List Constructor
//---------------------------------------
List::List( void )
{
    impl = new Impl;
    impl->count = 0;
    impl->alloc_count = INIT_ENTRY_CNT;
    impl->entries = new Entry[ INIT_ENTRY_CNT ];
    for( int i = 0; i < INIT_ENTRY_CNT; i++ )
    {
        impl->entries[i].kind = UNDEF;
    }
}

//---------------------------------------
// List Destructor
//---------------------------------------
List::~List( void )
{
    //---------------------------------------
    // No reference counts, just delete the apparatus.
    //---------------------------------------
    delete impl->entries;
    impl->entries = nullptr;
    impl = nullptr;
}

//---------------------------------------
// List Length
//---------------------------------------
int List::length( void )
{
    return impl->count;
}

//---------------------------------------
// List Property Exists?
//---------------------------------------
bool List::exists( int i )
{
    dassert( i >= 0 );
    return i < impl->count;
}

//---------------------------------------
// List Property Defined?
//---------------------------------------
bool List::defined( int i )
{
    Entry * e = impl->get( i );
    return e != nullptr && e->kind != UNDEF;
}

//---------------------------------------
// List Property Kind
//---------------------------------------
nKind List::kind( int i )
{
    Entry * e = impl->get( i );
    return (e == nullptr) ? UNDEF : e->kind; 
}

//---------------------------------------
// List Property Setters
//---------------------------------------
List& List::undef( int i )
{
    Entry * e = impl->set( i );
    e->kind = UNDEF;
    return *this;
}

List& List::i( int i, nInt v )
{
    Entry * e = impl->set( i );
    e->kind = INT;
    e->u.i = v;
    return *this;
}

List& List::f( int i, nFlt v )
{
    Entry * e = impl->set( i );
    e->kind = FLT;
    e->u.f = v;
    return *this;
}

List& List::s( int i, nStr v )
{
    Entry * e = impl->set( i );
    e->kind = STR;
    e->u.s = v;
    return *this;
}

List& List::hp( int i, Hash * v )
{
    Entry * e = impl->set( i );
    e->kind = HASH;
    e->u.hp = v;
    return *this;
}

List& List::lp( int i, List * v )
{
    Entry * e = impl->set( i );
    e->kind = LIST;
    e->u.lp = v;
    return *this;
}

//---------------------------------------
// List Property Getters
//---------------------------------------
nInt List::i( int i )
{
    Entry * e = impl->get( i, INT );
    return e->u.i;
}

nFlt List::f( int i )
{
    Entry * e = impl->get( i, FLT );
    return e->u.f;
}

nStr List::s( int i )
{
    Entry * e = impl->get( i, STR );
    return e->u.s;
}

Hash& List::h( int i )
{
    Entry * e = impl->get( i, HASH );
    return *e->u.hp;
}

Hash * List::hp( int i )
{
    Entry * e = impl->get( i, HASH );
    return e->u.hp;
}

List& List::l( int i )
{
    Entry * e = impl->get( i, LIST );
    return *e->u.lp;
}

List * List::lp( int i )
{
    Entry * e = impl->get( i, LIST );
    return e->u.lp;
}

//---------------------------------------
// List Append
//---------------------------------------
List& List::pushi( nInt v )
{
    return this->i( impl->count, v );
}

List& List::pushf( nFlt v )
{
    return this->f( impl->count, v );
}

List& List::pushs( nStr v )
{
    return this->s( impl->count, v );
}

List& List::pushhp( Hash * v )
{
    return this->hp( impl->count, v );
}

List& List::pushlp( List * v )
{
    return this->lp( impl->count, v );
}

//---------------------------------------
// List Pop Last
//---------------------------------------
nInt List::popi( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == INT );
    return e->u.i;
}

nFlt List::popf( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == FLT );
    return e->u.f;
}

nStr List::pops( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == STR );
    return e->u.s;
}

Hash& List::poph( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == HASH );
    return *e->u.hp;
}

Hash * List::pophp( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == HASH );
    return e->u.hp;
}

List& List::popl( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == LIST );
    return *e->u.lp;
}

List * List::poplp( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[--impl->count];
    dassert( e->kind == LIST );
    return e->u.lp;
}

//---------------------------------------
// List Prepend
//---------------------------------------
List& List::unshifti( nInt v )
{
    impl->shift_up();
    return this->i( 0, v );
}

List& List::unshiftf( nFlt v )
{
    impl->shift_up();
    return this->f( 0, v );
}

List& List::unshifts( nStr v )
{
    impl->shift_up();
    return this->s( 0, v );
}

List& List::unshifthp( Hash * v )
{
    impl->shift_up();
    return this->hp( 0, v );
}

List& List::unshiftlp( List * v )
{
    impl->shift_up();
    return this->lp( 0, v );
}

//---------------------------------------
// List Pop First
//---------------------------------------
nInt List::shifti( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == INT );
    nInt v = e->u.i;
    impl->shift_down();
    return v;
}

nFlt List::shiftf( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == FLT );
    nFlt v = e->u.f;
    impl->shift_down();
    return v;
}

nStr List::shifts( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == STR );
    nStr v = e->u.s;
    impl->shift_down();
    return v;
}

Hash& List::shifth( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == HASH );
    Hash& v = *e->u.hp;
    impl->shift_down();
    return v;
}

Hash * List::shifthp( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == HASH );
    Hash * v = e->u.hp;
    impl->shift_down();
    return v;
}

List& List::shiftl( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == LIST );
    List& v = *e->u.lp;
    impl->shift_down();
    return v;
}

List * List::shiftlp( void )
{
    dassert( impl->count > 0 );
    Entry * e = &impl->entries[0];
    dassert( e->kind == LIST );
    List * v = e->u.lp;
    impl->shift_down();
    return v;
}

List& List::print( nStr s )
{
    printf( "%s\n", s );
    Entry * e = impl->entries;
    for( int i = 0; i < impl->count; i++, e++ )
    {
        printf( "    %d => ", i );

        switch( e->kind )
        {
            case UNDEF: 
                printf( "undef" ); 
                break;

            case INT:
                printf( "%ld", e->u.i );
                break;

            case FLT:
                printf( "%f", e->u.f );
                break;

            case STR:
                printf( "%s", e->u.s );
                break;

            case HASH:
                printf( "hash" );
                break;

            case LIST:
                printf( "list" );
                break;

            default:
                dassert( 0 );
                break;
        } 
        printf( "\n" );
    }

    return *this;
}
