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

/* <<-- CHANGE START (lexer) -->> */
%option outfile="lex.yy.c"
%option prefix="graph_yy"
%option noyywrap
%option yylineno

%{
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

using namespace std;

#include "graph-parser.hh"
#include "config.h"

extern int graph_yyerror(char const *msg);

stringstream _literal, _iri;
string _lit;
%}

whitespace		[ \n\r\t]+
iri 			<[^<>]*>
newline			[\n\r]
identifier    [a-zA-Z][a-zA-Z0-9]*
alphanum    [a-zA-Z0-9]+

%x COMMENT STRING LITERAL SPECIAL LIRI

%%

#					{ BEGIN(COMMENT); }

<COMMENT>{newline}	{ BEGIN(INITIAL); }
<COMMENT>.			{ /* ignored */ }

;           { return SEMICOLON; }
:           { return COLON; }

\" {
	BEGIN(STRING); 
	_literal << '\"';
}

<STRING>\"		{
  BEGIN(LITERAL); 
  _literal << '\"'; 
}
<STRING>\\		{ BEGIN(SPECIAL); _literal << '\\'; }
<SPECIAL>.    { BEGIN(STRING);  _literal << graph_yytext; }

<STRING>.     { _literal << graph_yytext; }

<LITERAL>\. {
  _literal >> _lit;
  _literal.str("");
  _literal.clear();

  graph_yylval.str = strdup(_lit.c_str()); 
  _lit = "";
  
  BEGIN(INITIAL);
  unput('.'); 
  return XLITERAL; 
}
<LITERAL>[ \t]+ { 
  graph_yylval.str = strdup(_literal.str().c_str()); 
  _literal.str(""); 
  BEGIN(INITIAL); 
  return XLITERAL;
}
<LITERAL>"<"  { 
  BEGIN(LIRI); 
  _literal << '<';
}
<LITERAL>.		{
  _literal << graph_yytext;
}

<LIRI>">"   { BEGIN(LITERAL); _literal << '>'; }
<LIRI>.     { _literal << graph_yytext; }

\.					{ return TRIPLE_END; }

{iri}				{ graph_yylval.str = strdup(graph_yytext); return IRI; }

{whitespace}		{ /* do nothing */ }

.					{ graph_yyerror("lexical error"); }

%%

int graph_yyerror(char const *msg) {
    std::cerr << PACKAGE << ": " << yylineno
              << ": ERROR near '" << yytext
              << "': " << msg << std::endl;
    std::cerr << _literal.str() << std::endl << _iri.str() << std::endl;
    //exit(EXIT_FAILURE);

    _literal.str("");

    return 1;
}
/* <<-- CHANGE END -->> */
