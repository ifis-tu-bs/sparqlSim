#ifndef REPORTER_H
#define REPORTER_H 

#include <chrono>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>

using namespace std;
using namespace std::chrono;

class Reporter {

public:
	/// Constructors & Destructors
	Reporter();
	~Reporter();

	/// Timing Functions
	void start(const string &callee, const string &note = "");
	void end(const string &callee, const string &note = "");

	void note(const string &callee, const string &note, const string &val);

	const unsigned int size() const;

	void shout(const string &callee);

	string get(const string &callee, const string &note = "");
	string getValue(const string &callee, const string &note = "");

	string getPretty(const string &callee, const string &note = "");

	// double getDoubleValue(const string &callee, const string &note = "");

	void report();

	/// Verbose output
	Reporter & operator<<(const string & s);
	Reporter & operator<<(const int & s);
	void endline();

	// Replaces the output stream
	void operator()(std::ostream &os);

	ostream &out();

	/// Progress Bar Capabilities
	void initProgress(const unsigned &max, const unsigned &steps);
	void printProgress(const unsigned val = 0);
	void updateProgress(const unsigned val);
	void updateProgressVal(const unsigned val);
	void finishProgress();

	const unsigned step() const;

private:
	stringstream tmp;
	map<string, vector<std::pair<string, string> > > _report;
	map<string, map<string, high_resolution_clock::time_point> > _rstart;
	map<string, map<string, high_resolution_clock::time_point> > _rend;

	vector<string> _order;

	unsigned int _counter = 0;

	bool _barPrinted = false;
	unsigned _max;
	unsigned short _steps; // granularity of steps (e.g. 4 => every 25%)
	unsigned _step;
	unsigned short _progress;

	ostream *_os;

};

#endif /* REPORTER_H */
