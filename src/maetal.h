#ifndef MaEtAl_H
#define MaEtAl_H

#include "simulation.h"

class MaEtAl : public QGSimulation {
public:
	MaEtAl(Graph &db);
	~MaEtAl();

	unsigned evaluate(std::ostream &os = devnull);

private:
	unsigned int doMaEtAl();

}; /* class MaEtAl */

#endif /* MaEtAl_H */
