////////////////////////////////////////////////////////////////////////////
//
// file CubicSpline.cpp
//
////////////////////////////////////////////////////////////////////////////
#include "akjCubicSpline.hpp"
#include "akjExceptional.hpp"
#include <algorithm>

namespace akj
{

/**
 * Calculates the blend matrix.
 ***/

void CubicSpline::calcBlendMatrix(void)
{
	if (localBIndx + 5 >= knotVec.size()) {
		AKJ_THROW("Blend matrix for out-of-range knot");
	}

	const float* k = &knotVec[0] + localBIndx;
	const float d1 = k[1] - k[0];
	const float d2 = k[2] - k[1];
	const float d3 = k[3] - k[2];
	const float d4 = k[4] - k[3];
	const float d5 = k[5] - k[4];
	const float d3sq = d3 * d3;
	const float d23 = d2 + d3;
	const float d234 = d23 + d4;
	const float a = d3sq / ((d1 + d2 + d3) * d23);
	const float b = d3sq / (d234 * (d2 + d3));
	const float c = d3sq / (d234 * (d3 + d4));
	const float d = d3sq / ((d3 + d4 + d5) * (d3 + d4));
	float e = (d2) / (d234 * d23);
	const float f = d2 * e;
	e *= d3;
	B[0] = -a;	      B[4] = a + b + c;     B[8]  = -b - c - d; B[12] = d;
	B[1] = 3 * a;	  B[5] = -3 * (a + b);	  B[9]  = 3 * b;	      B[13] = 0.0f;
	B[2] = -3 * a; 	B[6] = 3 * (a - e);   B[10] = 3 * e;	      B[14] = 0.0f;
	B[3] = a;       B[7] = 1 - a - f;     B[11] = f;	          B[15] = 0.0f;
}


void CubicSpline::insertKnot(uint32_t indx, float k /*=1.0f*/)
{
	if ((indx) >= knotVec.size()) {
		appendKnot(k);
		return;
	}

	knotVec.push_back(0);
	for (size_t i = indx == 0? 1: indx; i < knotVec.size() ; ++i)
	{
		knotVec[i] = knotVec[i-1] + k;
	}
	if(indx == 0)
	{
		knotVec[0]=k;
	}
}
}//end namespace akj



