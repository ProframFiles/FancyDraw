#pragma once
#include "akjPixelFormats.hpp"
#include "imageworsener/imagew.h"

namespace akj
{

	template <class tFunctor, 
						class tPixAgen, class tPixBgen, class tPixCgen, class tPixDgen>
	inline void ForEachPixel( tFunctor my_function,
													const tPixAgen& a_gen, const tPixBgen& b_gen,
													const tPixCgen& c_gen, const tPixDgen& d_gen )
	{
		auto a_pix = a_gen.begin();
		auto b_pix = b_gen.begin();
		auto c_pix = c_gen.begin();
		auto d_pix = d_gen.begin();
		while(a_pix != a_gen.end())
		{
			my_function(*a_pix, *b_pix, *c_pix, *d_pix);
			++a_pix;
			++b_pix;
			++c_pix;
			++d_pix;
		}
	}
	template <class tFunctor, class tPixAgen, class tPixBgen, class tPixCgen>
	inline void ForEachPixel( tFunctor my_function,
													const tPixAgen& a_gen, const tPixBgen& b_gen,
													const tPixCgen& c_gen)
	{
		auto a_pix = a_gen.begin();
		auto b_pix = b_gen.begin();
		auto c_pix = c_gen.begin();
		while(a_pix != a_gen.end())
		{
			my_function(*a_pix, *b_pix, *c_pix);
			++a_pix;
			++b_pix;
			++c_pix;
		}
	}

	template <class tFunctor, class tPixAgen, class tPixBgen>
	inline void ForEachPixel( tFunctor my_function,
													const tPixAgen& a_gen, const tPixBgen& b_gen)
	{
		auto a_pix = a_gen.begin();
		auto b_pix = b_gen.begin();
		while(a_pix != a_gen.end())
		{
			my_function(*a_pix, *b_pix);
			++a_pix;
			++b_pix;
		}
	}

	template <class tFunctor, class tPixAgen>
	inline void ForEachPixel( tFunctor my_function, const tPixAgen& a_gen)
	{
		auto a_pix = a_gen.begin();
		while(a_pix != a_gen.end())
		{
			my_function(*a_pix);
			++a_pix;
		}
	}

	template <uint32_t kAlignment>
	inline void FlipVertically(const cBitmap<kAlignment>& bitmap)
	{
		size_t bpl = bitmap.BytesPerLine();
		if(kAlignment >= 4 && (bitmap.BytesPerLine() & 3) ==0)
		{
			bpl/=4;
			for (uint32_t i = 0; i < bitmap.H(); ++i)
			{
				uint32_t* bot_row 
					= reinterpret_cast<uint32_t*>(bitmap.RowData(bitmap.H()-i-1));
				uint32_t* bot_row_end = bot_row+bpl;
				uint32_t* top_row = reinterpret_cast<uint32_t*>(bitmap.RowData(i));
				if(bot_row <= top_row) break;
				while(bot_row != bot_row_end)
				{
					const uint32_t temp = *bot_row;
					*bot_row++ = *top_row;
					*top_row++ = temp;
				}
			}
		}
		else
		{
			for (uint32_t dest_row_i = 0; dest_row_i < bitmap.H(); ++dest_row_i)
			{
				auto src_row = bitmap.RowData(bitmap.H()-dest_row_i-1);
				auto src_row_end = src_row+bpl;
				auto dest_row = bitmap.RowData(dest_row_i);
				auto dest_row_end = dest_row+bpl;
				if(src_row <= dest_row) break;
				while(src_row != src_row_end)
				{
					const uint8_t temp = *src_row;
					*src_row++ = *dest_row;
					*dest_row++ = temp;
				}
			}
		}
	}

	template <uint32_t kAlign>
	inline void ConvertBGRToRGB(const cBitmap<kAlign>& bitmap)
	{
		AKJ_ASSERT(bitmap.Comp() == 3 && bitmap.BPC() == 1);
		ForEachPixel([](pix::RGB8& px){
			const uint8_t r_tmp = px.r();
			px.r() = px.b();
			px.b() = r_tmp;
		}, bitmap.Pixels<pix::RGB8>());

	}

	template <uint32_t kAlign>
	inline void ConvertBGRAToRGBA(const cBitmap<kAlign>& bitmap)
	{
		AKJ_ASSERT(bitmap.Comp() == 4 && bitmap.BPC() == 1);
		ForEachPixel([](pix::RGBA8& px){
			const uint8_t r_tmp = px.r();
			px.r() = px.b();
			px.b() = r_tmp;
		}, bitmap.Pixels<pix::RGBA8>());
	}

	class cImageWorsener
	{
	public:
		cImageWorsener(const cAlignedBitmap& bitmap)
			: mBitmap(bitmap)
		{
			mInitParams.api_version = IW_VAL_API_VERSION;
			mInitParams.userdata = this;
			mInitParams.freefn = &DeAllocate;
			mInitParams.mallocfn = &Allocate;
			mContext = iw_create_context(&mInitParams);
			iw_image image = {};
			image.imgtype =  CompNumeToType(bitmap.Comp());
			image.bit_depth = bitmap.BPC()*8;
			image.sampletype = IW_SAMPLETYPE_UINT;
			image.width = bitmap.W();
			image.height = bitmap.H();
			image.pixels = bitmap.Data();
			image.bpr = bitmap.Stride();
			iw_set_input_image(mContext, &image);
		};

		cBitmap<1> Resample(int w, int h)
		{
			iw_image image;
			iw_set_output_canvas_size(mContext, w, h);
			iw_set_resize_alg(mContext, IW_DIMENSION_H, 
												IW_RESIZETYPE_MIX, 0.75, 4, 0);
			iw_set_resize_alg(mContext, IW_DIMENSION_V, 
												IW_RESIZETYPE_MIX, 0.75, 4, 0);
			iw_set_output_depth(mContext, mBitmap.BPC()*8);
			iw_set_output_profile(mContext, IW_PROFILE_ALWAYSLINEAR 																										| IW_PROFILE_TRANSPARENCY);
			iw_csdescr cs;
			cs.cstype = IW_CSTYPE_LINEAR;
			iw_set_input_colorspace(mContext, &cs);
			iw_set_output_colorspace(mContext, &cs);
			iw_set_allow_opt(mContext, IW_OPT_GRAYSCALE, 0);
			iw_set_allow_opt(mContext, IW_OPT_PALETTE, 0);
			iw_set_allow_opt(mContext, IW_OPT_16_TO_8, 0);
			iw_set_allow_opt(mContext, IW_OPT_STRIP_ALPHA, 0);
			iw_set_allow_opt(mContext, IW_OPT_BINARY_TRNS, 0);
			iw_process_image(mContext);
			iw_get_output_image(mContext, &image);
			
			cBitmap<1> out(image.pixels, image.width, image.height,
										 ImgTypeToComp(image.imgtype), (image.bit_depth+7)/8,
										image.bpr);
			return out;
		}

		static unsigned int ImgTypeToComp(unsigned int img_type)
		{
			if(img_type == IW_IMGTYPE_GRAY) return 1;
			else if(img_type == IW_IMGTYPE_GRAYA)  return 2;
			else if(img_type == IW_IMGTYPE_RGB)  return 3;
			else if(img_type == IW_IMGTYPE_RGBA)  return 4;
			AKJ_ASSERT_AND_THROW(!"what is going on here?");
			return 0;
		}

		static unsigned int CompNumeToType(unsigned int comp_num)
		{
			if(comp_num == 1) return IW_IMGTYPE_GRAY;
			else if(comp_num == 2) return  IW_IMGTYPE_GRAYA;
			else if(comp_num == 3) return  IW_IMGTYPE_RGB;
			else if(comp_num == 4) return  IW_IMGTYPE_RGBA;
			AKJ_ASSERT_AND_THROW(!"what is going on here?");
			return 0;
		}

		~cImageWorsener()
		{
			iw_destroy_context(mContext);
		};

		static void* Allocate(void* user,unsigned int flags, size_t num_bytes)
		{
			cImageWorsener* self = reinterpret_cast<cImageWorsener*>(user);
			void* ret = self->mBuffers.Allocate(num_bytes);
			if(flags == IW_MALLOCFLAG_ZEROMEM)
			{
				cAlignedBuffer* buf = self->mBuffers.Find(ret);
				AKJ_ASSERT_AND_THROW(buf != nullptr);
				buf->fill_value(0);
			}
			return ret;
		}

		static void DeAllocate(void* user, void* ptr)
		{
			cImageWorsener* self = reinterpret_cast<cImageWorsener*>(user);
			if(ptr != self->mBitmap.Data())
			{
				self->mBuffers.DeAllocate(ptr);
			}
		}


	private:
		cAlignedBitmap mBitmap;
		cBufferPool mBuffers;
		iw_init_params mInitParams;
		iw_context* mContext;
	};



	template <typename tPixel>
	class cBitmapExpander
	{
	public:
		cBitmapExpander(cAlignedBuffer& temp_buffer)
			:mTempBuffer(temp_buffer)
		{}

		template <class tBitmap>
		tBitmap Expanded(const tBitmap& in, uint32_t factor)
		{
			tBitmap out(mTempBuffer, in.W()*factor, in.H()*factor,
									in.Comp(), in.BPC());

			//create two interleaved bitmaps that can compose "out"
			for (size_t i = 0; i < factor ; ++i)
			{
				tBitmap out_slice(mTempBuffer.data()+i*out.Stride(), out.W(), in.H(), 
													in.Comp(), in.BPC(), out.Stride()*factor);

				auto out_pixels = out_slice.Pixels<tPixel>().begin();
			
				for(auto& pix: in.Pixels<tPixel>())
				{
					for (uint32_t count = 0; count < factor ; ++count)
					{
						*out_pixels = pix;
						++out_pixels;
					}
				}
				AKJ_ASSERT(mTempBuffer.BoundsIntact());
			}
			return out;
		}
	private:
		cAlignedBuffer& mTempBuffer;
	};
}
