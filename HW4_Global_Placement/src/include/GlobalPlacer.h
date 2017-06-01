#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Placement.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

class GlobalPlacer 
{
public:
    GlobalPlacer(Placement &placement);
	void place();
	void randomPlace(vector<double> &x);
    void plotPlacementResult( const string outfilename, bool isPrompt = false );

private:
    Placement& _placement;
    void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 );
};

#endif // GLOBALPLACER_H
