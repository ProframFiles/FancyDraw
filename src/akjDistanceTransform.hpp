#pragma once
#include "Bitmap.hpp"
#include "akjAlignedBuffer.hpp"
#include "FancyDrawMath.hpp"
#include "FatalError.hpp"
#include "akjLog.hpp"
#include "akjPixelFormats.hpp"


namespace akj
{
	class EuclideanDistanceTransform
	{
		struct xy
		{
			short x;
			short y;
		};
	public:
		EuclideanDistanceTransform(cAlignedBitmap source)
			: mSource(source)
		{
			uint32_t max_line_pix = GreaterOf(source.Width(), source.Height()) + 1;

			// allocate some buffers to be used in the 1D transform of each row
			mV.reset((max_line_pix)*sizeof(int));
			mZ.reset(sizeof(float)*max_line_pix);
			mLineBufferSrc.reset(sizeof(float)*max_line_pix);
			mLineBufferSrcIndex.reset(sizeof(float)*max_line_pix);
			mLineBufferDest.reset(sizeof(float)*max_line_pix);
			mInverseTable.reset(sizeof(float)*max_line_pix);
			mLineBufferDest.fill_value(0);
			mLineBufferSrc.fill_value(0);
			mInverseTable.fill_value(0);
			PopulateInverseTable(max_line_pix-1);
		}

		// pixels greater than threshold are considered "lit"
		cAlignedBitmap RunTransform(float scale = 10.f, uint8_t threshold = 127)
		{
			const float scale_sqr = scale*scale;
			return RunTransformImpl(scale, [threshold, scale_sqr](const uint8_t src){
					return src > threshold ? scale_sqr : 0.0f;
			});
		}

		//pixels less than or equal to threshold are considered "lit"
		cAlignedBitmap 
			RunInverseTransform(float scale = 10.f,  uint8_t threshold = 127)
		{
			const float scale_sqr = scale*scale;
			return RunTransformImpl(scale, [threshold, scale_sqr](const uint8_t src){
					return src <= threshold ? scale_sqr : 0.0f;
			});
		}

		cAlignedBitmap GetDistances()
		{
			cAlignedBitmap dest_indices(mSource.W(), mSource.H(), 2, BIT_DEPTH_16);
			dest_indices.SetData(mDestStorage.data()+dest_indices.Size());
			return dest_indices;
		}

		static void SelfTest()
		{
			cAlignedBuffer buf;
			cAlignedBitmap bitmap(buf, 16, 16, 1, BIT_DEPTH_8);
			bitmap.Clear(255);
			*bitmap.PixelData(4, 0) = 0;
			*bitmap.PixelData(8, 8) = 0;
			EuclideanDistanceTransform edt(bitmap);
			cAlignedBitmap result = edt.RunTransform(255);
			auto out = bitmap.Pixels<pix::A8>().begin();
			for(auto& edt_pixel: result.Pixels<pix::A32f>())
			{
				(*out).a() = static_cast<uint8_t>(255.9f*edt_pixel.a());
				++out;
			}
			bitmap.ExportPNG("EDTSelfTest.png");

			bitmap =  cAlignedBitmap(buf, 16, 16, 3, BIT_DEPTH_8);

			auto distances = edt.GetDistances();

			auto out2 = bitmap.Pixels<pix::RGB8>().begin();
			for(auto& edt_pixel: distances.Pixels<pix::RG16s>())
			{
				(*out2).r() = 128+static_cast<int8_t>(edt_pixel.r());
				(*out2).g() = 128+static_cast<int8_t>(edt_pixel.g());
				(*out2).b() = 128;
				++out2;
			}

			bitmap.ExportPNG("EDTDirectionTest.png");

		}

		private:

		template <class tFunctor>
		cAlignedBitmap RunTransformImpl(float scale,const tFunctor& func)
		{
			cAlignedBitmap dest_image(mSource.W(), mSource.H(), 1, BIT_DEPTH_32);
			cAlignedBitmap dest_indices(mSource.W(), mSource.H(), 1, BIT_DEPTH_32);
			mDestStorage.reset(dest_image.Size()+dest_indices.Size());
			mDestStorage.fill_value(0);
			dest_image.SetData(mDestStorage.data());
			dest_indices.SetData(mDestStorage.data()+dest_image.Size());

			const float scale_sqr = scale*scale;
			const float inv_scale = 1.0f/scale;

			for (uint32_t column = 0; column < mSource.Width(); column++)
			{
				const size_t source_stride = mSource.Stride();
				uint8_t* src = mSource.PixelData(column, 0);
				float* const temp_source = mLineBufferSrc.ptr<float>();
				xy* const temp_source_i = mLineBufferSrcIndex.ptr<xy>();


				//transpose and convert one row to floating point
				// scale is effectively the "height" of the lightest areas
				// in the image. the edt fringe always has a slope of 1, and
				// so scale effectively sets the limit on the reach of the fringe
				// (it can't be more than the max "height")
				for (uint32_t row = 0; row < mSource.Height(); 
						row++, src += source_stride)
				{
					temp_source_i[row].x = column;
					temp_source_i[row].y = row;
					temp_source[row] = func(*src);
				}

				float* const dest = 
					reinterpret_cast<float*>(dest_image.PixelData(column, 0));
				xy* const dest_i = 
					reinterpret_cast<xy*>(dest_indices.PixelData(column, 0));
				Transform1Row(temp_source, temp_source_i,
											dest, dest_i, mSource.Height(), 
											dest_image.Stride()/sizeof(float));
			}
			
			for (uint32_t row = 0; row < mSource.Height() ; ++row)
			{
				// Transform1row isn't set up to do an in-place transform
				// so copy the data into a buffer, then transform it back onto itself
				memcpy(mLineBufferSrc.data(), dest_image.RowData(row),
								dest_image.Stride());
				memcpy(mLineBufferSrcIndex.data(), dest_indices.RowData(row),
								dest_indices.Stride());
				Transform1Row(mLineBufferSrc.ptr<float>(),
											mLineBufferSrcIndex.ptr<xy>(),
					reinterpret_cast<float*>(dest_image.RowData(row)),
					reinterpret_cast<xy*>(dest_indices.RowData(row)),
					dest_image.Width());
			}


			CoordToDistance(dest_indices, dest_image, scale_sqr);

			dest_image.ForEachPixel<float>([inv_scale](float& src){
				src = std::sqrt(src);
			});
			return dest_image;
		}

	
		void Transform1Row(const float* src_ptr, const xy* src_i,
												float* dest_ptr, xy* dest_i,
												int n, size_t dest_stride = 1)
		{
			uint32_t dest_pix_stride = static_cast<uint32_t>(dest_stride);
			const float inf = 2e16f;
			const float* const inverse_table = mInverseTable.ptr<float>();
			const float* const src = reinterpret_cast<const float*>(src_ptr);
			float* const z = mZ.ptr<float>();
			int* const v = mV.ptr<int>();
			
			v[0] = 0;
			z[0] = -inf;
			z[1] = inf;
			dest_ptr[0] = src[0];
			dest_i[0] = src_i[0];
			int k = 0;

			for (int q = 1; q < n; q++)
			{
				float s;
				const float fq = src[q];
				const float qsqr = static_cast<float>(q*q);
				k++;
				do
				{
					k--;
					const int p = v[k];
					s = (fq + qsqr - src[p] - p*p)*inverse_table[q - p];
				} while (s <= z[k]);
				k++;
				v[k] = q;
				z[k] = s;
				z[k + 1] = inf;
			}
			k = 0;
			uint32_t dest_index = 0;
			for (int q = 0; q < n; q++,dest_index+=dest_pix_stride)
			{
				while (z[k + 1] < q)
				{
					k++;
				}
				const int p = v[k];
				dest_ptr[dest_index] = (q-p)*(q - p) + src[p];
				dest_i[dest_index] = src_i[p];
			}
		}
	
		void PopulateInverseTable(uint32_t maxval)
		{
			float* inverse_table = mInverseTable.ptr<float>();
			AKJ_ASSERT(maxval < mInverseTable.size() / sizeof(float));
			for (uint32_t i = 1; i <= maxval ; ++i)
			{
				inverse_table[i] = static_cast<float>(0.5 / i);
			}
		}

		bool CoordToDistance(const cAlignedBitmap& dest_indices,
												const cAlignedBitmap& dest_image, float max_val)
		{
			for(int y = 0; y < static_cast<int>(dest_indices.H()); ++y)
			{
				xy* line = reinterpret_cast<xy*>(dest_indices.RowData(y));
				float* line_f = reinterpret_cast<float*>(dest_image.RowData(y));
				for(int x = 0; x < static_cast<int>(dest_indices.W()); ++x)
				{
					line[x].x -= static_cast<short>(x);
					line[x].y -= static_cast<short>(y);
					if( line_f[x] != max_val)
					{
						AKJ_ASSERT(line[x].x*line[x].x + line[x].y*line[x].y == line_f[x]);
					}
				}
			}
			return true;
		}
		

		cAlignedBitmap mSource;
		AlignedBuffer<16> mV;
		AlignedBuffer<16> mZ;
		AlignedBuffer<16> mLineBufferSrc;
		AlignedBuffer<16> mLineBufferDest;
		AlignedBuffer<16> mLineBufferSrcIndex;

		AlignedBuffer<16> mInverseTable;
		AlignedBuffer<16> mDestStorage;
	};
}
