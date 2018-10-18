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

#include "Misc.h"
#include "Node.h"
#include "zlib.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "math.h"
#include "assert.h"

#undef dprintf
#define dprintf if ( 0 ) printf

//--------------------------------------------
// Internal Implementation Structure
//--------------------------------------------
const int LINE_LEN = 1024;

enum 
{
    TOK_NONE    = 0,
    TOK_EOF     = 1,
    TOK_ID      = 2,
    TOK_STR     = 3,
    TOK_INT     = 4,
    TOK_FLT     = 5,
    TOK_LSQUARE = 6,
    TOK_RSQUARE = 7,
    TOK_LCURLY  = 8,
    TOK_RCURLY  = 9,
    TOK_LPAREN  = 10,
    TOK_RPAREN  = 11,
    TOK_COMMA   = 12,
    TOK_COLON   = 13
};

class NodeIO::Impl
{
public:
    //------------------------------------------------------------
    // NodeIO Info
    //------------------------------------------------------------
    gzFile              file_hdl;                                                       // open file hdl
    char                line[LINE_LEN];                                                 // current line being parsed
    int                 line_pos;                                                       // position in current line
    int                 token;                                                          // current token kind, if any
    char                token_str[LINE_LEN];                                            // current token string, if relevant
    nInt                token_int;                                                      // when token is an int
    nFlt                token_flt;                                                      // when token is a flt

    List *              list_parse();                                                   // parse list
    Hash *              hash_parse();                                                   // parse hash
    int                 token_peek();                                                   // parse one token
    bool                token_peek_eq( int tok );                                       // return true if next token is this
    void                token_expect( int tok );                                        // dassertion and consumption of token

    bool                getline( void );                                                // get one line
};

//----------------------------------------------------------------
// Initialization
//----------------------------------------------------------------
NodeIO::NodeIO( const char * file_path )
{
    impl = new NodeIO::Impl();

    impl->file_hdl = gzopen( file_path, "r" );
    if ( !impl->file_hdl ) {
         char msg[256];
         sprintf( msg, "could not open file %s for reading, errno=%d", file_path, errno );
         error( msg );
    }

    impl->line[0] = '\0';
    impl->line_pos = 0;
    impl->token = TOK_NONE;
}

//----------------------------------------------------------------
// Destructor
//----------------------------------------------------------------
NodeIO::~NodeIO()
{
    gzclose( impl->file_hdl );
    delete impl;
    this->impl = nullptr;
}

//----------------------------------------------------------------
// Parses an entire list.
//----------------------------------------------------------------
List * NodeIO::list_parse()
{
    return impl->list_parse();
}

List * NodeIO::Impl::list_parse( void )
{
    dprintf( "begin list_parse()\n" );
    List * list = new List();
    this->token_expect( TOK_LSQUARE );
    for( ;; )
    {
        dprintf( "tok=%d\n", token_peek() );
        if ( this->token_peek_eq( TOK_LCURLY ) ) {
            list->pushhp( this->hash_parse() );
        } else if ( this->token_peek_eq( TOK_LSQUARE ) ) {
            list->pushlp( this->list_parse() );
        } else if ( this->token_peek_eq( TOK_ID ) ) {
            list->pushs( this->token_str );
            this->token_expect( TOK_ID );
        } else if ( this->token_peek_eq( TOK_STR ) ) {
            list->pushs( this->token_str );
            this->token_expect( TOK_STR );
        } else if ( this->token_peek_eq( TOK_INT ) ) {
            list->pushi( this->token_int );
            this->token_expect( TOK_INT );
        } else if ( this->token_peek_eq( TOK_FLT ) ) {
            list->pushf( this->token_flt );
            this->token_expect( TOK_FLT );
        }
        if ( this->token_peek_eq( TOK_COMMA ) ) {
            this->token_expect( TOK_COMMA );
        } else {
            break;
        }
    }
    this->token_expect( TOK_RSQUARE );
    dprintf( "end list_parse()\n" );
    return list;
}

//----------------------------------------------------------------
// Parses an entire hash.
//----------------------------------------------------------------
Hash * NodeIO::hash_parse()
{
    return impl->hash_parse();
}

Hash * NodeIO::Impl::hash_parse( void )
{
    dprintf( "begin hash_parse()\n" );
    Hash * hash = new Hash();
    this->token_expect( TOK_LCURLY );
    for( ;; )
    {
        if ( this->token_peek_eq( TOK_ID ) ) {
            int name_id = Hash::str_to_id( this->token_str );
            this->token_expect( TOK_ID );
            this->token_expect( TOK_COLON );
            if ( this->token_peek_eq( TOK_LCURLY ) ) {
                hash->hp( name_id, this->hash_parse() );
            } else if ( this->token_peek_eq( TOK_LSQUARE ) ) {
                hash->lp( name_id, this->list_parse() );
            } else if ( this->token_peek_eq( TOK_ID ) ) {
                hash->s( name_id, this->token_str );
                this->token_expect( TOK_ID );
            } else if ( this->token_peek_eq( TOK_STR ) ) {
                hash->s( name_id, this->token_str );
                this->token_expect( TOK_STR );
            } else if ( this->token_peek_eq( TOK_INT ) ) {
                hash->i( name_id, this->token_int );
                this->token_expect( TOK_INT );
            } else if ( this->token_peek_eq( TOK_FLT ) ) {
                hash->f( name_id, this->token_flt );
                this->token_expect( TOK_FLT );
            } else {
                char msg[256];
                sprintf( msg, "hash field expression is unexpected: %s", this->line );
                error( msg );
            }
        }
        if ( this->token_peek_eq( TOK_COMMA ) ) {
            this->token_expect( TOK_COMMA );
        } else {
            break;
        }
    }
    token_expect( TOK_RCURLY );
    dprintf( "end hash_parse()\n" );
    return hash;
}

//----------------------------------------------------------------
// Parses one token.
//----------------------------------------------------------------
int NodeIO::Impl::token_peek( void )
{
    if ( this->token != TOK_NONE ) return this->token;

    for( ;; )
    {
        while( this->line[this->line_pos] == '\0' )
        {
            if ( !getline() ) {
                this->token = TOK_EOF;
                return this->token;
            }
        }

        char ch0 = this->line[this->line_pos];
        switch( ch0 ) 
        {
            case '#':   
                this->line[this->line_pos] = '\0';
                break;

            case ' ':
            case '\t':
                this->line_pos++;
                break;

            case '[':   
                this->token = TOK_LSQUARE;
                this->line_pos++;
                return this->token;

            case ']':  
                this->token = TOK_RSQUARE;
                this->line_pos++;
                return this->token;

            case '{':   
                this->token = TOK_LCURLY;
                this->line_pos++;
                return this->token;

            case '}':  
                this->token = TOK_RCURLY;
                this->line_pos++;
                return this->token;

            case '(':   
                this->token = TOK_LPAREN;
                this->line_pos++;
                return this->token;

            case ')':  
                this->token = TOK_RPAREN;
                this->line_pos++;
                return this->token;

            case ':': 
                this->token = TOK_COLON;
                this->line_pos++;
                return this->token;

            case ',':
                this->token = TOK_COMMA;
                this->line_pos++;
                return this->token;

            case '"':
                this->token = TOK_STR;
                this->line_pos++;
                for( int j = 0; ; j++ )
                {
                    char ch = this->line[this->line_pos++];
                    if ( ch == '\0' ) {
                        char msg[256];
                        sprintf( msg, "string literal may not span a line: %s", this->line ); 
                        error( msg );
                    } 

                    if ( ch == '"' ) {
                        // done
                        this->token_str[j] = '\0';
                        return this->token;
                    }

                    if ( ch == '\\' ) {
                        // escape next character
                        ch = this->line[this->line_pos];
                        dassert( ch != '\0' );
                    }
                    this->token_str[j] = ch;
                }
                break;

            default:
                if ( (ch0 >= 'a' && ch0 <= 'z') ||
                     (ch0 >= 'A' && ch0 <= 'Z') ||
                     ch0 == '_' ) {
                    this->token = TOK_ID;
                    int j;
                    for( j = 0; ; j++, this->line_pos++ )
                    {
                        char ch = this->line[this->line_pos];
                        if ( (ch >= '0' && ch <= '9') ||
                             (ch >= 'a' && ch <= 'z') ||
                             (ch >= 'A' && ch <= 'Z') ||
                             ch == '_' ) {
                            this->token_str[j] = ch;
                        } else {
                            break;
                        }
                    }
                    this->token_str[j] = '\0';
                    return this->token;
                } else if ( ch0 == '+' || ch0 == '-' || (ch0 >= '0' && ch0 <= '9') ) {
                    int sign = 0;
                    if ( ch0 == '+' || ch0 == '-' ) {
                        if ( ch0 == '-' ) sign = 1;        
                        this->line_pos++;
                    }

                    bool got_one = false;
                    this->token_int = 0;
                    for( ;; )
                    {
                        char ch = this->line[this->line_pos];
                        if ( ch >= '0' && ch <= '9' ) {
                            this->token_int *= 10;
                            this->token_int += (ch - '0');
                            this->line_pos++;
                            got_one = true;
                        } else {
                            dassert( got_one );
                            break;
                        }
                    }
                    if ( this->line[this->line_pos] != '.' ) {
                        this->token = TOK_INT;
                        if ( sign ) this->token_int = -this->token_int;
                    } else {
                        this->line_pos++;
                        this->token = TOK_FLT;
                        this->token_flt = this->token_int;
                        this->token_int = 0;
                        int j;
                        for( j = 0; ; j++ )
                        {
                            char ch = this->line[this->line_pos];
                            if ( ch >= '0' && ch <= '9' ) {
                                this->token_int *= 10;
                                this->token_int += (ch - '0');
                                this->line_pos++;
                            } else {
                                break;
                            }
                        }
                        this->token_flt += nFlt( this->token_int ) / pow( 10.0, j );
                        if ( sign ) this->token_flt = -this->token_flt;
                    }
                    return this->token;
                } else {
                    char msg[256];
                    sprintf( msg, "unexpected char '%c' at index %d: %s", ch0, this->line_pos, this->line ); 
                    error( msg );
                    return TOK_NONE;
                }
                break;
        }
    }

    return TOK_NONE;  // should not get here
}

//----------------------------------------------------------------
// See if next token is equal to some token.
//----------------------------------------------------------------
bool NodeIO::Impl::token_peek_eq( int tok )
{
    return this->token_peek() == tok;
}

//----------------------------------------------------------------
// Expect a token next.
//----------------------------------------------------------------
void NodeIO::Impl::token_expect( int tok )
{
    if ( this->token_peek_eq( tok ) ) {
        this->token = TOK_NONE;
    } else {
        char msg[256];
        sprintf( msg, "expected token %d, got %d: %s", tok, this->token, this->line );
        error( msg );
    }
}
//----------------------------------------------------------------
// Reads line and strips newline.
//----------------------------------------------------------------
bool NodeIO::Impl::getline( void )
{
    this->line[0] = '\0';
    this->line_pos = 0;
    char * ptr = gzgets( this->file_hdl, this->line, LINE_LEN );
    if ( ptr == nullptr ) return false;

    int i = strlen( this->line ) - 1;
    if ( i >= 0 && this->line[i] == '\n' ) {
        this->line[i] = '\0';
    }
    dprintf( "line: %s\n", this->line );
    return true;
}
