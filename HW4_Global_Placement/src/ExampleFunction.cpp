#include "ExampleFunction.h"
#include <cmath>
#include <cstdlib>

// minimize 3*x^2 + 2*x*y + 2*y^2 + 7

ExampleFunction::ExampleFunction(Placement &placement)
	:_placement(placement)
{
	beta = 0;
	Width =_placement.boundryRight() - _placement.boundryLeft();
	Height = _placement.boundryTop() - _placement.boundryBottom();
	gamma = Height / 600;
	binNumPerEdge = 14;
	dBinW = Width / binNumPerEdge;
	dBinH = Height / binNumPerEdge;
	binNum = binNumPerEdge * binNumPerEdge;
	g_temp = new double[_placement.numModules() * 2]();
	dExp = new double[_placement.numModules() * 4]();
	targetDensity = 0;
	for(unsigned i = 0; i < _placement.numModules(); ++i){
		targetDensity += _placement.module(i).area();
	}
	targetDensity = targetDensity / (Width * Height);
	binDensity.resize(binNum);
}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g){

	double x1, x2, y1, y2;
	double overlapX, overlapY, alphaX, betaX, alphaY, betaY, dX, dY, ABSdX, ABSdY;
	double densityRatio;
	//f is the objective cost function
	f = 0;
	int vectorGSize = g.size();

	for(int i = 0; i < vectorGSize; ++i){
		g[i] = 0;
	}
	int modulesCount = _placement.numModules();
	//calculate exp(Xk/r)

	for(int i = 0; i < modulesCount; ++i){
		if (_placement.module(i).isFixed()){	
			Module cur_module = _placement.module(i);
			dExp[4 * i    ] = exp(     cur_module.centerX() / gamma);
			dExp[4 * i + 1] = exp((-1)*cur_module.centerX() / gamma);
			dExp[4 * i + 2] = exp(     cur_module.centerY() / gamma);
			dExp[4 * i + 3] = exp((-1)*cur_module.centerY() / gamma);
		}else{
			dExp[4 * i    ] = exp(     x[2 * i    ] / gamma);
			dExp[4 * i + 1] = exp((-1)*x[2 * i    ] / gamma);
			dExp[4 * i + 2] = exp(     x[2 * i + 1] / gamma);
			dExp[4 * i + 3] = exp((-1)*x[2 * i + 1] / gamma);
		}
	}

	//calculate log-sum-exp and gradient
	for(unsigned i = 0; i <  _placement.numNets(); ++i){
		x1 = x2 = y1 = y2 = 0;
		//calculate inner Σexp(Xk/r) of LSE wirelength
		for(unsigned j = 0; j < _placement.net(i).numPins(); ++j){
			int cur_moduleID =_placement.net(i).pin(j).moduleId();
			x1 += dExp[4 * cur_moduleID    ];
			x2 += dExp[4 * cur_moduleID + 1];
			y1 += dExp[4 * cur_moduleID + 2];
			y2 += dExp[4 * cur_moduleID + 3];
		}
		//calculate Σ(ln(Σexp(Xk/r)) + ln(Σexp(-Xk/r) + ln(Σexp(Yk/r) + ln(Σexp(-Yk/r)) of LSE wirelength
		f += log(x1) + log(x2) + log(y1) + log(y2);
		//calculate gradient of objective function f
		for(unsigned j = 0; j < _placement.net(i).numPins(); j++){
			Module cur_module = _placement.module(i);
			int cur_moduleID =_placement.net(i).pin(j).moduleId();
			//g is the numerator of gradient
			if(cur_module.isFixed()){
				g[2 * cur_moduleID] = g[2 * cur_moduleID + 1] = 0;
			}else{
				g[2 * cur_moduleID    ] += dExp[4 * cur_moduleID    ] / (gamma * x1);
				g[2 * cur_moduleID    ] -= dExp[4 * cur_moduleID + 1] / (gamma * x2);
				g[2 * cur_moduleID + 1] += dExp[4 * cur_moduleID + 2] / (gamma * y1);
				g[2 * cur_moduleID + 1] -= dExp[4 * cur_moduleID + 3] / (gamma * y2);
			}
		}	
	}

	//focus on minimize LSE wirelength, return whenever it is the first iteration
	if(beta==0)	return;
	
	for(int i = 0; i < binNum; ++i){
		binDensity[i] = 0;
	}
	for(int i = 0; i < vectorGSize; ++i){
		g_temp[i] = 0;
	}

	for (int a = 0; a < binNumPerEdge; ++a){
		for (int b = 0; b < binNumPerEdge; ++b){
			for (int i = 0; i < modulesCount; ++i){
				Module cur_module = _placement.module(i);
				if(!cur_module.isFixed()){
					alphaX = 4 / ((dBinW + cur_module.width()) * (2 * dBinW + cur_module.width()));
					betaX  = 4 / (dBinW * (2 * dBinW + cur_module.width()));
					alphaY = 4 / ((dBinH + cur_module.height()) * (2 * dBinH + cur_module.height()));
					betaY  = 4 / (dBinH * (2 * dBinH + cur_module.height()));
					
					densityRatio = cur_module.area()/(dBinW * dBinH);
					
					dX = x[2*i] - ((a+0.5)*dBinW + _placement.boundryLeft());
					ABSdX = abs(dX);
					dY = x[2*i+1] - ((b+0.5)*dBinH + _placement.boundryBottom());
					ABSdY = abs(dY);
					
					//bell-shaped function of bin density smoothing	
					if (ABSdX <= (dBinW / 2 + cur_module.width() / 2)){
						overlapX = 1 - alphaX * ABSdX * ABSdX;
					}else if(ABSdX <= (dBinW + cur_module.width() / 2)){
						overlapX = betaX * (ABSdX - (dBinW + cur_module.width() / 2)) * (ABSdX - (dBinW + cur_module.width() / 2));
					}else{
						overlapX = 0;
					}
					//bell-shaped function of bin density smoothing	
					if (ABSdY <= (dBinH / 2 + cur_module.height() / 2)){
						overlapY = 1 - alphaY * ABSdY * ABSdY;
					}else if(ABSdY <= (dBinH + cur_module.height() / 2)){
						overlapY = betaY * (ABSdY - (dBinH + cur_module.height() / 2)) * (ABSdY - (dBinH + cur_module.height() / 2));
					}else{
						overlapY = 0;
					}
						
					if(ABSdX <= (dBinW / 2 + cur_module.width() / 2)){
						g_temp[2 * i] = densityRatio * ((-2) * alphaX * dX) * overlapY;
					}else if(ABSdX <= (dBinW + cur_module.width() / 2)){
						if(dX > 0){
							g_temp[2 * i] = densityRatio * 2 * betaX * (dX - (dBinW + cur_module.width() / 2)) * overlapY;
						}else{
							g_temp[2 *i ] = densityRatio * 2 * betaX * (dX + (dBinW + cur_module.width() / 2)) * overlapY;
						}
					}else{
						g_temp[2 * i] = 0;
					}					
			
					if(ABSdY <= (dBinH/2 + cur_module.height() / 2)){
						g_temp[2 * i + 1] = densityRatio * ((-2) * alphaY * dY) * overlapX;
					}else if (ABSdY <= (dBinH + cur_module.height() / 2)) {
						if(dY > 0){
							g_temp[2 * i + 1] = densityRatio * 2 * betaY * (dY - (dBinH + cur_module.height() / 2)) * overlapX;
						}else{
							g_temp[2 * i + 1] = densityRatio * 2 * betaY * (dY + (dBinH + cur_module.height() / 2)) * overlapX;
						}
					}else{
						g_temp[2 * i + 1] = 0;
					}
					//calculate overlap length
					binDensity[binNumPerEdge*b + a] += densityRatio * overlapX * overlapY;
				}			
			}
			//calculate (beta * Σ((Db(x, y)-Tb)^2)) part in objective function
			f += beta * (binDensity[binNumPerEdge * b + a] - targetDensity) * (binDensity[binNumPerEdge * b + a] - targetDensity);
			
			for(int j = 0; j < modulesCount; ++j){
				g[2 * j    ] += beta * 2 * (binDensity[binNumPerEdge * b + a] - targetDensity) * g_temp[2 * j    ];
				g[2 * j + 1] += beta * 2 * (binDensity[binNumPerEdge * b + a] - targetDensity) * g_temp[2 * j + 1];
			}
		}
	}
	
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f){

	double x1, x2, y1, y2;
	double overlapX, overlapY, alphaX, betaX, alphaY, betaY, dX, dY, ABSdX, ABSdY;
	double densityRatio;
	//f is the objective cost function
	f = 0;
	int modulesCount = _placement.numModules();
	//calculate exp(Xk/r)
	for(int i = 0; i < modulesCount; ++i){
		if (_placement.module(i).isFixed()){	
			Module cur_module = _placement.module(i);
			dExp[4 * i    ] = exp(     cur_module.centerX() / gamma);
			dExp[4 * i + 1] = exp((-1)*cur_module.centerX() / gamma);
			dExp[4 * i + 2] = exp(     cur_module.centerY() / gamma);
			dExp[4 * i + 3] = exp((-1)*cur_module.centerY() / gamma);
		}else{
			dExp[4 * i    ] = exp(     x[2 * i    ] / gamma);
			dExp[4 * i + 1] = exp((-1)*x[2 * i    ] / gamma);
			dExp[4 * i + 2] = exp(     x[2 * i + 1] / gamma);
			dExp[4 * i + 3] = exp((-1)*x[2 * i + 1] / gamma);
		}
	}
	//calculate log-sum-exp and gradient
	for(unsigned i = 0; i <  _placement.numNets(); ++i){
		x1 = x2 = y1 = y2 = 0;
		//calculate inner Σexp(Xk/r) of LSE wirelength
		for(unsigned j = 0; j < _placement.net(i).numPins(); ++j){
			int cur_moduleID =_placement.net(i).pin(j).moduleId();
			x1 += dExp[4 * cur_moduleID    ];
			x2 += dExp[4 * cur_moduleID + 1];
			y1 += dExp[4 * cur_moduleID + 2];
			y2 += dExp[4 * cur_moduleID + 3];
		}
		//calculate Σ(ln(Σexp(Xk/r)) + ln(Σexp(-Xk/r) + ln(Σexp(Yk/r) + ln(Σexp(-Yk/r)) of LSE wirelength
		f += log(x1) + log(x2) + log(y1) + log(y2);
		
	}
	//focus on minimize LSE wirelength, return whenever it is the first iteration
	if(beta==0)	return;
	
	for(int i = 0; i < binNum; ++i){
		binDensity[i] = 0;
	}
	for (int a = 0; a < binNumPerEdge; ++a){
		for (int b = 0; b < binNumPerEdge; ++b){
			for (int i = 0; i < modulesCount; ++i){
				Module cur_module = _placement.module(i);
				if(!cur_module.isFixed()){
					alphaX = 4/((dBinW + cur_module.width()) * (2 * dBinW + cur_module.width()));
					betaX  = 4/(dBinW * (2 * dBinW + cur_module.width()));
					alphaY = 4/((dBinH + cur_module.height()) * (2 * dBinH + cur_module.height()));
					betaY  = 4/(dBinH * (2 * dBinH + cur_module.height()));
					
					densityRatio = cur_module.area()/(dBinW * dBinH);
					
					dX = x[2*i] - ((a+0.5)*dBinW + _placement.boundryLeft());
					ABSdX = abs(dX);
					dY = x[2*i+1] - ((b+0.5)*dBinH + _placement.boundryBottom());
					ABSdY = abs(dY);
				
					//bell-shaped function of bin density smoothing	
					if (ABSdX <= (dBinW / 2 + cur_module.width() / 2)){
						overlapX = 1 - alphaX * ABSdX * ABSdX;
					}else if(ABSdX <= (dBinW + cur_module.width() / 2)){
						overlapX = betaX * (ABSdX - (dBinW + cur_module.width() / 2)) * (ABSdX - (dBinW + cur_module.width() / 2));
					}else{
						overlapX = 0;
					}
					//bell-shaped function of bin density smoothing	
					if (ABSdY <= (dBinH / 2 + cur_module.height() / 2)){
						overlapY = 1 - alphaY * ABSdY * ABSdY;
					}else if(ABSdY <= (dBinH + cur_module.height() / 2)){
						overlapY = betaY * (ABSdY - (dBinH + cur_module.height() / 2)) * (ABSdY - (dBinH + cur_module.height() / 2));
					}else{
						overlapY = 0;
					}
					//calculate overlap length
					binDensity[binNumPerEdge*b + a] += densityRatio * overlapX * overlapY;
				}			
			}
			//calculate (beta * Σ((Db(x, y)-Tb)^2)) part in objective function
			f += beta * (binDensity[binNumPerEdge * b + a] - targetDensity) * (binDensity[binNumPerEdge * b + a] - targetDensity);
		}
	}
		
}

unsigned ExampleFunction::dimension()
{
    return _placement.numModules() * 2; // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}
