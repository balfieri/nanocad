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
#ifndef _Node_h
#define _Node_h

// Node.h - Hash and list nodes that are designed to be more efficient than std::map and std::vector.
//          They are particularly suited to implement dynamic data structures found in languages
//          such as Python or Javascript. Our goal here is purely performance, not elegance.
//
#include <string>

typedef long         nInt;      // 64-bit integer 
typedef double       nFlt;      // 64-bit float
typedef const char * nStr;      // immutable character string

//---------------------------------------
// Property Kinds
//---------------------------------------
typedef enum
{
    UNDEF = 0,
    INT   = 1,
    FLT   = 2,
    STR   = 3,
    HASH  = 4,
    LIST  = 5 
} nKind;

class List;

//---------------------------------------
// Hash
//
// This is similar to std::map but is designed
// to be much more efficient.
//---------------------------------------
class Hash 
{
public:
    Hash( void );
    ~Hash();

    static int  str_to_id( nStr s );  // map string to unique property id which is used in all routines below
    static nStr id_to_str( int id );  // map unique property back to string (better exist)

    bool  exists( int id );
    bool  defined( int id );
    nKind kind( int id );

    Hash& remove( int id ); // remove property
    Hash& undef( int id ); // make property undefined but existing
    Hash& i( int id, nInt v );
    Hash& f( int id, nFlt v );
    Hash& s( int id, nStr v );
    Hash& hp( int id, Hash * v );
    Hash& lp( int id, List * v );

    nInt  i( int id );
    nFlt  f( int id );
    nStr  s( int id );
    Hash& h( int id );
    Hash* hp( int id );
    List& l( int id );
    List* lp( int id );

    int   id_first( int& hdl ); // property iteration
    int   id_next( int& hdl );

    Hash& print( nStr s = "" );

private:
    class Impl;
    Impl * impl;
};

//---------------------------------------
// List
//
// This is similar to std::vector but is designed
// to be much more efficient.
//---------------------------------------
class List
{
public:
    List( void );
    ~List();

    int    length( void );
    bool   exists( int i );
    bool   defined( int i );
    nKind  kind( int i );

    List& undef( int i ); 
    List& i( int i, nInt v );
    List& f( int i, nFlt v );
    List& s( int i, nStr v );
    List& hp( int i, Hash * v );
    List& lp( int i, List * v );

    nInt  i( int i );
    nFlt  f( int i );
    nStr  s( int i );
    Hash& h( int i );
    Hash* hp( int i );
    List& l( int i );
    List* lp( int i );

    List& pushi( nInt v );
    List& pushf( nFlt v );
    List& pushs( nStr v );
    List& pushhp( Hash * v );
    List& pushlp( List * v );

    nInt  popi( void );
    nFlt  popf( void );
    nStr  pops( void );
    Hash& poph( void );
    Hash* pophp( void );
    List& popl( void );
    List* poplp( void );

    List& unshifti( nInt v );
    List& unshiftf( nFlt v );
    List& unshifts( nStr v );
    List& unshifthp( Hash * v );
    List& unshiftlp( List * v );

    nInt  shifti( void );
    nFlt  shiftf( void );
    nStr  shifts( void );
    Hash& shifth( void );
    Hash* shifthp( void );
    List& shiftl( void );
    List* shiftlp( void );

    List& print( nStr s = "" );

private:
    class Impl;
    Impl * impl;
};

//---------------------------------------
// STATIC: NodeIO
//---------------------------------------
class NodeIO
{
public:
    NodeIO( const char * file_path );
    ~NodeIO();

    List * list_parse( void );
    Hash * hash_parse( void );

private:
    class Impl;
    Impl * impl;
};

#endif
