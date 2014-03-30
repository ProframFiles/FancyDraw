//#include "UtilPreHead.hpp"
//#include "akj_util.hpp"

#ifndef AKJSPLINEDEF_H
#define AKJSPLINEDEF_H
namespace akj{
typedef float knotType;
typedef arma::fmat44 splineSpanMat;
typedef arma::fmat::fixed<4,1> splineSpanCol;
typedef arma::fmat::fixed<4,4> spanControlPoints;
typedef arma::fmat splineMat;
class SplineCP
{
	public:
	SplineCP(){};
	~SplineCP(){};
	float x;
	float y;
	float width;
	float color;
	
};
class SplineSpanIndex
{
	public:
	int spline;
	int span;
	inline bool SplineSpanIndex::operator <(const SplineSpanIndex& b) const{return (spline==b.spline?span>b.span:spline>b.spline);};
};
class SplineSpanRef
{
	public:
	SplineSpanRef():points(NULL),trnsMat(NULL){};
//	SplineSpanRef(const SplineSpanRef & spr):points(spr.points),trnsMat(spr.trnsMat){};
	SplineSpanRef(spanControlPoints const & scp,splineSpanMat const & tM):points(&scp),trnsMat(&tM){};
	~SplineSpanRef(){};
	knotType start;
	knotType span;
	int id;
	int ex;
	spanControlPoints const *  points;
	splineSpanMat const *  trnsMat;
};
class SmoothingSpline
{
public:
	inline SmoothingSpline(int indx):sizeChunk(8),ID(indx){
		//knotVec.reserve(sizeChunk+1);
		//knotVec.push_back(0.0f);
		sm.resize(4,(sizeChunk)*4);
		sm.zeros();
	};
	~SmoothingSpline(void){};

	inline const std::set<SplineSpanIndex>& appendKnot(knotType k){
		if(!knotVec.empty()) k+=knotVec.back();
		knotVec.push_back(k);
		resizeMat();
		rejig(static_cast<int>(knotVec.size())-6, static_cast<int>(knotVec.size())-1);
		return tmpSet;
	};

	inline const  std::set<SplineSpanIndex>& adjustKnot(int n, knotType k){
		if( n>=knotVec.size()||n<0) throw akjException();

		if(k>=0.0f && n==knotVec.size()-1){
			knotVec[n]+=k;
		}else if(k>=0.0f){
			knotVec[n]=(std::min(knotVec[1+n],knotVec[n]+k ));
		}else if(k<0.0f&&n==0){
			knotVec[0]=std::max(knotVec[0],knotVec[0]+k);
		}else if(k<0.0){
			knotVec[n]=(std::max(knotVec[n-1],knotVec[n]+k ));
		}else{
			throw akjMissedCaseException();
		}
		rejig(n-5, n+5);
		return tmpSet;
	};
	inline void calcMat(int span){
		for(int row=0;row<=3;row++){
			sm.col(span*4+row) = calcMatSection(row,span);
		}
	};
	inline void calcMat(){
		for(unsigned int span = 0;span<knotVec.size()-1 ;span++){
			calcMat(span);
		}
	};

protected:
	int sizeChunk;
	inline void resizeMat(){
		while(knotVec.size()>sizeChunk){
			sm.insert_cols( sizeChunk*4-1, sizeChunk*4, true );
			sizeChunk*=2;
			knotVec.reserve(sizeChunk+1);
		}
	};
	inline void rejig(int bottom,int top)
	{
		SplineSpanIndex ssi;
		tmpSet.clear();
		ssi.spline=ID;
		auto setIter=tmpSet.begin();
		for(int span = std::max(0,bottom); span< std::min(top, static_cast<int>(knotVec.size())-1) ;span++){
			ssi.span=span;
			setIter=tmpSet.insert(setIter, ssi);
			calcMat(span);
		}
	};
	int ID;
	splineMat sm;
	std::set<SplineSpanIndex> tmpSet;
	int nSpans;
	std::vector<knotType> knotVec;
	splineSpanCol calcMatSection(int section, int kleft);
};

class Spline: public SmoothingSpline
{
	public:
	Spline():SmoothingSpline(0){};
	Spline(int indx):SmoothingSpline(indx){};
	~Spline(void){};
	inline  const  std::set<SplineSpanIndex>& startSpline(const SplineCP& inCP){
		return appendNode(inCP, 0.0f);
	};
	inline const  std::set<SplineSpanIndex>& appendNode(const SplineCP& cp, knotType k){

		if(cpVec.size()>=1){
			cpVec.push_back(cp);
			return appendKnot(k);
		}
		else if(cpVec.size()==0){
			cpVec.push_back(cp);
			return appendKnot(0.0f);
		}
		else if(cpVec.size()==1){
			cpVec.push_back(cp);
			return appendKnot(k*0.3333333f);
		}
		cpVec.push_back(cp);
		return adjustKnot(1, k*0.3333333f);
	};
	inline  const std::set<SplineSpanIndex>& adjustLastControlPoint(const SplineCP& cp){
		cpVec[cpVec.size()-1]=cp;
		rejig(static_cast<int>(knotVec.size())-5, static_cast<int>(knotVec.size())-1);
		return tmpSet;
	};
	inline const SplineSpanRef& getSpanRef(int span){
		int maxCP=static_cast<int>(cpVec.size())-1;
		int cp=span+3;
		tmp_cp.row(0)=arma::Row<float>((float*)&cpVec.at(Clamp(cp,maxCP,0)), 4);
		tmp_cp.row(1)=arma::Row<float>((float*)&cpVec.at(Clamp(cp-1,maxCP,0)), 4);
		tmp_cp.row(2)=arma::Row<float>((float*)&cpVec.at(Clamp(cp-2,maxCP,0)), 4);
		tmp_cp.row(3)=arma::Row<float>((float*)&cpVec.at(Clamp(cp-3,maxCP,0)), 4);
		
		tmp_sm=sm.submat(arma::span::all,arma::span((span)*4,(1+span)*4-1));
		tmpRef= SplineSpanRef(tmp_cp,tmp_sm);
		tmpRef.start=knotVec.at(span);
		tmpRef.span=knotVec.at(1+span)-tmpRef.start;
		tmpRef.id=span;
		tmpRef.ex=4;
		return tmpRef;
	};

	private:
	SplineSpanRef tmpRef;
	splineSpanMat tmp_sm;
	spanControlPoints tmp_cp;
	std::vector<SplineCP> cpVec;
};

class SplineSet
{
	public:
	SplineSet():splineIndx(0),iteratorOK(false){};
	~SplineSet(){};
	inline void commitUpdate(const SplineSpanIndex& ssi){
		//::pantheios::log_DEBUG( L"commit update: spline: ", ::pantheios::integer(ssi.spline),L", span: ",::pantheios::integer(ssi.span));
		updateSet.insert(ssi);
	};
	inline void commitUpdate(const std::set<SplineSpanIndex>& ssiSet){
		//::pantheios::log_DEBUG( L"commit update: spline: ", ::pantheios::integer(ssiSet.begin()->spline),L", span: ",::pantheios::integer(ssiSet.begin()->spline));
		updateSet.insert(ssiSet);
	};

	inline bool getNextSpan(SplineSpanIndex& ssi){
		bool ret=false;
		if(!iteratorOK){
			updateSet.flip();
			updateIter=updateSet.begin();
			iteratorOK=true;
		}
		if(updateIter==updateSet.end()){
			iteratorOK=false;
			ret =false;
		}
		else{
			ssi=*updateIter;
			updateIter++;
			ret=true;
		}
		return ret;
	};
	inline const SplineSpanRef& getSpanRef(const SplineSpanIndex& ssi){return getSpanRef(ssi.spline, ssi.span);};
	inline const SplineSpanRef& getSpanRef(int spline, int span){return splineMap[spline].getSpanRef(span);};
	inline const std::set<SplineSpanIndex>& appendToCurrent(const SplineCP& cp, knotType knot){
		return curSpline->appendNode(cp, knot);
	};
	inline const std::set<SplineSpanIndex>& adjustLastPoint(const SplineCP & cp){
		return curSpline->adjustLastControlPoint(cp);
	};
	
	inline const std::set<SplineSpanIndex>& startNew(const SplineCP& cpv){
		Spline ss(static_cast<int>(splineIndx));
		splineMap.insert(std::make_pair(splineIndx, ss));
		curSpline=&splineMap[splineIndx];
		splineIndx++;
		return curSpline->startSpline(cpv);
	};
	inline void startNew(){
		Spline ss(static_cast<int>(splineIndx));
		splineMap.insert(std::make_pair(splineIndx, ss));
		curSpline=&splineMap[splineIndx];
		splineIndx++;
	};
	private:
	int splineIndx;
	Spline* curSpline;
	std::map<int, Spline> splineMap;
	std::set<SplineSpanIndex>::iterator updateIter;
	bool iteratorOK;
	akj::BufferedSet<SplineSpanIndex> updateSet;
};
}
#endif

//SplineMaker
//startSpline(location)
//endSpline()
//addPoint(location, knotPos)
//removePoint

//spline,
//knots,
//control points,
//trans matrix 



//splineDrawer
//updatespline(shared ptr to set of indices)