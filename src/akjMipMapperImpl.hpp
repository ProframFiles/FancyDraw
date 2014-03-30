#pragma once
#include "Bitmap.hpp"
#include <stdint.h>
#include "FancyDrawMath.hpp"

namespace akj{

	template <typename tScalar>
	const tScalar* AddBytesToPtr(const tScalar* ptr, size_t num)
	{
		return reinterpret_cast<const tScalar*>(
						reinterpret_cast<const uint8_t*>(ptr)+num);
	}

	template <typename tScalar>
	tScalar* AddBytesToPtr(tScalar* ptr, size_t num)
	{
		return reinterpret_cast<tScalar*>(
						reinterpret_cast<uint8_t*>(ptr)+num);
	}

	template <typename tScalar>
	intptr_t ByteDiff(const tScalar* lhs, const tScalar* rhs)
	{
		return reinterpret_cast<const char*>(lhs)
						- reinterpret_cast<const char*>(rhs);
	}

	////////////////////////////////////////////////////////////////////////// 
	// Structures to use for pixel averaging via pointer aliasing
	// This was the fastest way I found to do the fallback/unaligned case
	// (Where SIMD wasn't possible)
	//////////////////////////////////////////////////////////////////////////
	struct cFourPixelsRGB
	{
		uint8_t r0, g0, b0, r1, g1, b1, r2, g2, b2, r3, g3, b3; 
	};
	// ensures that cFourPixels is of size 12 bytes, and packs to 12 bytes, compile error otherwise
	
	struct cTwoPixelsRGB
	{
		uint8_t r0, g0, b0, r1, g1, b1;
		void set_by_mean_values(const cFourPixelsRGB* top_row, 
														const cFourPixelsRGB* bottom_row){
			r0 = (top_row->r0 + top_row->r1 + bottom_row->r0 
					+ static_cast<uint32_t>(bottom_row->r1)) >> 2;
			g0 = (top_row->g0 + top_row->g1 + bottom_row->g0 
					+ static_cast<uint32_t>(bottom_row->g1)) >> 2;
			b0 = (top_row->b0 + top_row->b1 + bottom_row->b0 
					+ static_cast<uint32_t>(bottom_row->b1)) >> 2;

			r1 = (top_row->r2 + top_row->r3 + bottom_row->r2 
					+ static_cast<uint32_t>(bottom_row->r3)) >> 2;
			g1 = (top_row->g2 + top_row->g3 + bottom_row->g2 
					+ static_cast<uint32_t>(bottom_row->g3)) >> 2;
			b1 = (top_row->b2 + top_row->b3 + bottom_row->b2 
					+ static_cast<uint32_t>(bottom_row->b3)) >> 2;
		}
	};
	
	struct cOnePixelRGB
	{
		uint8_t r0, g0, b0;
		void set_by_mean_values(const cTwoPixelsRGB* top_row, 
														const cTwoPixelsRGB* bottom_row){
			r0 = (top_row->r0 + top_row->r1 + bottom_row->r0 
					+ static_cast<uint32_t>(bottom_row->r1)) >> 2;
			g0 = (top_row->g0 + top_row->g1 + bottom_row->g0 
					+ static_cast<uint32_t>(bottom_row->g1)) >> 2;
			b0 = (top_row->b0 + top_row->b1 + bottom_row->b0 
					+ static_cast<uint32_t>(bottom_row->b1)) >> 2;
		}
		void set_by_mean_values(const cOnePixelRGB* top_row, 
														const cOnePixelRGB* bottom_row){
			r0 = (top_row->r0 + static_cast<uint32_t>(bottom_row->r0)) >> 1;
			g0 = (top_row->g0 + static_cast<uint32_t>(bottom_row->g0)) >> 1;
			b0 = (top_row->b0 + static_cast<uint32_t>(bottom_row->b0)) >> 1;
		}
	};
	

	struct cFourPixelsRG
	{
		uint8_t r0, g0, r1, g1, r2, g2, r3, g3; 
	};
	

	struct cTwoPixelsRG
	{
		uint8_t r0, g0, r1, g1;
		void set_by_mean_values(const cFourPixelsRG* top_row, 
														const cFourPixelsRG* bottom_row){
			r0 = (top_row->r0 + top_row->r1 + bottom_row->r0 
						+ static_cast<uint32_t>(bottom_row->r1)) >> 2;
			g0 = (top_row->g0 + top_row->g1 + bottom_row->g0 
						+ static_cast<uint32_t>(bottom_row->g1)) >> 2;

			r1 = (top_row->r2 + top_row->r3 + bottom_row->r2 
						+ static_cast<uint32_t>(bottom_row->r3)) >> 2;
			g1 = (top_row->g2 + top_row->g3 + bottom_row->g2 
						+ static_cast<uint32_t>(bottom_row->g3)) >> 2;
		}
	};
	
	struct cOnePixelRG
	{
		uint8_t r0, g0;
		void set_by_mean_values(const cTwoPixelsRG* top_row, 
														const cTwoPixelsRG* bottom_row){
			r0 = (top_row->r0 + top_row->r1 + bottom_row->r0 
					+ static_cast<uint32_t>(bottom_row->r1)) >> 2;
			g0 = (top_row->g0 + top_row->g1 + bottom_row->g0 
					+ static_cast<uint32_t>(bottom_row->g1)) >> 2;
		}
		void set_by_mean_values(const cOnePixelRG* top_row, 
														const cOnePixelRG* bottom_row){
			r0 = (top_row->r0 + static_cast<uint32_t>(bottom_row->r0)) >> 1;
			g0 = (top_row->g0 + static_cast<uint32_t>(bottom_row->g0)) >> 1;
		}
	};

	// check structure sizes and packing
	AKJ_SIZE_CHECK(cOnePixelRG[2],4);
	AKJ_SIZE_CHECK(cTwoPixelsRG[2],8);
	AKJ_SIZE_CHECK(cFourPixelsRG[2],16);
	
	AKJ_SIZE_CHECK(cOnePixelRGB[2],6);
	AKJ_SIZE_CHECK(cTwoPixelsRGB[2],12);
	AKJ_SIZE_CHECK(cFourPixelsRGB[2],24);
	
	
	//////////////////////////////////////////////////////////////////////////
	// End extra structs
	//////////////////////////////////////////////////////////////////////////

	namespace MipUtils{
		// get the size of some dimension, halved level times & rounded properly
		inline uint32_t GetScaledDimension(uint32_t val, uint32_t level)
		{
			const uint32_t scale_m_one = (1 << level) - 1;
			return (val+scale_m_one) >> level;
		}
	}

	template <uint32_t kComponents, typename tScalar>
	class RowReducer{ 
		static void Reduce(const tScalar* top_row,const tScalar* bottom_row, tScalar* output_buffer, uint32_t in_pixels){
			// general case not implemented
			assert(false);
		}
	};

	template <>
	struct RowReducer<1, uint8_t>{
		static void Reduce( const uint8_t* top_row,const uint8_t* bottom_row, uint8_t* output_buffer, uint32_t in_pixels )
		{
			uint32_t out_pixels = MipUtils::GetScaledDimension(in_pixels, 1);
			for (uint32_t col = 0, out_col=0; col < in_pixels; col+=2, out_col++)
			{
				uint32_t sum = top_row[col] + top_row[(col+1)] + bottom_row[col] + bottom_row[(col+1)];
				output_buffer[out_col] = static_cast<uint8_t> (sum >> 2);	
			}
			if (out_pixels*2 > in_pixels){
				output_buffer[out_pixels-1] = static_cast<uint8_t>((top_row[in_pixels-1] + static_cast<uint32_t>(bottom_row[in_pixels-1])) >> 1);
			}
		}
	};

	template <>
	struct RowReducer<2, uint8_t>{
		static void Reduce( const uint8_t* top_row,const uint8_t* bottom_row, uint8_t* output_buffer, uint32_t in_pixels )
		{

			// struct aliasing: blocks of 4
			const size_t num_structs = in_pixels/4;

			const cFourPixelsRG* top_ptr = reinterpret_cast<const cFourPixelsRG*>(top_row);
			const cFourPixelsRG* bottom_ptr = reinterpret_cast<const cFourPixelsRG*>(bottom_row);
			cTwoPixelsRG* out_ptr = reinterpret_cast<cTwoPixelsRG*>(output_buffer);

			for (uint32_t in = 0, out = 0; in < num_structs; in += 1, out ++)
			{
				(out_ptr + out)->set_by_mean_values(top_ptr+in,bottom_ptr+in);
			}

			// then do the trailing values
			const uint32_t trailing_values = ((in_pixels) % 4);
			cOnePixelRG* trailing_out_ptr = reinterpret_cast<cOnePixelRG*>(output_buffer+num_structs*sizeof(cTwoPixelsRG));
			top_ptr += num_structs;
			bottom_ptr += num_structs;
			switch (trailing_values)
			{
			case 1:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cOnePixelRG*>(top_ptr), reinterpret_cast<const cOnePixelRG*>(bottom_ptr) );
				break;
			case 2:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cTwoPixelsRG*>(top_ptr), reinterpret_cast<const cTwoPixelsRG*>(bottom_ptr) );
				break;
			case 3:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cTwoPixelsRG*>(top_ptr), reinterpret_cast<const cTwoPixelsRG*>(bottom_ptr) );
				(trailing_out_ptr+1)->set_by_mean_values(reinterpret_cast<const cOnePixelRG*>(top_ptr)+2, reinterpret_cast<const cOnePixelRG*>(bottom_ptr)+2 );
				break;
			case 0:
			default:
				break;
			}
		}
	};

	template <>
	struct RowReducer<3, uint8_t>{
		static void Reduce( const uint8_t* top_row,const uint8_t* bottom_row, uint8_t* output_buffer, uint32_t in_pixels )
		{

			const size_t num_structs = (in_pixels)/4;
			const cFourPixelsRGB* top_ptr = reinterpret_cast<const cFourPixelsRGB*>(top_row);
			const cFourPixelsRGB* bottom_ptr = reinterpret_cast<const cFourPixelsRGB*>(bottom_row);
			cTwoPixelsRGB* out_ptr = reinterpret_cast<cTwoPixelsRGB*>(output_buffer);


			for (uint32_t in = 0, out = 0; in < num_structs; in += 1, out ++)
			{
				(out_ptr + out)->set_by_mean_values(top_ptr+in,bottom_ptr+in);
			}

			// then do the trailing values
			const uint32_t trailing_values = ((in_pixels) % 4);
			cOnePixelRGB* trailing_out_ptr = reinterpret_cast<cOnePixelRGB*>(output_buffer+num_structs*6);
			top_ptr += num_structs;
			bottom_ptr += num_structs;
			switch (trailing_values)
			{
			case 1:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cOnePixelRGB*>(top_ptr), reinterpret_cast<const cOnePixelRGB*>(bottom_ptr) );
				break;
			case 2:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cTwoPixelsRGB*>(top_ptr), reinterpret_cast<const cTwoPixelsRGB*>(bottom_ptr) );
				break;
			case 3:
				trailing_out_ptr->set_by_mean_values(reinterpret_cast<const cTwoPixelsRGB*>(top_ptr), reinterpret_cast<const cTwoPixelsRGB*>(bottom_ptr) );
				(trailing_out_ptr+1)->set_by_mean_values(reinterpret_cast<const cOnePixelRGB*>(top_ptr)+2, reinterpret_cast<const cOnePixelRGB*>(bottom_ptr)+2 );
				break;
			case 0:
			default:
				break;
			}
		}
	};

	struct float4
	{
		float4(float a, float b, float c, float d)
			:x(a), y(b), z(c), w(d)
		{}
		float x, y, z, w;
	};

	struct float2
	{
		float2(float a, float b)
			:x(a), y(b)
		{}
		float x, y;
	};

	inline float4 operator*(const float4& rhs, float factor)
	{
		return float4(factor*rhs.x, factor*rhs.y, factor*rhs.z, factor*rhs.w);
	}
	
	inline float4 operator+(const float4& lhs, const float4& rhs)
	{
		return float4(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w);
	}
	inline float2 operator*(const float2& rhs, float factor)
	{
		return float2(factor*rhs.x, factor*rhs.y);
	}
	
	inline float2 operator+(const float2& lhs, const float2& rhs)
	{
		return float2(lhs.x+rhs.x, lhs.y+rhs.y);
	}



	template <>
	struct RowReducer<4, float>{
		static void Reduce( const float* top_row_in,const float* bottom_row_in,
												 float* output_buffer_in, uint32_t in_pixels )
		{
			const float4* top_row 
				= reinterpret_cast<const float4*>(top_row_in);
			const float4* bottom_row 
				= reinterpret_cast<const float4*>(bottom_row_in);
			float4* output_buffer 
				= reinterpret_cast<float4*>(output_buffer_in);

			// one short if in_pixels is odd
			const uint32_t out_pixels = in_pixels >> 1;
			for(uint32_t i = 0; i < out_pixels; ++i)
			{
				const uint32_t i2 = i*2;
				output_buffer[i] 
					= (top_row[i2]+top_row[i2+1]+bottom_row[i2]+bottom_row[i2+1])*0.25f;
			}
			// and the trailing pixel
			if(in_pixels & 1)
			{
				output_buffer[out_pixels] 
					= (top_row[in_pixels-1] + bottom_row[in_pixels-1])*0.5f;
			}
		}
	};

	template <>
	struct RowReducer<2, float>{
		static void Reduce( const float* top_row_in,const float* bottom_row_in,
												 float* output_buffer_in, uint32_t in_pixels )
		{
			const float2* top_row 
				= reinterpret_cast<const float2*>(top_row_in);
			const float2* bottom_row 
				= reinterpret_cast<const float2*>(bottom_row_in);
			float2* output_buffer 
				= reinterpret_cast<float2*>(output_buffer_in);

			// one short if in_pixels is odd
			const uint32_t out_pixels = in_pixels >> 1;
			for(uint32_t i = 0; i < out_pixels; ++i)
			{
				const uint32_t i2 = i*2;
				output_buffer[i] 
					= (top_row[i2]+top_row[i2+1]+bottom_row[i2]+bottom_row[i2+1])*0.25f;
			}
			// and the trailing pixel
			if(in_pixels & 1)
			{
				output_buffer[out_pixels] 
					= (top_row[in_pixels-1] + bottom_row[in_pixels-1])*0.5f;
			}
		}
	};

	template <>
	struct RowReducer<1, float>{
		static void Reduce( const float* top_row,const float* bottom_row,
												 float* output_buffer, uint32_t in_pixels )
		{
			// one short if in_pixels is odd
			const uint32_t out_pixels = in_pixels >> 1;
			for(uint32_t i = 0; i < out_pixels; ++i)
			{
				const uint32_t i2 = i*2;
				output_buffer[i] 
					= (top_row[i2]+top_row[i2+1]+bottom_row[i2]+bottom_row[i2+1])*0.25f;
			}
			// do the trailing pixel
			if(in_pixels & 1)
			{
				output_buffer[out_pixels] 
					= (top_row[in_pixels-1]+ bottom_row[in_pixels-1])*0.5f;
			}
		}
	};

	template <>
	struct RowReducer<4, uint8_t>{
		static void Reduce( const uint8_t* top_row,const uint8_t* bottom_row, uint8_t* output_buffer, uint32_t in_pixels )
		{
			const size_t num_int_aligned = (in_pixels)/2;
			const uint32_t* top_ptr = reinterpret_cast<const uint32_t*>(top_row);
			const uint32_t* bottom_ptr = reinterpret_cast<const uint32_t*>(bottom_row);
			uint32_t* out_ptr = reinterpret_cast<uint32_t*>(output_buffer);

			// then the less large 8 byte blocks
			for (uint32_t i = 0; i < num_int_aligned; ++i)
			{
				const uint32_t source_left_bottom = bottom_ptr[i+i];
				const uint32_t source_left_top = top_ptr[i+i];
				const uint32_t source_right_bottom = bottom_ptr[i+i+1];
				const uint32_t source_right_top = top_ptr[i+i+1];
				const uint32_t c0 = ( ((source_left_top & 0xFF) + (source_left_bottom & 0xFF) +
					(source_right_bottom & 0xFF)+(source_right_top & 0xFF) + 3) >> 2) & 0xFF;
				const uint32_t c1 = ( ((source_left_top & 0xFF00) + (source_left_bottom & 0xFF00) +
					(source_right_bottom & 0xFF00)+(source_right_top & 0xFF00)  +0x300) >> 2) & 0xFF00;
				const uint32_t c2 = ( ((source_left_top & 0xFF0000) + (source_left_bottom & 0xFF0000) +
					(source_right_bottom & 0xFF0000)+(source_right_top & 0xFF0000)  +0x30000) >> 2) & 0xFF0000;
				const uint32_t c3 = static_cast<uint32_t>( ( static_cast<uint64_t>(source_left_top & 0xFF000000) +  static_cast<uint64_t>(source_left_bottom & 0xFF000000) +
					static_cast<uint64_t>(source_right_bottom & 0xFF000000)+ static_cast<uint64_t>(source_right_top & 0xFF000000)  +0x3000000) >> 2) & 0xFF000000;
				out_ptr[i]= c0 | c1 | c2 | c3;
			}

			// then do the trailing values, clamping as needed
			const uint32_t trailing_values = ((in_pixels) & 0x1);
			if(trailing_values > 0){
				uint32_t* trailing_out_ptr = reinterpret_cast<uint32_t*>(output_buffer)+in_pixels/2;
				top_ptr += num_int_aligned*2;
				bottom_ptr += num_int_aligned*2;
				const uint32_t source_left_bottom = bottom_ptr[0];
				const uint32_t source_left_top = top_ptr[0];
				const uint32_t c0 = (( (source_left_top&0xFF) + (source_left_bottom&0xFF) + 1) >> 1) & 0xFF;
				const uint32_t c1 = ( ((source_left_top & 0xFF00) + (source_left_bottom & 0xFF00) +0x100) >> 1) & 0xFF00;
				const uint32_t c2 = ( ((source_left_top & 0xFF0000) + (source_left_bottom & 0xFF0000)+0x10000 ) >> 1) & 0xFF0000;
				const uint32_t c3 = static_cast<uint32_t>( (static_cast<uint64_t>(source_left_top & 0xFF000000) + static_cast<uint64_t>(source_left_bottom & 0xFF000000) +0x1000000) >> 1) & 0xFF000000;
				*trailing_out_ptr = c0 | c1 | c2 | c3;
			}
		}
	};



	template<uint32_t kComponents, typename tScalar>
	class MipMapperImpl{
	public:
		// generate all desired mip maps. output stride is determined by output buffer size
		static void GenerateMipLevels( const cAlignedBitmap& bitmap, tScalar** p_buf_array, tScalar** p_end_array, uint32_t min_level, uint32_t max_level )
		{
			if(bitmap.BPC() != sizeof(tScalar)){
				assert(false);
				return;
			}

			//Do the first level as a special case, then the remaining levels in a loop

			// do a simple copy into the first level
			if(min_level == 0){
				const size_t orig_stride = bitmap.Stride();
				const size_t dest_stride 
					= ByteDiff(p_end_array[0], p_buf_array[0])/bitmap.H();
				const uint32_t dest_width = bitmap.W();
				const size_t bytes_per_line = bitmap.BPC();
				const tScalar* src_data = bitmap.Data().As<tScalar>();

				for (uint32_t row = 0; row < bitmap.H(); row++)
				{
					const tScalar* in_data_row = AddBytesToPtr(src_data, row*orig_stride);
					tScalar* my_data_row = AddBytesToPtr(p_buf_array[0],row*dest_stride);
					memcpy(my_data_row, in_data_row, bytes_per_line);
				}
			}
			// do a reduction into the first level
			else if(min_level == 1){
				const uint32_t h_out = MipUtils::GetScaledDimension(bitmap.H(), 1);
				const size_t output_stride 
					= ByteDiff(p_end_array[0], p_buf_array[0])/h_out;
				GenerateMipData(bitmap.Data().As<tScalar>(), p_buf_array[0],
						bitmap.W(), bitmap.H(), bitmap.Stride(), output_stride);
			}
			// do something more complex into the first level...
			else if(min_level > 1){
				ReduceDestructively(bitmap, p_buf_array[0], p_end_array[0], min_level);
			}
			
			// at this point p_buf_array should be populated with valid data
			// so do the rest here
			const int iterations = static_cast<int>(max_level) - min_level;
			for (int i = 0; i < iterations; i++)
			{
				const uint32_t src_level = i+min_level;
				const uint32_t dst_level = i+min_level+1;

				const tScalar* src_buffer = p_buf_array[i];
				tScalar* dst_buffer = p_buf_array[i+1];
				
				const uint32_t w = MipUtils::GetScaledDimension(bitmap.W(), src_level);
				const uint32_t h = MipUtils::GetScaledDimension(bitmap.H(), src_level);
				const uint32_t h_out 
					= MipUtils::GetScaledDimension(bitmap.H(), dst_level);
				const size_t src_stride 
					= ByteDiff(p_end_array[i], p_buf_array[i])/h;
				const size_t dst_stride 
					= ByteDiff(p_end_array[i+1], p_buf_array[i+1])/h_out;

				GenerateMipData(src_buffer, dst_buffer, w, h, src_stride, dst_stride);
			}
		}
		// generate the mip-map for one level below the input data
		static void GenerateMipData( const tScalar* image_buffer, tScalar* output_buffer, uint32_t width, uint32_t height, size_t in_stride, size_t out_stride )
		{
			const int output_width = MipUtils::GetScaledDimension(width, 1);
			const size_t output_stride = out_stride;
			const uint32_t virtual_height = 2*MipUtils::GetScaledDimension(height, 1);
			for (uint32_t i = 0; i < virtual_height; i+=2)
			{
				const tScalar* top_row = AddBytesToPtr(image_buffer, in_stride*i);
				const tScalar* bottom_row 
					= i+1 < height ? AddBytesToPtr(top_row, in_stride) : top_row;
				RowReducer<kComponents, tScalar>
					::Reduce(top_row, bottom_row, output_buffer, width);
				output_buffer = AddBytesToPtr(output_buffer, output_stride);
			}
		}
	

	private:
		//Transform the image in-place to min_level-1, then place the last result 
		// @ min_level into p_buf 
		static void ReduceDestructively(cAlignedBitmap bitmap, tScalar* p_buf, 
																		tScalar* p_end, uint32_t min_level)
		{
			assert( ((ByteDiff(p_end, p_buf)
							/MipUtils::GetScaledDimension(bitmap.H(), min_level)) &15) == 0 );

			const uint32_t level_1_width = MipUtils::GetScaledDimension(bitmap.W(),1);
			size_t src_stride =  bitmap.Stride();
			const size_t level_1_stride = (level_1_width*kComponents+15) & ~(15);
			// we would like to create one line at min_level with each buffer
			const uint32_t buffer_lines = 1 << (min_level);
			int current_level = 0;
			uint32_t source_w = MipUtils::GetScaledDimension(bitmap.W(), 0);
			uint32_t source_h = MipUtils::GetScaledDimension(bitmap.H(), 0);

			for  (uint32_t i = 0; i < min_level; i++)
			{
				const uint32_t dest_w = MipUtils::GetScaledDimension(bitmap.W(), 1);
				const uint32_t dest_h = MipUtils::GetScaledDimension(bitmap.H(), 1);
				const size_t dst_stride 
					= (sizeof(tScalar)*dest_w*kComponents+15) & ~(15);
				tScalar* out_buf 
					= (i == min_level-1) ? p_buf: bitmap.Data().As<tScalar>();
				tScalar* in_buf = bitmap.Data().As<tScalar>();
				GenerateMipData(in_buf, out_buf, source_w, source_h, 
												src_stride, dst_stride);
				bitmap = cAlignedBitmap(reinterpret_cast<uint8_t*>(out_buf), dest_w,
																	dest_h, bitmap.Comp(), bitmap.BPC());
				src_stride = dst_stride;
				source_w = dest_w;
				source_h = dest_h;
			}
		}
		
		
	};
} // namespace Img




