#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
	:_placement(placement)
{

}

// Randomly place modules implemented by TA
void GlobalPlacer::randomPlace(vector<double> &x){
	double w = _placement.boundryRight() - _placement.boundryLeft();
	double h = _placement.boundryTop() - _placement.boundryBottom();
	for (size_t i = 0; i < _placement.numModules(); ++i){
	//	if(_placement.module(i).isFixed()) { continue; }
		double wx = _placement.module(i).width(), 
			   hx = _placement.module(i).height();
		double px = (int) rand() % (int)(w - wx) + _placement.boundryLeft();
		double py = (int) rand() % (int)(h - hx) + _placement.boundryBottom();
		x[2 * i] = px;
		x[2 * i + 1] = py;
		_placement.module(i).setPosition(px, py);
	}
}


void GlobalPlacer::place()
{
	///////////////////////////////////////////////////////////////////
	// The following example is only for analytical methods.
	// if you use other methods, you can skip and delete it directly.
	//////////////////////////////////////////////////////////////////

	ExampleFunction ef(_placement); // require to define the object function and gradient function

    vector<double> x;
	x.resize(ef.dimension()); // solution vector, size: num_blocks*2 
                         // each 2 variables represent the X and Y dimensions of a block
    NumericalOptimizer no(ef);

	////////////////////////////////////////////////////////////////
	
	// An example of random placement by TA. If you want to use it, please uncomment the folllwing 2 lines.
	srand(time(NULL));
	randomPlace(x);
	
	int round = 4;
	for(int i = 0; i < round; ++i){
	
		ef.beta = i * 5000;
		no.setX(x);//set initial solution
		if(!i){
			no.setNumIteration(150); // user-specified parameter
		}
		else{
			no.setNumIteration(30); // user-specified parameter
		}
		no.setStepSizeBound((_placement.boundryRight() - _placement.boundryLeft())*7); // user-specified parameter
		no.solve(); // Conjugate Gradient solver
		int numModulesCount = _placement.numModules();
		for(int i = 0; i < numModulesCount; ++i){
		double cur_x = no.x(2 * i);
		double cur_y = no.x(2 * i + 1);
		//	double cur_x = x[2 * i];
		//	double cur_y = x[2 * i + 1];
			//control boundry
			if(cur_x + _placement.module(i).width() > _placement.boundryRight()){
				cur_x = _placement.boundryRight() - _placement.module(i).width();
			}
			else if(cur_x - _placement.module(i).width() < _placement.boundryLeft()){
				cur_x = _placement.boundryLeft();
			}
			else{
				//Null
			}
			
			if(cur_y + _placement.module(i).height() > _placement.boundryTop()){
				cur_y = _placement.boundryTop() - _placement.module(i).height();
			}
			else if(cur_y - _placement.module(i).height() < _placement.boundryBottom()){
				cur_y = _placement.boundryBottom();
			}
			else{
				//Null
			}
			
			_placement.module(i).setPosition(cur_x, cur_y);

			x[2 * i] = cur_x; 
			x[2 * i + 1] = cur_y; 
		}
	
	
	
	
	
	}
	
	/* @@@ TODO 
	 * 1. Understand above example and modify ExampleFunction.cpp to implement the analytical placement
	 * 2. You can choose LSE or WA as the wirelength model, the former is easier to calculate the gradient
     * 3. For the bin density model, you could refer to the lecture notes
     * 4. You should first calculate the form of wirelength model and bin density model and the forms of their gradients ON YOUR OWN 
	 * 5. Replace the value of f in evaluateF() by the form like "f = alpha*WL() + beta*BinDensity()"
	 * 6. Replace the form of g[] in evaluateG() by the form like "g = grad(WL()) + grad(BinDensity())"
	 * 7. Set the initial vector x in main(), set step size, set #iteration, and call the solver like above example
	 * */
	
}


void GlobalPlacer::plotPlacementResult( const string outfilename, bool isPrompt )
{
    ofstream outfile( outfilename.c_str() , ios::out );
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT( outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop() );
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;
	int numModulesCount = _placement.numModules();
    for( size_t i = 0; i < numModulesCount; ++i ){
        Module &module = _placement.module(i);
        plotBoxPLT( outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height() );
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if( isPrompt ){
        char cmd[ 200 ];
        sprintf( cmd, "gnuplot %s", outfilename.c_str() );
        if( !system( cmd ) ) { cout << "Fail to execute: \"" << cmd << "\"." << endl; }
    }
}

void GlobalPlacer::plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
