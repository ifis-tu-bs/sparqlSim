/*****************************************************************************\
 Hello -- <<-- Hello World -->>

 Copyright (c) <<-- 20XX Author1, Author2, ... -->>

 Hello is free software: you can redistribute it and/or modify it under the
 terms of the GNU Affero General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 Hello is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 more details.

 You should have received a copy of the GNU Affero General Public License
 along with Hello.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

/* <<-- CHANGE START (parser) -->> */
%{
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>


#include "config.h"

#include "cmdline.h"
#include "graph.h"
#include "simulation.h"
#include "utils.h"
#include "reporter.h"

/// output stream - set in main.cc
// extern std::ostream* myOut;

/// from flex
extern char* graph_yytext;
extern int graph_yylex();
extern int graph_yyerror(char const *msg);

extern std::string out;
extern std::stringstream errmsg;


/// variables
extern std::string filename;
extern Graph *DB;

extern gengetopt_args_info args_info;

extern Reporter Karla;

extern stringstream _literal, _iri;

string subj, pred, obj;

unsigned N = 1;

%}

%name-prefix "graph_yy"
%error-verbose
%token-table
%defines

%token IRI XLITERAL SLITERAL TRIPLE_END NEWLINE
%token PREFIX SEMICOLON COLON

%union {
  char *str;
  int num;
}

%type <str> IRI
%type <str> prefix


%start start
%%


start:
  prefix_declaration triples
;

prefix_declaration:
  PREFIX prefix dotopt prefix_declaration
| /* empty */
;

prefix:
  COLON IRI
  {  }
;

dotopt:
  TRIPLE_END
| /* empty */
;

triples:
  triples triple
| /* empty */
;

triple:
  subject predicate object
  {
    //cout << $1 << " " << $2 << " " << _literal.str() << endl;
    if (N++ % 100000000 == 0) {
      cerr << N-1 << " : ";
      cerr << subj << " " << pred << " " << obj << " ." << endl;

    }
    DB->addTriple(subj, pred, obj);
    subj.clear(); pred.clear(); obj.clear();
  }
;

subject:
  IRI { subj = _iri.str(); _iri.str(""); }
;

predicate:
  IRI { pred = _iri.str(); _iri.str(""); }
;

object:
  XLITERAL TRIPLE_END { obj = _literal.str(); _literal.str(""); }
| IRI TRIPLE_END { obj = _iri.str(); _iri.str(""); }
| SLITERAL { obj = _literal.str(); _literal.str(""); }
;

/* <<-- CHANGE END -->> */
