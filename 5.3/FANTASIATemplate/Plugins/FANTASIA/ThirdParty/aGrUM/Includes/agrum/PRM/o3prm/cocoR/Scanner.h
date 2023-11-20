/***************************************************************************
 *  aGrUM modified frames and atg files for cocoR
 *   Copyright (c) 2005 by Christophe GONZALES(@AMU) and Pierre-Henri WUILLEMIN(@LIP6)  *
 *   info_at_agrum_dot_org
***************************************************************************/
/*----------------------------------------------------------------------
Compiler Generator Coco/R,
Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
extended by M. Loeberbauer & A. Woess, Univ. of Linz
ported to C++ by Csaba Balazs, University of Szeged
with improvements by Pat Terry, Rhodes University

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

As an exception, it is allowed to write an extension of Coco/R that is
used as a plugin in non-free software.

If not otherwise stated, any source code generated by Coco/R (other than
Coco/R itself) does not fall under the GNU General Public License.
-----------------------------------------------------------------------*/


#if !defined(gum_prm_o3prm_COCO_SCANNER_H__)
#define gum_prm_o3prm_COCO_SCANNER_H__

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#include <agrum/tools/core/signal/signaler1.h>
#include <agrum/tools/core/errorsContainer.h>

namespace gum {
namespace prm {
namespace o3prm {


class Token {
  public:
    int kind;     // token kind
    int pos;      // token position in bytes in the source text (starting at 0)
    int charPos;  // token position in characters in the source text (starting at 0)
    int col;      // token column (starting at 1)
    int line;     // token line (starting at 1)
    wchar_t* val; // token value
    Token* next;  // ML 2005-03-11 Peek tokens are kept in linked list

    Token();
    ~Token();
};

class Buffer {
// This Buffer supports the following cases:
// 1) seekable stream (file)
//    a) whole stream in buffer
//    b) part of stream in buffer
// 2) non seekable stream (network, console)
  private:
    unsigned char* buf; // input buffer
    int bufCapacity;    // capacity of buf
    int bufStart;       // position of first byte in buffer relative to input stream
    int bufLen;         // length of buffer
    int fileLen;        // length of input stream (may change if the stream is no file)
    int bufPos;         // current position in buffer
    FILE* stream;       // input stream (seekable)
    bool isUserStream;  // was the stream opened by the user?

    int ReadNextStreamChunk();
    bool CanSeek();     // true if stream can be seeked otherwise false

  public:
    int GetPercent( void );

    static const int EoF = COCO_WCHAR_MAX + 1;

    Buffer( FILE* s, bool isUserStream );
    Buffer( const unsigned char* buf, int len );
    Buffer( Buffer* b );
    virtual ~Buffer();

    virtual void Close();
    virtual int Read();
    virtual int Peek();
    virtual wchar_t* GetString( int beg, int end );
    virtual int GetPos();
    virtual void SetPos( int value );
};

class UTF8Buffer : public Buffer {
  public:
    UTF8Buffer( Buffer* b ) : Buffer( b ) {};
    virtual int Read();
};

//-----------------------------------------------------------------------------------
// StartStates  -- maps characters to start states of tokens
//-----------------------------------------------------------------------------------
class StartStates {
  private:
    class Elem {
      public:
        int key, val;
        Elem* next;
        Elem( int key, int val ) { this->key = key; this->val = val; next = nullptr; }
    };

    Elem** tab;

  public:
    StartStates() { tab = new Elem*[128]; memset( tab, 0, 128 * sizeof( Elem* ) ); }
    virtual ~StartStates() {
      for ( int i = 0; i < 128; ++i ) {
        Elem* e = tab[i];

        while ( e != nullptr ) {
          Elem* next = e->next;
          delete e;
          e = next;
        }
      }

      delete [] tab;
    }

    void set( int key, int val ) {
      Elem* e = new Elem( key, val );
      int k = ( ( unsigned int ) key ) % 128;
      e->next = tab[k]; tab[k] = e;
    }

    int state( int key ) {
      Elem* e = tab[( ( unsigned int ) key ) % 128];

      while ( e != nullptr && e->key != key ) e = e->next;

      return e == nullptr ? 0 : e->val;
    }
};

//-------------------------------------------------------------------------------------------
// KeywordMap  -- maps strings to integers (identifiers to keyword kinds)
//-------------------------------------------------------------------------------------------
class KeywordMap {
  private:
    class Elem {
      public:
        wchar_t* key;
        int val;
        Elem* next;
        Elem( const wchar_t* key, int val ) { this->key = coco_string_create( key ); this->val = val; next = nullptr; }
        virtual ~Elem() { coco_string_delete( key ); }
    };

    Elem** tab;

  public:
    KeywordMap() { tab = new Elem*[128]; memset( tab, 0, 128 * sizeof( Elem* ) ); }
    virtual ~KeywordMap() {
      for ( int i = 0; i < 128; ++i ) {
        Elem* e = tab[i];

        while ( e != nullptr ) {
          Elem* next = e->next;
          delete e;
          e = next;
        }
      }

      delete [] tab;
    }

    void set( const wchar_t* key, int val ) {
      Elem* e = new Elem( key, val );
      int k = coco_string_hash( key ) % 128;
      e->next = tab[k]; tab[k] = e;
    }

    int get( const wchar_t* key, int defaultVal ) {
      Elem* e = tab[coco_string_hash( key ) % 128];

      while ( e != nullptr && !coco_string_equal( e->key, key ) ) e = e->next;

      return e == nullptr ? defaultVal : e->val;
    }
};

class Scanner {
  private:
    void* firstHeap;
    void* heap;
    void* heapTop;
    void** heapEnd;

    std::wstring filenamne__;
    bool trace__;

    unsigned char EOL;
    int eofSym;
    int noSym;
    int maxT;
    // int charSetSize; // not used
    StartStates start;
    KeywordMap keywords;
    int percent;

    Token* t;         // current token
    wchar_t* tval;    // text of current token
    int tvalLength;   // length of text of current token
    int tlen;         // length of current token

    Token* tokens;    // list of tokens already peeked (first token is a dummy)
    Token* pt;        // current peek token

    int ch;           // current input character
    
    int pos;          // byte position of current character
    int charPos;      // position by unicode characters starting with 0
    int line;         // line number of current character
    int col;          // column number of current character
    int oldEols;      // EOLs that appeared in a comment;

    void CreateHeapBlock();
    Token* CreateToken();
    void AppendVal( Token* t );
    void SetScannerBehindT();

    void Init();
    void NextCh();
    void AddCh();
    	bool Comment0();
	bool Comment1();

    Token* NextToken();

  public:
    Signaler1<int> onLoad;

    Buffer* buffer;   // scanner buffer

    Scanner( const unsigned char* buf, int len, std::string filename="anonymous buffer", bool trace=false );
    Scanner( const char* fileName,bool trace=false );
    Scanner( const wchar_t* fileName,bool trace=false );
    Scanner( FILE* s,bool trace=false );
    ~Scanner();

    void setTrace( bool b ) { trace__=b;}

    void Load( const wchar_t* fileName );
    Token* Scan();
    Token* Peek();
    void ResetPeek();

    const std::wstring& filename() const {return filenamne__;}
    Buffer* getBuffer() {return buffer;}

}; // end Scanner

} // namespace
} // namespace
} // namespace


#endif // !defined(COCO_SCANNER_H__)


