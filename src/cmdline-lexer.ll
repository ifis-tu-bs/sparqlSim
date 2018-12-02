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
%option prefix="_yy"
%option noyywrap
%option yylineno
%option nodefault

%{
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

using namespace std;

#include "cmdline-parser.hh"
#include "config.h"

extern std::stringstream _literal;

extern int _yyerror(char const *msg);
%}

whitespace		[\n\r\t ]+
iri 					<[^<>]*>
ident					[_a-zA-Z0-9]+
newline				[\n\r]
number			[1-9][0-9]*

%x COMMENT LITERAL SPECIAL SUFFIX

%%

"#"						{ BEGIN(COMMENT); }
<COMMENT>{newline}	{ BEGIN(INITIAL); }
<COMMENT>.				{ /* nothing happens, everything ignored */ }

\" { 
	BEGIN(LITERAL); 
	_literal << _yytext;
}
<LITERAL>\\				{ BEGIN(SPECIAL); _literal << _yytext; }
<SPECIAL>.				{ BEGIN(LITERAL); _literal << _yytext; }
<LITERAL>\"			{ BEGIN(SUFFIX); _literal << _yytext; }
<SUFFIX>{iri}			{ _literal << _yytext; }
<SUFFIX>"."			{ BEGIN(INITIAL);
							_yylval.str = strdup(_literal.str().c_str());
							_literal.str("");
							unput('.');
						  	return XLITERAL;
						}
<SUFFIX>[ \t]			{ BEGIN(INITIAL);
						  _yylval.str = strdup(_literal.str().c_str());
						  return XLITERAL;
						}
<LITERAL>.				{ _literal << _yytext; }
<SUFFIX>.				{ _literal << _yytext; }

PREFIX|prefix|@prefix|@PREFIX {
	return KEY_PREFIX; 
}
SELECT|select { 
	return KEY_SELECT; 
}
ASK|ask {
	return KEY_ASK;
}
WHERE { 
	return KEY_WHERE; 
}

AND 					{ return OP_JOIN; }
UNION					{ return OP_UNION; }
OPTIONAL			{ return OP_OPTIONAL; }

:							{ return COLON; }
;							{ return SEMICOLON; }
"."						{ return TRIPLE_END; }
"*"						{ return ASTERISK; }

"{"						{ return LPAR; }
"}"						{ return RPAR; }

"HELP"|"help"	{ return KEY_HELP; }
"LOAD"|"load"	{ return KEY_LOAD; }
"IMPORT"|"import"	{ return KEY_IMPORT; }
"DATABASE"|"database" { return KEY_DATABASE; }
"PATTERN"|"pattern" { return KEY_PATTERN; }
"STORE"|"store" { return KEY_STORE; }
"REPORT"|"report"	{return KEY_REPORT; }
"VERBOSE"|"verbose"	{return KEY_VERBOSE; }
"QUIT"|"quit"|"EXIT"|"exit"	{ return KEY_QUIT; }
"QUERY"|"query"	{ return KEY_QUERY; }

"CHECK"|"check" { return KEY_CHECK; }

"SET"|"set"		{ return KEY_SET; }
"ON"|"on" { return KEY_ON; }
"OFF"|"off" { return KEY_OFF; }

"SETTINGS"|"settings"	{ return KEY_SETTINGS; }
"ITERATIONS"|"iterations" { return KEY_ITERATIONS; }
"PRUNING"|"pruning" { return KEY_PRUNING; }
"PROFILE"|"profile" { return KEY_PROFILE; }
"VIRTUOSO"|"virtuoso" { return KEY_VIRTUOSO; }
"CSV"|"csv" { return KEY_CSV; }
"EVAL"|"eval" { return KEY_EVAL; }

"?"{ident}      { _yylval.str = strdup(_yytext); return VARIDENT; }
{number}		{ _yylval.num = stoi(_yytext); return NUMBER; }
{ident}				{ _yylval.str = strdup(_yytext); return IDENT; }
{iri}					{ _yylval.str = strdup(_yytext); return IRI; }
{whitespace}	{ /* do nothing */ }

.							{ _yyerror("lexical error"); }

%%

int _yyerror(char const *msg) {
    std::cerr << PACKAGE << ": " << yylineno
              << ": ERROR near '" << yytext
              << "': " << msg << std::endl;
    //exit(EXIT_FAILURE);
    return 1;
}
/* <<-- CHANGE END -->> */
