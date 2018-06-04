#include <chrono>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace std::chrono;

class Reporter {

public:
	Reporter() {}
	~Reporter() {}

	void start(const string &callee, const string &note = "") {
		if (_report.find(callee) == _report.end()) {
			_report[callee] = vector<std::pair<string, double> >();
			_rstart[callee] = map<string, high_resolution_clock::time_point>();
			_rend[callee] = map<string, high_resolution_clock::time_point>();
		}

		// _report[callee][note] = 0.0;
		_rstart[callee][note] = high_resolution_clock::now();
	}

	void end(const string &callee, const string &note = "") {
		_rend[callee][note] = high_resolution_clock::now();
		if (_rstart.find(callee) != _rstart.end()
			&& _rstart[callee].find(note) != _rstart[callee].end()) {
			duration<double> time_span =
					duration_cast<duration<double> >(
						_rend[callee][note]-_rstart[callee][note]);
			_report[callee].push_back(std::pair<string,double>(note, time_span.count()));
		}
		_order.push_back(callee);
	}

	void note(const string &callee, const string &note) {
		_report[callee].push_back(std::pair<string,double>(note, -1.0));
	}

	const unsigned int size() const { return _report.size(); }

	void shout(const string &callee) {
		_os 	<< "# Report: " << callee;
		// if (_report[callee].first >= 0.0)
		// 	_os << " [runtime: " << _report[callee][note] << " s]";
		// if (note.size() > 0)
		// 	_os << " (" << note << ")";
		// _os << endl;
	}

	string get(const string &callee, const string &note = "") {
		stringstream result;
		if (_report[callee].empty())
			return "";
		result << callee << "," << _report[callee].back().second << "," << _report[callee].back().first << endl;
		return result.str();
	}

	string getPretty(const string &callee, const string &note = "") {
		stringstream result;
		result << callee << "     " << _report[callee].back().second << " [" << note << "]" << endl;
		return result.str();
	}

	double getDoubleValue(const string &callee, const string &note = "") {
		return _report[callee].back().second;
	}

	void report() {
		_os << endl << "## Runtime Report ##" << endl;
		for (auto & i : _report) {
			for (auto & j : i.second) {
				_os 	<< "# Report: " << i.first;
				if (j.second >= 0.0)
					_os << " [runtime: " << j.second << " s]";
				if (j.first.size())
					_os << " (" << j.first << ")";
				_os << endl;
			}
		}
		_os << "####################" << endl;
	}


	Reporter & operator<<(const string & s) {
		if (_verbose) {
			_os << "# " << s;
		}
		return *this;
	}

	Reporter & operator<<(const int & s) {
		if (_verbose) {
			_os << "# " << s;
		}
		return *this;
	}

	void operator()(const bool verbose) {
		_verbose = verbose;
	}

private:
	map<string, vector<std::pair<string, double> > > _report;
	map<string, map<string, high_resolution_clock::time_point> > _rstart;
	map<string, map<string, high_resolution_clock::time_point> > _rend;

	vector<string> _order;

	unsigned int _counter = 0;

	ostream &_os = cout;
	bool _verbose = false;

};