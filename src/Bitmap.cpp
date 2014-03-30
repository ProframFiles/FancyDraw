#include "Bitmap.hpp"
#include <mutex>
#include <sstream>
#include <unordered_map>
#include "Path.hpp"
#include "turbojpeg/turbojpeg.h"
#include "png.h"
#include <memory>
#include "akjIOUtil.hpp"
#include "akjSerialization.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

namespace akj{

struct cTGAHeader{
	enum eTGABytes
	{
		kIdlength = 0,
		kColorMapType = 1,
		kDataType = 2,
		kColorMapStart = 3,
		kColorMapSize = 5,
		kColorMapDepth = 7,
		kXOrigin = 8,
		kYOrigin = 10,
		kWidth = 12,
		kHeight =14,
		kBitsPerPixel = 16,
		kImageDescriptor = 17,
		kHeaderBytes = 18
	};
	cTGAHeader(const cBitmap<1>& bitmap)	
	{
		AKJ_ASSERT_AND_THROW(bitmap.H() < 1<<16 && bitmap.W() < 1<<16);
		bytes[kIdlength] = 0;
		bytes[kColorMapType] = 0;
		// 3 -> B&W image data, 2 -> RGB(A) data
		bytes[kDataType] = bitmap.Comp()==1 ? 3 : 2;
		*reinterpret_cast<short*>(&bytes[kColorMapStart]) = 0;
		*reinterpret_cast<short*>(&bytes[kColorMapSize]) = 0;
		bytes[kColorMapDepth] = 0;
		*reinterpret_cast<unsigned short*>(&bytes[kXOrigin]) = 0;
		*reinterpret_cast<unsigned short*>(&bytes[kYOrigin]) = 0;
		*reinterpret_cast<unsigned short*>(&bytes[kWidth]) = bitmap.W();
		*reinterpret_cast<unsigned short*>(&bytes[kHeight]) = bitmap.H();
		bytes[kBitsPerPixel] = bitmap.BPP()*8;
		bytes[kImageDescriptor] = 0;
	}
	unsigned char bytes[kHeaderBytes];
};


	uint32_t PngCounter(cStringRef str_ref)
	{
		std::string str = str_ref.str();
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);
		static std::unordered_map<std::string, uint32_t> name_map;
		uint32_t count = name_map[str];
		name_map[str] += 1;
		return count;
	}

	uint32_t TgaCounter(cStringRef str_ref)
	{
		std::string str = str_ref.str();
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);
		static std::unordered_map<std::string, uint32_t> name_map;
		uint32_t count = name_map[str];
		name_map[str] += 1;
		return count;
	}

	uint32_t JpgCounter(cStringRef str_ref)
	{
		std::string str = str_ref.str();
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);
		static std::unordered_map<std::string, uint32_t> name_map;
		uint32_t count = name_map[str];
		name_map[str] += 1;
		return count;
	}


	int cBitmapWriter::WriteTGA(const cBitmap<1>& bitmap
															, const Twine& utf8_path)
	{
		cSmallVector<char,256> temp_vec;
		cSmallVector<char,256> final_vec;
		cSmallVector<char,32> number_vec;

		cStringRef input_path = utf8_path.toStringRef(temp_vec);
		cStringRef root_path = sys::path::root_path(input_path);
		cStringRef base_name = sys::path::stem(input_path);
		cStringRef extension = sys::path::extension(input_path);

		cStringRef full_name;
		uint32_t num = PngCounter(base_name);

		if(num == 0)
		{
			full_name = (root_path+ base_name + extension)
										.toNullTerminatedStringRef(final_vec);
		}
		else
		{
			full_name = (root_path+ base_name +"(" +Twine(num)+")"+ extension)
										.toNullTerminatedStringRef(final_vec);
		}
		size_t size = bitmap.H()*bitmap.BytesPerLine();
		cTGAHeader header(bitmap);

		auto fout = GetMappedOutFile(size+header.kHeaderBytes, full_name);
		memcpy(fout->getBufferStart(), header.bytes, header.kHeaderBytes);

		uint8_t* dst = fout->getBufferStart()+header.kHeaderBytes;
		if(bitmap.Comp() == 3)
		{
			for(auto& pix: bitmap.PixelsUpsideDown<pix::RGB8>())
			{
				*dst++ = pix.b();
				*dst++ = pix.g();
				*dst++ = pix.r();
			}
		}
		else if(bitmap.Comp() == 4)
		{
			for(auto& pix: bitmap.PixelsUpsideDown<pix::RGBA8>())
			{
				*dst++ = pix.b();
				*dst++ = pix.g();
				*dst++ = pix.r();
				*dst++ = pix.a();
			}
		}

		auto err = fout->commit();
		if(err)
		{
			AKJ_THROW("Problem writing " + full_name + " to disk: " + err.message());
		}
		return 1;
	}

	int cBitmapWriter::WritePNG(const cBitmap<1>& bitmap, const Twine& utf8_path)
	{
		cSmallVector<char,256> temp_vec;
		cSmallVector<char,256> final_vec;
		cSmallVector<char,32> number_vec;

		cStringRef input_path = utf8_path.toStringRef(temp_vec);
		cStringRef root_path = sys::path::parent_path(input_path);
		cStringRef base_name = sys::path::stem(input_path);
		cStringRef extension = sys::path::extension(input_path);

		cStringRef full_name;
		uint32_t num = PngCounter(base_name);

		unsigned char my_root[2] = {'.', '\0'};
		if(root_path.empty())
		{
			root_path = my_root;
		}

		
		


		if(num == 0)
		{
			sys::path::append(final_vec, root_path, base_name + extension);
		}
		else
		{
			sys::path::append(final_vec, root_path, base_name +"(" 
													+Twine(num)+")"+ extension);
		}
		full_name = cStringRef(final_vec.data(), final_vec.size());

		cAlignedBuffer buf;
		int ret = WritePNGToMem(bitmap, buf);
		if(ret < 0)
		{
			AKJ_THROW("png writing failed!");
		}
		WriteFileToDisk(cStringRef(buf.data(), buf.size()), full_name);
		return 1;
	}

	int cBitmapWriter::WriteJPEG(const cBitmap<1>& bitmap, const Twine& utf8_path)
	{
		cSmallVector<char,256> temp_vec;
		cSmallVector<char,256> final_vec;
		cSmallVector<char,32> number_vec;

		cStringRef input_path = utf8_path.toStringRef(temp_vec);
		cStringRef root_path = sys::path::parent_path(input_path);
		cStringRef base_name = sys::path::stem(input_path);
		cStringRef extension = sys::path::extension(input_path);

		cStringRef full_name;
		uint32_t num = PngCounter(base_name);

		unsigned char my_root[2] = {'.', '\0'};
		if(root_path.empty())
		{
			root_path = my_root;
		}
		if(num == 0)
		{
			sys::path::append(final_vec, root_path, base_name + extension);
		}
		else
		{
			sys::path::append(final_vec, root_path, base_name +"(" 
													+Twine(num)+")"+ extension);
		}
		full_name = cStringRef(final_vec.data(), final_vec.size());
		cAlignedBuffer buf;
		int ret = WriteJPEGToMem(bitmap, buf);
		if(ret < 0)
		{
			AKJ_THROW("jpeg writing failed!");
		}
		WriteFileToDisk(cStringRef(buf.data(), buf.size()), full_name);
		return 1;
	}


	int cBitmapWriter::WriteJPEGToMem(const cBitmap<1>& bitmap, cAlignedBuffer& buf)
	{
			AKJ_ASSERT(bitmap.BPC() == 1 
									&& (bitmap.Comp() == 3 || bitmap.Comp() == 1 || bitmap.Comp() == 4));
			cAlignedBuffer temp_buf;
			tjhandle tj = tjInitCompress();
			if(tj ==nullptr)
			{
					return -1;
			}

			const int subsampling = 0;

			unsigned long buf_size = tjBufSize(bitmap.W(), bitmap.H(), subsampling);
			temp_buf.reset(buf_size);
			unsigned char* dst = temp_buf.data();
			const int quality = 100;
			TJPF pix_format = TJPF_RGB;
			if(bitmap.Comp() ==1){
				pix_format = TJPF_GRAY;
			}
			else if(bitmap.Comp() == 4)
			{
				pix_format = TJPF_RGBA;
			}

			int ret = tjCompress2(tj, bitmap.Data(), 
									bitmap.W(), (int)bitmap.Stride(), bitmap.H(), pix_format,
									&dst, &buf_size, subsampling, quality,
									TJFLAG_ACCURATEDCT);
			if(ret < 0)
			{
				return ret;
			}
			buf.reset(buf_size);
			memcpy(buf.data(), temp_buf.data(), buf_size);

			return 1;
	}

	int PNGColorType(int comp)
	{
		switch (comp)
		{
		case 1:
			return PNG_COLOR_TYPE_GRAY;
		case 2:
			return PNG_COLOR_TYPE_GA;
		case 3:
			return PNG_COLOR_TYPE_RGB;
		case 4:
			return PNG_COLOR_TYPE_RGB_ALPHA;
		default:
			AKJ_THROW("Invalid png colortype " + Twine(comp));
		}
		return 0;
	}

	int PNGComponents(int type)
	{
		switch (type)
		{
		case PNG_COLOR_TYPE_GRAY:
			return 1;
		case PNG_COLOR_TYPE_GA:
			return 2;
		case PNG_COLOR_TYPE_RGB:
			return 3;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return 4;
		default:
			AKJ_THROW("Invalid png colortype " + Twine(type));
		}
		return 0;
	}

	int cBitmapWriter
	::WritePNGToMem(const cBitmap<1>& bitmap, cAlignedBuffer& buf)
{
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
													nullptr, nullptr, nullptr);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		typedef std::vector<unsigned char> tVec;
		std::vector<unsigned char> data_vec;
		data_vec.reserve(bitmap.Size());

		if(setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_write_struct(&png_ptr, &info_ptr);
			AKJ_THROW("png writing died");
			return -1;
		}

		png_set_write_fn(png_ptr, &data_vec,
		[](png_structp png_ptr, png_bytep data, png_size_t length)
		{
			tVec* vec = reinterpret_cast<tVec*>(png_get_io_ptr(png_ptr));
			//our write function
			for (png_size_t i = 0; i < length; i++)
			{
				vec->push_back(*data++);
			}
		},
			[](png_structp png_ptr){
			// no need to flush
		});

		png_set_IHDR(png_ptr, info_ptr, bitmap.W(), bitmap.H(), bitmap.BPC()*8,
									PNGColorType(bitmap.Comp()), PNG_INTERLACE_NONE, 
									PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);

		for (uint32_t i = 0; i < bitmap.H(); ++i)
		{
			png_write_row(png_ptr, bitmap.RowData(i)); 
		}

		png_write_end(png_ptr, info_ptr);

		buf.reset(data_vec.size());
		memcpy(buf.data(), data_vec.data(), data_vec.size());

		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 1;
}


	akj::cAlignedBitmap cBitmapReader::ReadPNGFromMem(cStringRef in,
																										cAlignedBuffer& out)
	{
		const int not_png = png_sig_cmp((uchar*)in.data(), 0, 8);
		if(not_png)
		{
			AKJ_THROW("tried to png-decode a non-png!");

		}
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
													nullptr, nullptr, nullptr);
		AKJ_ASSERT_AND_THROW(png_ptr);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		AKJ_ASSERT_AND_THROW(info_ptr);

		if(setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			AKJ_THROW("png reading died");
			return cAlignedBitmap();
		}

		png_set_read_fn(png_ptr, &in,
		[](png_structp png_ptr, png_bytep data, png_size_t length)
		{
			cStringRef* in_data 
				= reinterpret_cast<cStringRef*>(png_get_io_ptr(png_ptr));
			//our write function
			for (png_size_t i = 0; i < length; i++)
			{
				*data++ = (*in_data)[i];
			}
			*in_data = in_data->drop_front(length);
		});

		png_read_info(png_ptr, info_ptr);

		uint32_t width;
		uint32_t height;
		int bit_depth;
		int color_type;
		int interlacing;
		int compression_type;
		int filter_method;

		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
									&color_type, &interlacing, &compression_type,
									&filter_method);
		int comp = PNGComponents(color_type);
		cAlignedBitmap bitmap(out, width, height, comp, bit_depth/8);

		for (uint32_t i = 0; i < bitmap.H(); i++)
		{
			png_read_row(png_ptr, bitmap.RowData(i),NULL);
		}

		png_read_end(png_ptr, NULL);

		png_destroy_read_struct(&png_ptr, &info_ptr,NULL);
		return bitmap;
	}

	struct cBMPFile
	{
		template <class tReader>
		cBMPFile(tReader& r)
		{
			mIsValid = false;
			size_t start = r.Tell();
			r.Read(bfB);
			r.Read(bfM);
			if(bfB != 'B' || bfM != 'M')
			{
				mStatusString = "Not A valid Bitmap File";
			}
			else{
				r.Read(bfSize);
				r.Read(bfReserved1);
				r.Read(bfReserved2);
				r.Read(bfOffBits);
				r.Read(biSize);
				r.Read(biWidth);
				r.Read(biHeight);
				r.Read(biPlanes);
				r.Read(biBitCount);
				r.Read(biCompression);
				r.Read(biSizeImage);
				r.Read(biXPelsPerMeter);
				r.Read(biYPelsPerMeter);
				r.Read(biClrUsed);
				r.Read(biClrImportant);
			
				if(biBitCount < 8 || biCompression > 0)
				{
					mStatusString = "unsupported BMP features";
				}
				else{
					mBitmap = cAlignedBitmap(biWidth, biHeight, biBitCount/8, BIT_DEPTH_8);
					uint32_t sz = u32(mBitmap.Size());
					uint32_t stride = (bfSize-bfOffBits)/biHeight;
					size_t bytes = r.Tell() - start;
					mBitmap.UseAsStorage(mData);
					r.Skip(bfOffBits - bytes);
					for (int row = 0; row < biHeight ; ++row)
					{
						r.ReadBytes(mBitmap.RowData(row), stride);
					}
					mIsValid = true;
				}

			}
		}
    unsigned char	 bfB;
		unsigned char  bfM;/* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */

    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
		cAlignedBuffer mData;
		cAlignedBitmap mBitmap;
		bool mIsValid;
		std::string mStatusString;
	};



	akj::cAlignedBitmap cBitmapReader::ReadBMPFromMem(cStringRef in, cAlignedBuffer& out)
	{
		auto byte_reader = cSerialization::Reader(in);
		cBMPFile my_bmp(byte_reader);
		if(my_bmp.mIsValid)
		{
			out = std::move(my_bmp.mData);
			FlipVertically(my_bmp.mBitmap);
			if(my_bmp.mBitmap.Comp() ==3){
				ConvertBGRToRGB(my_bmp.mBitmap);
			}
			else if(my_bmp.mBitmap.Comp() ==4){
				ConvertBGRAToRGBA(my_bmp.mBitmap);
			}

			return my_bmp.mBitmap;
		}
		AKJ_THROW("Error reading bitmap: " + my_bmp.mStatusString);
	}

}
