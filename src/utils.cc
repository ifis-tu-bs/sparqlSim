#include "cmdline.h"
#include "graph.h"
#include "graph.io.h"
#include "label.h"
#include "utils.h"

#include <limits>

#include <iomanip>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <ctime>

#include <map>

using namespace std;

#define BATCH_SIZE 1000

extern gengetopt_args_info args_info;
extern Graph *DB;

extern FILE *graph_yyin;
extern int graph_yyparse();
extern void graph_yyrestart  (FILE * input_file );
extern int graph_yylineno;

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE graph_yy_scan_string(const char * str);
extern void graph_yy_delete_buffer(YY_BUFFER_STATE buffer);

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
    	throw std::runtime_error("popen() failed!");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}

map<string, string> commands_help {
	{ "database", "loads file contents into working database object" },
	{ "help", "prints basic help strings for every command" },
	{ "import", "loads file contents into working database object" },
	{ "load", "loads file contents into working database object" },
	{ "store", "stores the in-memory contents to files" },
	{ "pattern", "expects pattern file and computes simulation on current database object" },
	{ "prefix", "starts a query with prefix declaration"},
	{ "query", "loads a query file and lets the queries run line-wise"},
	{ "select", "starts a query with select statement"},
	{ "settings", "shows the settings currently active"},
	{ "set", "sets settings on/off"}
};

void printInteractiveHelp() {
	int max = 0;
	for (auto &i : commands_help) {
		if (i.first.size() > max)
			max = i.first.size();
	}
	for (auto &i : commands_help) {
		cout << i.first;
		for (int j = i.first.size(); j <= max; ++j)
			cout << " ";
		cout << "  .." << i.second << endl;
	}
}

void printTime() {
	time_t raw;
	struct tm * timeinfo;

	time(&raw);
	timeinfo = localtime(&raw);
	cout << endl << asctime(timeinfo) << endl;
}

void printTime(string &filename) {
	time_t raw;
	struct tm * timeinfo;

	time(&raw);
	timeinfo = localtime(&raw);

	stringstream tmp;
	tmp << "_" << setw(4) << setfill('0') << (1900 + timeinfo->tm_year) 
	    << setw(2) << setfill('0') << (1 + timeinfo->tm_mon) 
	    << setw(2) << setfill('0') << timeinfo->tm_mday 
	    << setw(2) << setfill('0') << timeinfo->tm_hour
	    << setw(2) << setfill('0') << timeinfo->tm_min;
	filename += tmp.str();

	cout << endl << asctime(timeinfo) << endl;
}

void printSettings() {
	cout << endl;
	cout << "output          .. " << (args_info.output_given ? "on" : "off") << " (";
	switch (args_info.output_arg) {
		output_arg_FILE:
			cout << "FILE";
			break;
		output_arg_STDOUT:
			cout << "STDOUT";
			break;
		output_arg_STDERR:
			cout << "STDERR";
			break;
		output_arg_STDLOG:
			cout << "STDLOG";
			break;
		default:
			cout << " ";
			break;
	}
	cout << ")" << endl;
	cout << "csv output      .. " << (args_info.csv_flag ? "on" : "off") << endl << endl
	     << "profile report  .. " << (args_info.profile_flag ? "on" : "off") << endl
	     << "verbose output  .. " << (args_info.verbose_flag ? "on" : "off") << endl << endl
	     << "random order    .. " << (args_info.random_flag ? "on" : "off") << endl
	     << "#iterations     .. ";
    if (args_info.permute_flag)
		cout << "all permutations";
	else
		cout << args_info.iterations_arg;
	cout << endl;
}

char getDelimiter() {
	// static char delim = '';
	// if (delim != '')
	// 	return delim;
	switch (args_info.delim_arg[0]) {
		case 't': return '\t';
		default: return ' ';
	}

	return ' ';
}

void loadDBFile(const string &filename, bool triples) {
	// graph_yylineno = 1;
	bool gzipped = false;
	string fname = filename;
	if (filename.find(".gz")!=string::npos) {
		// stringstream gunzip;
		// gunzip << "gunzip " << filename;
		gzipped = true;
		string cmd = "gunzip " + filename;
		exec(cmd.data());
		fname = filename.substr(0,filename.size()-3);
		// // readGraphFromCompressed(filename, *DB, triples);
		// ifstream dbf;

		// dbf.open(filename, ios_base::in | ios_base::binary);
		// if (!checkStream(filename, dbf)) {
		// 	return;
		// }

		// // adding decompressor capabilities
		// filtering_streambuf<input> in;
		// in.push(gzip_decompressor());
		// in.push(dbf);

		// // cout << "  .. unzipped" << endl;

		// stringstream cont;
		// boost::iostreams::copy(in, cont);

		// // string tmp;
		// // stringstream batch;
		// // while (getline(cont, tmp)) {
		// // 	unsigned int n = 1;
		// // 	do {
		// // 		batch << tmp;
		// // 	} while (++n < BATCH_SIZE && getline(cont, tmp, '\n'));

		// 	char *str = strdup(cont.str().c_str());
		// 	// char *str = strdup(batch.str().c_str());

		// 	YY_BUFFER_STATE b = graph_yy_scan_string(str);

		// 	// while (graph_yyparse() && getline(cont,tmp)) {
		// 	// 	cerr << "round: " << tmp << endl;
		// 	// 	batch << " " << tmp;

		// 	// 	delete str;
		// 	// 	str = strdup(batch.str().c_str());

		// 	// 	graph_yy_delete_buffer(b);
		// 	// 	b = graph_yy_scan_string(str);
		// 	// 	// cerr << str << endl;
		// 	// }

		// 	graph_yyparse();

		// 	// batch.str("");
		// 	graph_yy_delete_buffer(b);
		// 	delete str;
		// // }

		// // cont.close();
		// in.reset();
		// dbf.close();
	}
	// } else {

		// readGraphFromFile(filename, *DB, triples);
		graph_yyin = fopen64(fname.c_str(), "r");
		if (!graph_yyin) {
			cerr << "error opening file '" << fname << "'" << endl;
			return;
		}

		graph_yyrestart(graph_yyin);

		graph_yyparse();
		
		fclose(graph_yyin);
		graph_yyin = NULL;

		if (gzipped) {
			string cmd = "gzip " + fname;
			exec(cmd.data());
		}
	// }
}

// void readGraphFromFile(const string &filename, Graph &g, bool triples) {
// 	static char delim = getDelimiter();
// 	static ifstream dbf;
// 	static string subj, pred, obj, dot;

// 	dbf.open(filename);
// 	if (!checkStream(filename, dbf))
// 		return;

// 	while (dbf >> subj) {
// 		if (subj[0] == '#') {
// 			getline(dbf, dot);
// 			continue;
// 		} else {
// 			dbf >> pred >> obj;
// 			bool f = true;
// 			if (obj[obj.size()-1]=='.') {
// 				int pos = obj.size()-1;
// 				do {
// 					--pos;
// 				} while (obj[pos] == ' ' || obj[pos] == '\t');
// 				obj = obj.substr(0,pos+1);
// 			} else {
// 				do {
// 					dbf >> dot;
// 					if (dot[dot.size()-1] != '.') {
// 						obj += " "+dot;
// 					} else {
// 						f = false;
// 					}
// 				} while (f);				
// 			}
// 		}

// 		if (triples) {
// 			g.addTriple(subj, pred, obj);
// 		} else {
// 			g.addNode(subj);
// 			g.addNode(obj);
// 			g.addLabel(pred);
// 		}
// 	}
// 	dbf.close();
// }

// void readGraphFromCompressed(const string &filename, Graph &g, bool triples) {
// 	char delim = getDelimiter();
// 	string subj, pred, obj, dot;
// 	ifstream dbf;
// 	stringstream cont;

// 	dbf.open(filename, ios_base::in | ios_base::binary);
// 	if (!checkStream(filename, dbf)) {
// 		return;
// 	}

// 	// adding decompressor capabilities
// 	filtering_streambuf<input> in;
// 	in.push(gzip_decompressor());
// 	in.push(dbf);
// 	boost::iostreams::copy(in, cont);

// 	while (cont >> subj) {
// 		cont >> pred;
// 		cont >> obj;
// 		bool f = true;
// 		do {
// 			cont >> dot;
// 			if (dot.rfind('.')== string::npos) {
// 				obj += " " + dot;
// 				// obj += dot;
// 			} else {
// 				f = false;
// 			}
// 		} while (f);
// 		// if (i++ % 1000000 == 0)
// 		// 	cout << subj << " " << pred << " " << obj << endl;
// 		// if (pos == string::npos) {
// 		// 	cout << "error triple(" << num+1 << "): " << subj << " | " << pred << " | " << obj << " | " << dot << endl;
// 		// 	continue;
// 		// 	// continue;
// 		// }
// 		// }

// 		if (triples)
// 			g.addTriple(subj, pred, obj);
// 		else {
// 			g.addNode(subj);
// 			g.addNode(obj);
// 			g.addLabel(pred);
// 		}
// 		// if (n) --n;
// 	}
// 	// Karla.end("Read graph from file");
// 	dbf.close();
// 	cont.str("");
// }

bool checkStream(const string &fname, ifstream &f) {
	if (!f.is_open()) {
		cerr << "could not open '" << fname << "'" << endl;
		return false;
	}
	return true;
}

bool checkStream(const string &fname, ofstream &f) {
	if (!f.is_open()) {
		cerr << "could not open '" << fname << "'" << endl;
		return false;
	}
	return true;
}
