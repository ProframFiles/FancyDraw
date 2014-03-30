#include "UtilPreHead.hpp"
#include "SplineMaker.hpp"
#include "splineDef.hpp"
int SplineMaker::processQ()
{
	auto iters = aiQ.flip();
	float smoothFac=0.9565378f;
	std::for_each(iters.first, iters.second,
		[this, smoothFac](akjInput &aI){
			mouseNow.addNew(&aI, smoothFac);
			//pantheios::log_NOTICE(L"m: ", pantheios::real(mouseNow.x), L": ", pantheios::real(aI.x));
	});
	return 0;
}
int SplineMaker::startLine()
{
	return 0;
}
int SplineMaker::endLine()
{
	return 0;
}
namespace akj{
splineSpanCol SmoothingSpline::calcMatSection(int sec, int kleft)
{
	splineSpanCol c;
	int kmax=static_cast<int>(knotVec.size()-1);
	c.zeros();
	float zero=knotVec[Clamp(kleft-sec,kmax,0)];
	float k1=knotVec[Clamp(kleft-sec+1,kmax,0)];
	float k2=knotVec[Clamp(kleft-sec+2,kmax,0)];
	float k3=knotVec[Clamp(kleft-sec+3,kmax,0)];
	float d01=k1-zero;
	float d12=k2-k1;
	float d23=k3-k2;
	float d34=knotVec[Clamp(kleft-sec+4,kmax,0)]-k3 ;
	//if (d23==0.0f) d23=d34;
	//if (d34==0.0) d34=d23;
	
	//d01=d12=d34=d23=1.0f;
	float cfac;
	if((sec==0)&&d01!=0.0f){
		 cfac=1.0f/(d01*(d01 + d12)*(d01 + d12 + d23));
		 c(3,0)=0.0f;
		 c(2,0)=0.0f;
		 c(1,0)=0.0f;
		 c(0,0)=cfac;
	 }
	 else if((sec==1)&&d12!=0.0f){
		 cfac=1.0f/(d12*(d01 + d12)*(d12 + d23)*(d01 + d12 + d23)*(d12 + d23 + d34));
		  c(3,0)=cfac*(d01*d01*d12*(d23*(d23 + d34) + d12*(d12 + 2*d23 + d34))); 
		c(2,0)=cfac*(d01*d12*(d23*(3*d23 + 3*d34) + d12*(3*d12 + 6*d23 + 3*d34))); 
		  c(1,0)=cfac*(d12*(d23*(3*d23 + 3*d34) + d12*(3*d12 + 6*d23 + 3*d34)));
		  c(0,0)=cfac*(d12*(-3*d12 - 4*d23 - 2*d34) + d01*(-d01 - 3*d12 - 2*d23 - d34) + d23*(-d23 - d34));
	 }
	 else if((sec==2)&&d23!=0.0f){
		  cfac=1.0f/(d23*(d12 + d23)*(d01 + d12 + d23)*(d23 + d34)*(d12 + d23 + d34));
				c(3,0)=cfac*(d01*( d23*d23*( d34*d34 + d23*(d23 + 2*d34)) + d12*d23*( d34*d34+d23*(2*d23 + 3*d34))) +
						  d12*(d12*d23*( d34*d34 + d23*(2*d23 + 3*d34)) + d23*d23*(2* d34*d34 + d23*(2*d23 + 4*d34))));
			  c(2,0)=cfac*(d01*d12*d23*(-3*d23 - 3*d34) +  d12*d12*d23*(-3*d23 - 3*d34) +
								d23*d23*(3* d34*d34 + d23*(3*d23 + 6*d34)));
			  c(1,0)=cfac*(d01*d23*(-3*d23 - 3*d34) + d23*(d23*(-6*d12 - 6*d23 - 9*d34) - 6*d12*d34 - 3* d34*d34));
				c(0,0)=cfac*( d34*d34 + d01*(d12 + 2*d23 + d34) + d12*(d12 + 4*d23 + 2*d34) + d23*(3*d23 + 3*d34));
	 }
	 else if((sec==3)&&d34!=0.0f){
		  cfac=1.0f/(d34*(d23 + d34)*(d12 + d23 + d34));
			c(3,0)=cfac* d34*d34*d34;
			c(2,0)=-cfac*3* d34*d34;
		  c(1,0)=cfac*3*d34;
			c(0,0)=-cfac;
	 }
	 return c;
}
}