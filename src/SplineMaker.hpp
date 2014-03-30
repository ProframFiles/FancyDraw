#include "UtilPreHead.hpp"
#ifndef AKJSPLINEMAKER_H
#define AKJSPLINEMAKER_H
#include "akj_util.hpp"
#include "splineDef.hpp"

class SplineMaker 
{
public:
	SplineMaker(void){rawInputBuffer.resize(16);};
	~SplineMaker(void){};
	inline akj::SplineSet* getSplineSetPtr(){return &splineSet;}; 
	void drawAll(ID3D11DeviceContext* DC);
	int startLine();
	int endLine();
	inline void addPoint(akjInput& aI){
		aiQ.addItem(aI);
	};
	int readRawInputBuffer();
	inline void rawInputPoint(RAWINPUTHEADER& rih, RAWINPUT& ri){
		riQ.addItem(std::make_pair(rih, ri));
	};
	int processQ();

private:
	akjInput mouseNow;
	akjInput mouseThen;
	std::vector<RAWINPUT> rawInputBuffer;
	arma::fmat calcMat(int sec, std::vector<float>::iterator v);
	akj::SplineSet splineSet;
	akjDoubleQueue<std::pair<RAWINPUTHEADER, RAWINPUT> > riQ;
	akjDoubleQueue<akjInput> aiQ;
};
#endif