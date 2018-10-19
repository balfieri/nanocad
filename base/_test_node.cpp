// Copyright (c) 2014-2016 Robert A. Alfieri
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
#include "List.h"
#include "stdio.h"
#include "assert.h"

int main( int argc, const char * argv[] )
{
    //-------------------------------------------
    // HASH
    //-------------------------------------------
    Hash * h1 = new Hash;

    const int field1 = Hash::str_to_id( "field1" );
    const int field2 = Hash::str_to_id( "field2" );
    const int field3 = Hash::str_to_id( "field3" );

    h1->i( field1, 123 );
    h1->f( field2, 456.223 );
    h1->s( field3, "Hello, world" );
    h1->print( "3 fields" );
    assert( h1->exists( field1 ) );
    assert( h1->defined( field3 ) );

    h1->remove( field1 );
    assert( !h1->exists( field1 ) );
    assert( !h1->defined( field1 ) );
    h1->print( "2 fields" );

    for( int id = 20; id < 100; id++ )
    {
        h1->s( id, "foo" );
    }
    h1->print( "many fields" );

    //-------------------------------------------
    // LIST
    //-------------------------------------------
    List * l1 = new List;
    l1->i( 0, 123 );
    l1->f( 1, 456.223 );
    l1->s( 2, "Hello, world" );
    l1->print( "3 entries" );
    assert( l1->length() == 3 );

    l1->s( 9, "Weird with a beard" );
    l1->print( "10 entries" );
    assert( l1->length() == 10 );
    assert( l1->defined( 9 ) );
    assert( !l1->defined( 4 ) );

    l1->pushi( 1010 );
    l1->print( "11 entries" );

    assert( l1->shifti() == 123 );
    l1->print( "after shift" );

    assert( l1->popi() == 1010 );
    l1->print( "after pop" );

    l1->unshiftf( 223.476 );
    l1->print( "after unshift" );

    return 0;
}
