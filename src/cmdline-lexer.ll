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
%option nounput

%{
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

using namespace std;

#include "cmdline-parser.hh"
#include "config.h"

extern int _yyerror(char const *msg);
%}

whitespace		[\n\r\t ]+
iri 					<[^<>]*>
ident					[^{}?*:<>\n\r\t ]+
newline				[\n\r]
number			[1-9][0-9]*

%x COMMENT

%%

"#"						{ BEGIN(COMMENT); }
<COMMENT>{newline}	{ BEGIN(INITIAL); }
<COMMENT>.				{ /* nothing happens, everything ignored */ }

PREFIX|prefix|@prefix|@PREFIX      	{ return KEY_PREFIX; }
SELECT|select				{ return KEY_SELECT; }
WHERE					{ return KEY_WHERE; }

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

"SET"|"set"		{ return KEY_SET; }
"SETTINGS"|"settings"	{ return KEY_SETTINGS; }
"ITERATIONS"|"iterations" { return KEY_ITERATIONS; }
"PRUNING"|"pruning" { return KEY_PRUNING; }
"PROFILE"|"profile" { return KEY_PROFILE; }
"VIRTUOSO"|"virtuoso" { return KEY_VIRTUOSO; }
"CSV"|"csv" { return KEY_CSV; }
"ON"|"on" { return KEY_ON; }
"OFF"|"off" { return KEY_OFF; }

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
