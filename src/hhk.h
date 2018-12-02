#ifndef HHK_H
#define HHK_H

#include "simulation.h"

class HHK : public QGSimulation {
public:
	HHK(Graph &db);
	~HHK();

	unsigned evaluate(std::ostream &os = devnull);

private:
	unsigned int doHHK();

}; /* class HHK */

#endif /* HHK_H */
