#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H
#include "Placement.h"
#include "NumericalOptimizerInterface.h"

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Placement &placement);
	Placement &_placement;
	
    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
	
	int beta;
	int binNumPerEdge;
	int binNum;
	double gamma;
	double Width;
	double Height;
	double dBinW;
	double dBinH;
	double targetDensity;
	double *g_temp;
	double *dExp;
	vector<double> binDensity;

};
#endif // EXAMPLEFUNCTION_H
