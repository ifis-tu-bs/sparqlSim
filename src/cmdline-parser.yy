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

#include "cmdline-parser.h"

#include "cmdline.h"
#include "graph.h"
#include "simulations.h"
#include "utils.h"
#include "reporter.h"

/// output stream - set in main.cc
// extern std::ostream* myOut;

/// from flex
extern char* _yytext;
extern int _yylex();
extern int _yyerror(char const *msg);

extern std::string out;
extern std::stringstream errmsg;


/// variables
extern std::string filename;
extern Graph *DB;
// extern Graph *Q, *DB;
extern QGSimulation *Q;

std::stringstream tmp;

std::map<std::string, std::string> prefixes;
std::vector<std::string> files;

extern gengetopt_args_info args_info;
extern unsigned MOD;

extern void loadDBFile(const string &filename);
extern Reporter Karla;
CmdReturn cret;

%}

%name-prefix "_yy"
%error-verbose
%token-table
%defines

%token KEY_PREFIX KEY_SELECT KEY_ASK KEY_WHERE
%token OP_JOIN OP_UNION OP_OPTIONAL
%token COLON SEMICOLON TRIPLE_END VARIABLE_BEGIN ASTERISK
%token LPAR RPAR IRI_BEGIN IRI_END IRI
%token NUMBER IDENT VARIDENT WORDS XLITERAL

%token KEY_HELP KEY_LOAD KEY_IMPORT KEY_DATABASE KEY_PATTERN
%token KEY_STORE KEY_REPORT KEY_QUIT KEY_QUERY KEY_CHECK
%token KEY_SET KEY_SETTINGS KEY_ITERATIONS KEY_PRUNING
%token KEY_CSV KEY_PROFILE KEY_VIRTUOSO KEY_VERBOSE
%token KEY_EVAL
%token KEY_ON KEY_OFF

%union {
  char *str;
  int num;
}

%type <str> IDENT
%type <str> VARIDENT
%type <str> WORDS
%type <str> IRI
%type <str> variable
%type <str> prefix
%type <num> NUMBER
%type <str> subject
%type <str> predicate
%type <str> object
%type <str> XLITERAL

%left OP_OPTIONAL

%start start
%%


start:
  prefix_decl
  query
  { cret = QUERY; }
| commands
;

commands:
  help { cret = NOTHING; }
| db_load { cret = LOAD; }
| db_import { cret = IMPORT; }
| db_store { cret = STORE; }
| settings { cret = NOTHING; }
| quit { cret = QUIT; }
| report { cret = NOTHING; }
| query_load { cret = NOTHING; }
;

settings:
  KEY_SET KEY_ITERATIONS NUMBER
  { args_info.iterations_arg = $3; printSettings(); }
| KEY_SET KEY_CSV KEY_ON
  { args_info.csv_flag = true; printSettings(); }
| KEY_SET KEY_CSV KEY_OFF
  { args_info.csv_flag = false; printSettings(); }
| KEY_SET KEY_PROFILE KEY_ON
  { args_info.profile_flag = true; printSettings(); }
| KEY_SET KEY_PROFILE KEY_OFF
  { args_info.profile_flag = false; printSettings(); }
| KEY_SET KEY_VERBOSE KEY_ON
  { args_info.verbose_flag = true; printSettings(); }
| KEY_SET KEY_VERBOSE KEY_OFF
  { args_info.verbose_flag = false; printSettings(); }
| KEY_SETTINGS
  {
    printSettings();
  }
;

report:
  KEY_REPORT { Karla.report(); }
;

help:
  KEY_HELP { printInteractiveHelp(); }
| KEY_HELP command
;

db_import:
  KEY_IMPORT { files.clear(); } files
;

db_load:
  load { files.clear(); } files
;

load:
  KEY_LOAD | KEY_DATABASE
;

db_store:
  KEY_STORE { files.clear(); } IDENT
  { files.push_back($3); }

query_load:
  KEY_QUERY { files.clear(); } files
;

files:
  IDENT files
  { files.push_back($1); }
| /* empty */
;

quit:
  KEY_QUIT
;

command:
  KEY_LOAD    | KEY_IMPORT | KEY_DATABASE |
  KEY_PATTERN | KEY_SET    | KEY_REPORT   |
  KEY_QUERY
;

prefix_decl:
  prefix_decl dotopt KEY_PREFIX IDENT COLON IRI
  {
    //cout << "mapping " << $4 << " to " << $6 << endl;
    prefixes[$4] = $6;
  }
| /* empty production */
;

prefix:
  IDENT COLON
  { //cout << "query for " << $1 << ": " << prefixes[$1] << endl;
  $$ = strdup(prefixes[$1].c_str()); }
;

dotopt:
  TRIPLE_END
| /* empty */
;

query:
  KEY_SELECT
  {
    // dependent on args_info.eval
    // cout << "MOD: " << MOD << " .. ";
    switch (args_info.eval_arg[MOD]) {
      case eval_arg_STRONG: {
        // cout << "strong" << endl;
#ifdef MULTITHREADING
        Q = new StrongSimulation(*DB);
#endif /* MULTITHREADING */
        break;
      }
      case eval_arg_CSTRONG: {
        // cout << "centered" << endl;
#ifdef MULTITHREADING
        Q = new CenteredStrongSimulation(*DB);
#endif /* MULTITHREADING */
        break;
      }
      case eval_arg_DUAL: {
        // cout << "dual" << endl;
#ifdef MULTITHREADING
        Q = new AllDS(*DB);
#endif /* MULTITHREADING */
        break;
      }
      case eval_arg_HHK: {
        Q = new HHK(*DB, !args_info.no_count_flag);
        break;
      }
      case eval_arg_MAETAL: {
        Q = new MaEtAl(*DB);
        break;
      }
      case eval_arg_MINDUAL:
      case eval_arg_PRUNE:
      default: {
        // cout << "prune" << endl;
        Q = new QGSimulation(*DB);
        break;
      }
    }
    Q->push();
  }
  projection KEY_WHERE LPAR pattern RPAR
  {
    Q->pop();
  }
| KEY_ASK
  {
    Q = new QGSimulation(*DB);
    Q->push();
  }
  KEY_WHERE LPAR pattern RPAR
  {
    Q->pop();
  }
;

projection:
  ASTERISK
| VARIDENT varlist
  {
    Q->toProject($1);
  }
;

varlist:
  /* empty */
| VARIDENT varlist
  {
    Q->toProject($1);
  }
;

pattern:
  triple triples
| LPAR { Q->push(); } pattern RPAR { Q->pop(); }
| pattern OP_OPTIONAL
  { if (!args_info.bgp_flag) Q->push(true);
      else Q->push(); }
  LPAR pattern RPAR
  { Q->pop(); }
| pattern join
  { Q->push(); }
  LPAR pattern RPAR
  { Q->pop(); }
//| pattern OP_UNION
//  { Q->close(); Q->push(); Q->Union(); }
//  LPAR pattern RPAR
//  { Q->pop(); }
;

join:
  OP_JOIN
| /* empty */
;

triples:
  triple triples
| /* empty */
;

triple:
  subject predicate object t_end
  {
    if (!DB->isLetter($2)) {
      Q->setEmpty(true);
      tmp.str("");
      tmp << "runtime error: label '" << $2 << "' is not in DB";
      return _yyerror(tmp.str().c_str());
    } else {
      Q->addTriple($1, $2, $3);
    }
  }
;

t_end:
  TRIPLE_END
| /* empty */
;

subject:
  variable
  { $$ = $1; }
| IRI
  { $$ = $1; }
| prefix IDENT
  {
    string pred = $1;
    pred.insert(pred.size()-1,$2);
    $$ = strdup(pred.c_str());
  }
;

predicate:
  prefix IDENT
  {
    string pred = $1;
    pred.insert(pred.size()-1,$2);
    $$ = strdup(pred.c_str());
  }
| IRI
  { $$ = $1; }
| variable
  { $$ = $1; }
;

object:
  subject
  { $$ = $1; }
| XLITERAL
  { $$ = $1; }
;

variable:
  VARIDENT
  { $$ = $1; }
// | prefix IDENT
//   {
//     cout << "here" << endl;
//     string iri = $1;
//     iri.insert(iri.size()-1, $2);
//     $$ = strdup(iri.c_str());
//   }
;

/* <<-- CHANGE END -->> */
