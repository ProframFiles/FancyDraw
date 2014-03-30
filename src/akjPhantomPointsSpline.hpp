////////////////////////////////////////////////////////////////////////////
//
// file PhantomPointsSpline.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef AKJ_PHANTOMPOINTSSPLINE_HPP
#define AKJ_PHANTOMPOINTSSPLINE_HPP
#include "akjCubicSpline.hpp"
#include "Eigen\Core"
#include <vector>
#include <unordered_set>

namespace akj
{
template <typename tNVec>


class PhantomPointsSpline
{
public:
	PhantomPointsSpline(std::initializer_list<tNVec> list) {
		mControlPoints.reserve(std::max(64, (int)(list.size())));
		for(auto& vec : list){
			AppendCP(vec);
		}
	};
	~PhantomPointsSpline() {};

	inline PhantomPointsSpline& AppendCP(const tNVec & cp) {
		// add the point
		uint32_t i = NumCP();
		if(mControlPoints.empty()){
			AddInitialPoint(cp);
		}
		else{
			mControlPoints.emplace(mControlPoints.begin() + i + 1, cp);
		}
		
		const uint32_t spans = NumSpans();
		
		if(spans > 1){
			splineBase.appendKnot();
		}

		//maintain the phantom properties
		adjustEnd();
		if(u32(NumSpans()) <= 1 ) adjustStart();
		return *this;
	}



	PhantomPointsSpline& AdjustCP(uint indx, tNVec * CPptr) {
		mControlPoints.at(indx + 1) = (*CPptr);

		if (indx < 2) {
			adjustStart();
		}

		if (indx > NumCP() - 2) {
			adjustEnd();
		}

		return *this;
	}

	inline uint32_t NumCP() const {
		return u32(mControlPoints.size()) - 2;
	}
	inline uint32_t NumSpans() const {
		return u32(mControlPoints.size()) - 3;
	}
	/**
	 * Gets an apparent control point. this kind of spline is based on phantom 
	 * points. So there are actually 2 more points in the spline then are 
	 * apparent. We return the cp of index+1, basically.
	 *
	 * \param	indx	The control point index, 0 based.
	 * \return	null if it fails, else the apparent control point.
	 ***/
	inline const tNVec& ControlPoint(uint32_t indx) {
		return &mControlPoints.at((indx + 1));
	}

	template <class tListOfT, class tListOfCP>
	PhantomPointsSpline& eval(uint spanID, const tListOfT& in_t, tListOfCP& cp){
		if(spanID >= NumSpans())
		{
			AKJ_THROW("Phantom point span " + Twine(spanID) + "< "
								+ Twine(NumSpans())+" , Out of range.");
		}
		Eigen::Matrix<float, 1, 4> tmat;
		tmat(3)=1.0f;
		auto tmpCP = Eigen::Map < Eigen::Matrix < float, sizeof(tNVec) / 4, 4 > > 
			((float*)&mControlPoints.at(spanID)).transpose();
		auto tmpBCP = Eigen::Map<const Eigen::Matrix<float, 4, 4> >
			(splineBase.getBlendMatrix(spanID))*tmpCP;
		Eigen::Matrix<float,1, sizeof(tNVec) / 4 > ret;
		for (uint i = 0; i < u32(in_t.size()); i++)
		{
			const float t=in_t[i];
			const float tsq=t*t;
			tmat(0)=tsq*t; tmat(1)=tsq; tmat(2)=t;
			ret=tmat*tmpBCP;
			cp.emplace_back(*((tNVec*)ret.data()));
		}
		return *this;
	}

	const float * SpanMatrix(size_t indx) {
		auto tmpCP = Eigen::Map < Eigen::Matrix < float, tNVec::kNumElements, 4 > > 
			((float*)&mControlPoints.at(indx)).transpose();
		BCP = Eigen::Map<Eigen::Matrix<float, 4, 4> >(
			splineBase.getBlendMatrix(indx)) * tmpCP;
		return BCP.data();
	}
private:

	void AddInitialPoint(const tNVec & CPptr) {
		mControlPoints.resize(3, CPptr);
	}

	inline void adjustEnd() {
		size_t indx = NumCP();
		const tNVec& f1 = mControlPoints.at(indx - 1);
		tNVec& fe = mControlPoints.at(indx + 1);
		const tNVec& f0 = mControlPoints.at(indx);

		fe = 2.0f * f0 - f1;		
	}
	inline void adjustStart() {
		const tNVec&  f1 = mControlPoints.at(2);
		tNVec&  fe = mControlPoints.at(0);
		const tNVec&  f0 = mControlPoints.at(1);

		fe = 2.0f * f0 - 1.0f*f1;		
	}
	CubicSpline splineBase;
	std::vector<tNVec> mControlPoints;
	Eigen::Matrix < float, 4, sizeof(tNVec) / 4 >  BCP;
};


}//end namespace akj
#endif
