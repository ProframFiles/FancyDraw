#pragma once
#include "Bitmap.hpp"
#include <vector>

namespace akj {

class MipMapper{
public:
	enum eMipMapOptions
	{
		OPT_NO_SIMD
	};

	// generate all desired mip maps. output stride is determined by 
	// output buffer size
	template <typename tScalar>
	static void GenerateMipLevels(const cAlignedBitmap & bitmap, 
									tScalar** p_buf_array, 
									tScalar** p_end_array, 
									uint32_t min_level, 
									uint32_t max_level);

	static void GenerateMipLevels(const cAlignedBitmap & bitmap,
																const std::vector<cAlignedBitmap>& mips);

	static uint32_t GetScaledDimension(uint32_t val, uint32_t level)
	{
		const uint32_t scale_m_one = (1 << level) - 1;
		return (val + scale_m_one) >> level;
	}

	static cAlignedBitmap EmptyMipBitmap(const cAlignedBitmap& bm, uint32_t level)
	{
		const uint32_t w = GetScaledDimension(bm.W(), level);
		const uint32_t h = GetScaledDimension(bm.H(), level);
		return cAlignedBitmap(w, h, bm.Comp(), bm.BPC());
	}
	static cAlignedBitmap GenerateMipLevel(const cAlignedBitmap & bitmap, 
																	cAlignedBuffer& buffer, uint32_t level);

};
}

