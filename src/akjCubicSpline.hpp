////////////////////////////////////////////////////////////////////////////
//
// file CubicSpline.hpp
//
////////////////////////////////////////////////////////////////////////////

#ifndef AKJ_CUBICSPLINE_HPP
#define AKJ_CUBICSPLINE_HPP
#include <vector>
#include <stdint.h>

namespace akj {

	class CubicSpline {
	public:
		CubicSpline():localBIndx(0){
			knotVec.reserve(32);
			knotVec.resize(6);
			for (int i=0;i<6;i++)
			{
				knotVec[i]=static_cast<float>(i);
			}
			calcBlendMatrix();
		};
		~CubicSpline(){};
		
		/**
		* Appends a knot.
		*
		* \param	k	The size of the new knot span, the inserted knot will be last+k.
		*
		* \return	reference to this
		***/
		inline void appendKnot(float k=1.0f){
			k+=knotVec.back();
			knotVec.push_back(k);
		}
		
		void insertKnot(uint32_t indx, float k=1.0f);

		inline const float* getBlendMatrix(uint32_t indx){
			if(localBIndx!=indx)
			{
				localBIndx=indx;
				calcBlendMatrix();
			}
			return B;
		}
	private:
		void calcBlendMatrix();
		float B[16];
		size_t localBIndx;
		std::vector<float> knotVec;};

}//end namespace akj
#endif
