#pragma once
#include "FancyDrawMath.hpp"
#include <vector>
namespace akj{
	class cShaderObject;

	class cLightManager
	{
	public:
		cLightManager(void);
		virtual ~cLightManager(void);
		virtual void SetLights(cShaderObject* shader) const;
	private:
		cCoord4 mArrowLight;
		cCoord4 mAmbientLight;
		std::vector<cCoord4> mOrangeLightVec;
		std::vector<cCoord4> mBlueLightVec;
	};
}
