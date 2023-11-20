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


#if !defined(gum_MDPDAT_COCO_PARSER_H__)
#define gum_MDPDAT_COCO_PARSER_H__

#include <agrum/tools/core/cast_unicode.h>
// =====================================================================
#include <agrum/FMDP/IfmdpFactory.h>
// =====================================================================
#undef TRY
#define  TRY(inst) try { inst; } catch (gum::Exception& e) { SemErr(e.errorType());}

#include <iostream>
#include <string>
#include <fstream>
#include "Scanner.h"

namespace gum {
namespace MDPDAT {


class Parser {
  private:
    	enum {
		_EOF=0,
		_operand=1,
		_ident=2,
		_integer=3,
		_number=4,
		_string=5,
		_largestring=6,
		_lpar=7
	};
	int maxT;

    Token* dummyToken;
    int errDist;
    int minErrDist;

    void SynErr( int n );
    void Get();
    void Expect( int n );
    bool StartOf( int s );
    void ExpectWeak( int n, int follow );
    bool WeakSeparator( int n, int syFol, int repFol );

    ErrorsContainer  errors__;

  public:
    Scanner* scanner;

    Token* t;     // last recognized token
    Token* la;      // lookahead token

    gum::AbstractFMDPFactory* factory__;

                /// for each transition diagram, we need to know the associated variable
                std::string currentFunctionGraphVar__;

                /// for building the diagram, we need to keep track of var parents
                std::vector< gum::NodeId > parentNode__;

                /// and current modality
                std::vector< gum::Idx > parentModality__;


        // *************************************************************************************
        // Getters and Setters on those attributes
        // *************************************************************************************

                /// Sets the main factory
                void setFactory( gum::AbstractFMDPFactory* f ) {
                        factory__ = f;
                }

                gum::AbstractFMDPFactory& factory() {
                  if (factory__)
                        return *factory__;
                  GUM_ERROR(gum::OperationNotAllowed,"Please set a factory for scanning BIF file...");
                }

                bool IsFollowedByIdent() {
                        Token* next = scanner->Peek();
                        return la->kind == _lpar && next->kind == _ident;
                }

                void SemErr(std::string s) {
                  SemErr(widen(s).c_str());
                }

                void Warning(std::string s) {
                  Warning(widen("Warning : "+s).c_str());
                }



// ===================================================================================================
// CHARACTERS DEFINITION
// ===================================================================================================

    Parser( Scanner* scanner );
    ~Parser();
    void SemErr( const wchar_t* msg );
    void SynErr( const std::wstring& filename,int line, int col, int n );
    void Warning( const wchar_t* msg );
    const ErrorsContainer& errors() const;
    ErrorsContainer& errors();

    	void MDPDAT();
	void VARIABLES_DECLARATION();
	void ACTION();
	void REWARD_FUNCTION_GRAPH();
	void DISCOUNT();
	void TOLERANCE();
	void VARIABLE();
	void IDENT(std::string& name);
	void MODALITY_LIST();
	void IDENT_OR_INTEGER(std::string& name);
	void STRING(std::string& str);
	void FLOAT(float& val);
	void TRANSITION_FUNCTION_GRAPH();
	void COST_FUNCTION_GRAPH();
	void SUB_TRANSITION_FUNCTION_GRAPH();
	void TRANSITION_LEAF();
	void SUB_FUNCTION_GRAPH();
	void LEAF();
	void OPERAND(std::string& op);

    void Parse();

}; // end Parser

} // namespace
} // namespace


#endif // !defined(COCO_PARSER_H__)

