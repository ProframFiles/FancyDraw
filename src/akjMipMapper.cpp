#include "akjMipMapper.hpp"
#include "akjMipMapperImpl.hpp"
#include "Bitmap.hpp"

#include <cstdlib>


namespace akj{

	template <>
	void MipMapper::GenerateMipLevels( const cAlignedBitmap& bitmap,
										 uint8_t** p_buf_array, 
										 uint8_t** p_end_array, 
										 uint32_t min_level, 
										 uint32_t max_level )
	{
		switch (bitmap.NumComponents())
		{
		case(1):
			MipMapperImpl<1, uint8_t>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		case(2):
			MipMapperImpl<2, uint8_t>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		case(3):
			MipMapperImpl<3, uint8_t>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		case(4):
			MipMapperImpl<4, uint8_t>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		default:
			// not implemented
			assert(false);
			break;
		}
	}

	template <>
	void MipMapper::GenerateMipLevels( const cAlignedBitmap& bitmap,
										 float** p_buf_array, 
										 float** p_end_array, 
										 uint32_t min_level, 
										 uint32_t max_level )
	{
		switch (bitmap.NumComponents())
		{
		case(1):
			MipMapperImpl<1, float>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		case(2):
			MipMapperImpl<2, float>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		case(4):
			MipMapperImpl<4, float>::GenerateMipLevels(bitmap, p_buf_array, p_end_array, min_level, max_level);
			break;
		default:
			// not implemented
			assert(false);
			break;
		}
	}

	void MipMapper::GenerateMipLevels(const cAlignedBitmap & bitmap, 
																		const std::vector<cAlignedBitmap>& mips)
	{
		if(mips.empty()) return;

		uint8_t* begins[32];
		uint8_t* ends[32];
		uint32_t h = bitmap.H();
		uint32_t min_level = 0;
		while(h > mips[0].H())
		{
			min_level++;
			h >>= 1;
		}
		uint32_t max_level = min_level + static_cast<uint32_t>(mips.size())-1;
		for (size_t i = 0; i < mips.size() ; ++i)
		{
			begins[i] = mips[i].Data();
			ends[i] = begins[i] + mips[i].Size();
		}
		if(bitmap.BPC() == 4)
		{
			GenerateMipLevels(bitmap, 
												reinterpret_cast<float**>(begins), 
												reinterpret_cast<float**>(ends), 
												min_level, max_level);
		}
		else if(bitmap.BPC() == 1)
		{
			GenerateMipLevels(bitmap, begins, ends, min_level, max_level);
		}
	}
	
	cAlignedBitmap MipMapper::GenerateMipLevel(
		const cAlignedBitmap & bitmap, cAlignedBuffer& buffer, uint32_t level)
	{
		if (level == 0) return bitmap;
		cAlignedBuffer temp_buffer;
		cAlignedBitmap next_bm((bitmap.Width()+1)>>1, (bitmap.Height()+1)>>1, 
			bitmap.Comp(), bitmap.BPC());
		
		if(level == 1)
		{
			next_bm.UseAsStorage(buffer);
		}
		else
		{
			next_bm.UseAsStorage(temp_buffer);
		}
		uint8_t* begin = next_bm.Data();
		uint8_t* end = (next_bm.Size() + begin);
		if(bitmap.BPC() == 4)
		{
			GenerateMipLevels(bitmap, 
						reinterpret_cast<float**>(&begin),
						reinterpret_cast<float**>(&end), 1, 1);
		}
		else
		{
			GenerateMipLevels(bitmap, &begin, &end, 1, 1);
		}
		if(level == 1) 
		{
			return next_bm;
		}
		cAlignedBitmap ret_bm(buffer, GetScaledDimension(bitmap.Width(), level),
													GetScaledDimension(bitmap.Height(), level),
													bitmap.Comp(), bitmap.BPC());
		AKJ_ASSERT(ret_bm.Size()>0);
		begin = ret_bm.Data();
		end = begin+ret_bm.Size();

		if(next_bm.BPC() == 4)
		{
			GenerateMipLevels(next_bm, 
						reinterpret_cast<float**>(&begin),
						reinterpret_cast<float**>(&end), level-1, level-1);
		}
		else{
			GenerateMipLevels(next_bm, &begin, &end, level-1, level-1);
		}
		return ret_bm;
	}
}
