#ifndef HHK_H
#define HHK_H

#include "simulation.h"

#include <map>
#include <vector>

// multi-threading
#include <functional>
#include <thread>
#include <chrono>

using namespace std;

class HHK : public QGSimulation {
private:
  vector<map<SMatrix *, bm::bvector<> > > _remove;
  map<SMatrix *, SMatrix *> _trans;

  unsigned _nextV;
  SMatrix *_nextM = NULL;

  unsigned _iter = 0;

  void initRemoves();
  const bool get_next();

  bool _wc;
  vector<map<SMatrix *, unsigned*> > _count;
  void initCounts();

  vector<map<SMatrix *, bm::bvector<> > > _removed;

public:
	HHK(Graph &db, const bool withCount = true);
	~HHK();

	unsigned evaluate(std::ostream &os = devnull);

  void csv(const std::string &filename, const char delim = ',');
  void csv(std::ostream &os, const char delim = ',');

private:
	unsigned int doHHK();

  thread tout;
  bool _timeOut = false;
  bool _finish = false;

  static void timeOut(const unsigned t, bool *toRef);

}; /* class HHK */

#endif /* HHK_H */
