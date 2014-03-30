#pragma once
#include <stdint.h>
#include <vector>
#include "StringRef.hpp"
#include "akjWrappingCounter.hpp"


namespace akj
{
	struct cSubmittedLayer;
	class iLayeredDrawable
	{
	public:

		// warning: destructor is not virtual: 
		// this is an interface and should not be an owned pointer

		virtual void DepthSort() = 0;
		virtual void PrepareLayers(std::vector<cSubmittedLayer>& layers) =0;
		virtual void DrawSingleLayer(uint32_t layer) = 0;
		virtual void PostDrawCleanup() = 0;
		virtual cStringRef LayerName() const = 0;
		//higher priority draws last (on top)
		virtual uint32_t LayerPriority() const = 0;
	};

	//higher goes last (draws on top)
	enum eDrawPriorities
	{
		kPrimitivePriority,
		kTexturedQuadPriority,
		kTextPriority
	};



	struct cSubmittedLayer
	{
		iLayeredDrawable* mDrawable;
		uint32_t mLayer;
		cWrappingCounter mDepth;
		bool operator < (const cSubmittedLayer& rhs) const
		{
			return mDepth == rhs.mDepth 
							//depths are equal, use priority
							? mDrawable->LayerPriority() < rhs.mDrawable->LayerPriority()
							: mDepth < rhs.mDepth;
		}
		void Draw()
		{
			mDrawable->DrawSingleLayer(mLayer);
		}
	};

	// this is the way the interface is meant to be used
	template <typename tDrawableVec, typename tLayerVec>
	void LayeredDraw(const tDrawableVec& drawables, tLayerVec& layers)
	{
		layers.clear();
		for(iLayeredDrawable* drawable: drawables)
		{
			drawable->DepthSort();
		}
		for(iLayeredDrawable* drawable: drawables)
		{
			drawable->PrepareLayers(layers);
		}
		std::sort(layers.begin(), layers.end());
		for(auto& layer: layers)
		{
			//outs << "{"<< layer.mDrawable->LayerName() << "}";
			layer.Draw();
		}
		for(iLayeredDrawable* drawable: drawables)
		{
			drawable->PostDrawCleanup();
		}

	}


	template <typename T> 
	bool PtrLess(const T * const & a, const T * const & b)
	{
		return *a < *b;
	}
}
