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
#include "Hash.h"
#include "assert.h"
#include "Misc.h"

#undef dprintf
#define dprintf if ( 0 ) printf

//---------------------------------------
// STATIC: String to Property ID 
//---------------------------------------
#include <map>
static std::map<nStr, int> str_id_map;
static std::map<int, nStr> id_str_map;
static int                 str_id_next = 0;

int Hash::str_to_id( nStr s )
{
    //---------------------------------------
    // See if a mapping already exists by iterating 
    // through all values because we can't use the string pointer (due to duplicates).
    // TODO: this is way too slow
    //---------------------------------------
    for( std::map<nStr, int>::iterator it = str_id_map.begin(); it != str_id_map.end(); it++ )
    {
        if ( strcmp( s, it->first ) == 0 ) {
            return it->second;  // id
        }
    }
        
    const char * s_dup = strdup( s );
    str_id_map[s_dup] = str_id_next;
    id_str_map[str_id_next] = s_dup;
    return str_id_next++;
}

nStr Hash::id_to_str( int id )
{
    //---------------------------------------
    // Mapping better exist.
    //---------------------------------------
    return id_str_map[id];
}

//---------------------------------------
// Hash Entry
//---------------------------------------
class Entry 
{
public:
    int   id;
    nKind kind;
    union
    {
        nInt          i;
        nFlt          f;
        nStr          s;
        Hash *        hp;
        List *        lp;
    } u;
};

class Hash::Impl
{
public:
    int     count;               // entries used
    int     mask;                // allocated entries-1
    Entry * entries;             // array of allocated entries

    inline Entry * get( int id, nKind kind = UNDEF )
    {
        //---------------------------------------
        // Hash to proper starting entry, then increment from there.
        //---------------------------------------
        dassert( id >= 0 );
        int i = id & this->mask;
        Entry * e = &this->entries[i];
        for( int j = 0; j <= this->mask; j++ )
        {
            if( e->id == id ) {
                if ( kind != UNDEF && kind != e->kind ) {
                    printf( "ERROR: wanted %s id=%d kind=%d got e->id=%d e->kind=%d\n", Hash::id_to_str( id ), id, kind, e->id, e->kind );
                    my_exit( 1 );
                }
                dprintf( "get() found entry i=%d id=%d kind=%d\n", i, id, kind );
                return e; // found
            } 

            if ( e->id == -1 ) {
                break;    // no possibility, so stop here
            }

            if( i == this->mask ) {
                i = 0;
                e = &this->entries[0];
            } else {
                i++;
                e++;
            }
        }
        if ( kind != UNDEF ) {
            printf( "ERROR: wanted (%s) id=%d kind=%d, did not find entry\n", Hash::id_to_str( id ), id, kind );
            my_exit( 1 );
        }
        return NULL;
    }

    Entry * set( int id, int recursion = 0 )
    {
        //---------------------------------------
        // See if we need to resize the hash.
        //---------------------------------------
        if ( this->count == (this->mask >> 1) ) {
            this->count = 0;
            int old_mask = this->mask;
            this->mask = (old_mask << 1) | 1;  // double size
            dprintf( "resize from %d to %d\n", old_mask+1, this->mask+1 );
            Entry * old_entries = this->entries;
            this->entries = new Entry[this->mask+1];
            Entry * e = this->entries;
            for( int i = 0; i <= this->mask; i++, e++ )
            {
                e->id = -1;
            } 
            Entry * oe = old_entries;
            for( int i = 0; i <= old_mask; i++, oe++ )
            {
                if ( oe->id != -1 ) {
                    e = this->set( oe->id, recursion+1 );
                    e->kind = oe->kind;
                    e->u = oe->u;
                }
            }
            delete old_entries;
        }

        //---------------------------------------
        // Hash to proper starting entry, then increment from there until
        // we find existing entry or free entry.
        //---------------------------------------
        this->count++;
        dassert( id >= 0 );
        int i = id & this->mask;
        Entry * e = &this->entries[i];
        for( int j = 0; j <= this->mask; j++ )
        {
            if( e->id == id || e->id == -1 ) {
                dprintf( "set() found entry i=%d id=%d e->id=%d recursion=%d\n", i, id, e->id, recursion );
                e->id = id;
                return e;
            } 

            if( i == this->mask ) {
                i = 0;
                e = &this->entries[0];
            } else {
                i++;
                e++;
            }
        }
        dassert( 0 && "Should have found a free entry" );
        return 0;
    }
};

//---------------------------------------
// Hash Constructor
//---------------------------------------
const int INIT_ENTRY_CNT = 4;

Hash::Hash( void )
{
    impl = new Impl;
    impl->count = 0;
    impl->mask = INIT_ENTRY_CNT-1;
    impl->entries = new Entry[ INIT_ENTRY_CNT ];
    Entry * e = impl->entries;
    for( int id = 0; id < INIT_ENTRY_CNT; id++, e++ )
    {
        e->id = -1;   // indicates unused
    }
}

//---------------------------------------
// Hash Destructor
//---------------------------------------
Hash::~Hash( void )
{
    //---------------------------------------
    // No reference counts, just delete the apparatus.
    //---------------------------------------
    delete impl->entries;
    impl->entries = NULL;
    impl = NULL;
}

//---------------------------------------
// Hash Property Exists?
//---------------------------------------
bool Hash::exists( int id )
{
    return impl->get( id ) != NULL;
}

//---------------------------------------
// Hash Property Defined?
//---------------------------------------
bool Hash::defined( int id )
{
    Entry * e = impl->get( id );
    return e != NULL && e->kind != UNDEF;
}

//---------------------------------------
// Hash Property Kind
//---------------------------------------
nKind Hash::kind( int id )
{
    Entry * e = impl->get( id );
    return (e == NULL) ? UNDEF : e->kind; 
}

//---------------------------------------
// Hash Property Remove
//---------------------------------------
Hash& Hash::remove( int id )
{
    Entry * e = impl->get( id );
    dassert( e != 0 );
    e->id = -1;
    impl->count--;
    return *this;
}

//---------------------------------------
// Hash Property Setters
//---------------------------------------
Hash& Hash::undef( int id )
{
    Entry * e = impl->set( id );
    e->kind = UNDEF;
    return *this;
}

Hash& Hash::i( int id, nInt v )
{
    Entry * e = impl->set( id );
    e->kind = INT;
    e->u.i = v;
    return *this;
}

Hash& Hash::f( int id, nFlt v )
{
    Entry * e = impl->set( id );
    e->kind = FLT;
    e->u.f = v;
    return *this;
}

Hash& Hash::s( int id, nStr v )
{
    Entry * e = impl->set( id );
    e->kind = STR;
    e->u.s = strdup( v );
    return *this;
}

Hash& Hash::hp( int id, Hash * v )
{
    Entry * e = impl->set( id );
    e->kind = HASH;
    e->u.hp = v;
    return *this;
}

Hash& Hash::lp( int id, List * v )
{
    Entry * e = impl->set( id );
    e->kind = LIST;
    e->u.lp = v;
    return *this;
}

//---------------------------------------
// Hash Property Getters
//---------------------------------------
nInt Hash::i( int id )
{
    Entry * e = impl->get( id, INT );
    return e->u.i;
}

nFlt Hash::f( int id )
{
    Entry * e = impl->get( id );
    dassert( e != NULL );
    if ( e->kind == FLT ) {
        return e->u.f;
    } else {
        dassert( e->kind == INT );       // implicit conversion
        return e->u.i;
    }
}

nStr Hash::s( int id )
{
    Entry * e = impl->get( id, STR );
    return e->u.s;
}

Hash& Hash::h( int id )
{
    Entry * e = impl->get( id, HASH );
    return *e->u.hp;
}

Hash * Hash::hp( int id )
{
    Entry * e = impl->get( id, HASH );
    return e->u.hp;
}

List& Hash::l( int id )
{
    Entry * e = impl->get( id, LIST );
    return *e->u.lp;
}

List * Hash::lp( int id )
{
    Entry * e = impl->get( id, LIST );
    return e->u.lp;
}

//---------------------------------------
// Iteration
//---------------------------------------
int Hash::id_first( int& hdl )
{
    hdl = 0;
    return id_next( hdl );
}

int Hash::id_next( int& hdl )
{
    Entry * e = &impl->entries[hdl];
    for( int i = hdl; i <= impl->mask; i++, e++ )
    {
        if ( e->id != -1 ) {
            hdl = i+1;
            return e->id;
        }
    }

    return -1;
}

Hash& Hash::print( const char * s )
{
    int hdl;
    printf( "%s\n", s );
    for( int id = this->id_first( hdl ); id >= 0; id = this->id_next( hdl ) ) 
    {
        if ( id_str_map.count( id ) ) {
            printf( "    %s => ", id_str_map[id] );
        } else {
            printf( "    %d => ", id );
        }

        switch( this->kind( id ) )
        {
            case UNDEF: 
                printf( "undef" ); 
                break;

            case INT:
                printf( "%ld", this->i( id ) );
                break;

            case FLT:
                printf( "%f", this->f( id ) );
                break;

            case STR:
                printf( "%s", this->s( id ) );
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
