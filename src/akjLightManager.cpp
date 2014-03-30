#include "akjLightManager.hpp"
#include "akjShaderObject.hpp"

namespace akj{

	cLightManager::cLightManager(void)
		:mAmbientLight(0.2f)
	{
	}


	cLightManager::~cLightManager(void)
	{
	}

	void cLightManager::SetLights(akj::cShaderObject* shader) const
	{
		shader->BindUniformToVec4("uAmbient", mAmbientLight);
		shader->BindUniformToVec4("uPointerLoc", mArrowLight);
		shader->BindUniformToVec4Array("uOrangeLights", mOrangeLightVec);
		shader->BindUniformToVec4Array("uBlueLights", mBlueLightVec);
	}
}