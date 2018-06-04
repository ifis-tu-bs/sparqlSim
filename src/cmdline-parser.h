#include <map>
#include <string>

#include "cmdline-parser.hh"

using namespace std;

typedef enum { IMPORT, LOAD, STORE, NOTHING, QUERY, QUIT } CmdReturn;

// map<string, string> commands_help {
// 	{ "database", "loads file contents into working database object" },
// 	{ "help", "prints basic help strings for every command" },
// 	{ "import", "loads file contents into working database object" },
// 	{ "load", "loads file contents into working database object" },
// 	{ "pattern", "expects pattern file and computes simulation on current database object" }
// };

// void printHelp(const string &cmd = "help") {

// }
