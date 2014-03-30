#include "akjTextureObject.hpp"
#include "akjOGL.hpp"
#include "akjIOUtil.hpp"
#include "FancyDrawMath.hpp"
#include "Twine.hpp"
#include "Bitmap.hpp"
#include "akjMipMapper.hpp"
#include "akjBitmapOperations.hpp"


namespace akj
{
	int GetInternalFormat(pix::eFormat format)
	{
		switch (format)
		{
		case pix::kRGBA32f:
			return GL_RGBA32F;
			break;
		case pix::kRGBA8:
			return GL_RGBA8;
			break;
		case pix::kA32f:
			return GL_R32F;
			break;
		case pix::kAG32f:
			return GL_RG32F;
			break;
		default:
			break;
		}
		AKJ_THROW("Unrecognized pixel format enumeration");
		return 0;
	}
	int GetFormat(pix::eFormat format)
	{
		switch (format)
		{
		case pix::kRGBA32f:
			return GL_RGBA;
			break;
		case pix::kRGBA8:
			return GL_RGBA;
			break;
		case pix::kA32f:
			return GL_RED;
			break;
		case pix::kAG32f:
			return GL_RG;
			break;
		default:
			break;
		}
		AKJ_THROW("Unrecognized pixel format enumeration");
		return 0;
	}

	int GetType(pix::eFormat format)
	{
		switch (format)
		{
		case pix::kRGBA32f:
			return GL_FLOAT;
			break;
		case pix::kRGBA8:
			return GL_UNSIGNED_BYTE;
			break;
		case pix::kA32f:
			return GL_FLOAT;
			break;
		case pix::kAG32f:
			return GL_FLOAT;
			break;
		default:
			break;
		}
		AKJ_THROW("Unrecognized pixel format enumeration");
		return 0;
	}

static const double kLogOf2 =  0.6931471805599453;
cTextureObject::cTextureObject(cHWGraphicsContext* context, const Twine& object_name)
	:cHWGraphicsObject(context, object_name)
	,mBoundTextureUnit(-1)
{
	glGenTextures(1, &mObjectID);
	glCheckAllErrors(__FILE__, __LINE__);
}
cTextureObject::~cTextureObject()
{
	glDeleteTextures(1, &mObjectID);
}

void cTextureObject::
	CreateDepthTexture( int width, int height)
{
	GLint tex_unit_index;
	GLint currently_bound = GL_INVALID_VALUE;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currently_bound);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &tex_unit_index);
	tex_unit_index -= GL_TEXTURE0;
	glBindTexture(GL_TEXTURE_2D, mObjectID);
	if(glCheckAllErrors(__FILE__,__LINE__) > 0)
	{
		return;
	}
	mBoundTextureUnit = tex_unit_index;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	const int internal_format = GL_DEPTH_COMPONENT32F;
	const uint32_t format = GL_DEPTH_COMPONENT;
	const uint32_t pix_type = GL_FLOAT;

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
							pix_type, NULL);

	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Warn("there were errors when initializing an empty 2D texture");
	}

	glActiveTexture(GL_TEXTURE0 + tex_unit_index + 1);
}

void cTextureObject::
	CreateEmptyTexture2D( int width, int height, pix::eFormat pix_format )
{
	GLint tex_unit_index;
	GLint currently_bound = GL_INVALID_VALUE;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currently_bound);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &tex_unit_index);
	tex_unit_index -= GL_TEXTURE0;
	glBindTexture(GL_TEXTURE_2D, mObjectID);
	if(glCheckAllErrors(__FILE__,__LINE__) > 0)
	{
		return;
	}
	mBoundTextureUnit = tex_unit_index;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	const int internal_format = GetInternalFormat(pix_format);
	const uint32_t format = GetFormat(pix_format);
	const uint32_t pix_type = GetType(pix_format);

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
							pix_type, NULL);

	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Warn("there were errors when initializing an empty 2D texture");
	}

	glActiveTexture(GL_TEXTURE0 + tex_unit_index + 1);
}

void cTextureObject::Bind()
{
	glBindTexture(GL_TEXTURE_2D, mObjectID);
	if(mBoundTextureUnit != -1)
	{
		return;
	}
}

void cTextureObject::SetWrapMode( GLuint sval, GLuint tval ) const
{
	glBindTexture(GL_TEXTURE_2D, mObjectID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sval);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tval);
}

void cTextureObject::DisableInterpolation() const
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


void cTextureObject
::SetInterpMode( eInterpMode shrink_mode, eInterpMode grow_mode ) const
{
	GLuint shrink = 0;
	GLuint grow = 0;
	switch (shrink_mode)
	{
	case akj::cTextureObject::NEAREST:
		shrink = GL_NEAREST;
		break;
	case akj::cTextureObject::LINEAR:
		shrink = GL_LINEAR;
		break;
	default:
		break;
	}
	switch (grow_mode)
	{
	case akj::cTextureObject::NEAREST:
		grow = GL_NEAREST;
		break;
	case akj::cTextureObject::LINEAR:
		grow = GL_LINEAR;
		break;
	default:
		break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, grow);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, shrink);
}

void cTextureObject::CreateTexture2D(const cAlignedBitmap& image, 
																			bool is_srgb /*= false*/, 
																			int mip_levels /*= -1*/)
{
	if (image.Data())
	{
		GLint tex_unit_index;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &tex_unit_index);
		tex_unit_index -= GL_TEXTURE0;
		glBindTexture(GL_TEXTURE_2D, mObjectID);
		if (glCheckAllErrors(__FILE__, __LINE__) > 0)
		{
			return;
		}
		mBoundTextureUnit = tex_unit_index;
		int bpp = image.BPC()*image.Comp();
		AKJ_ASSERT(bpp == 3 || bpp == 4 || bpp == 1);
		GLint internal_format = bpp;
		GLint my_format = bpp == 4 ? GL_RGBA : ((bpp == 3) ? GL_RGB : GL_RED);
		if (is_srgb)
		{
			internal_format = bpp == 4 ? GL_SRGB8_ALPHA8 : ((bpp == 3) ? GL_SRGB8 : GL_R8);
		}
		else
		{
			internal_format = bpp == 4 ? GL_RGBA8 : ((bpp == 3) ? GL_RGB8 : GL_R8);
		}

		if (mip_levels < 0)
		{
			const int largest_side = image.Width() > image.Height() ? image.Width() : image.Height();
			mip_levels = static_cast<int>(floor(log(largest_side) / kLogOf2));
		}
		if (mip_levels > 0)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mip_levels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		Log::Info("Texture \"%s\". Size = %dx%d, channels = %d, mips = %d", 
			mObjectName, image.Width(), image.Height(), 
			image.BytesPerPixel(), mip_levels);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
			image.Width(), image.Height(), 0,
			my_format, GL_UNSIGNED_BYTE, image.Data());
		if (glCheckAllErrors(__FILE__, __LINE__))
		{
			Log::Warn("there were errors when initializing new 2D texture %s", mObjectName);
		}
		else
		{
			AlignedBuffer<16> mip_buffer;
			size_t buf_size = 0;
			std::vector<cAlignedBitmap> mip_bitmaps;
			mip_bitmaps.reserve(mip_levels);
			for (int i = 1; i <= mip_levels; i++)
			{
				mip_bitmaps.emplace_back(MipMapper::EmptyMipBitmap(image, i));
				buf_size += mip_bitmaps.back().Size();
			}
			mip_buffer.reset(buf_size);
			uint8_t* ptr = mip_buffer.data();
			//set the bitmap data
			for (int i = 0; i < mip_levels; i++)
			{
				mip_bitmaps.at(i).SetData(ptr);
				ptr += mip_bitmaps.at(i).Size();
			}

			MipMapper::GenerateMipLevels(image, mip_bitmaps);
			
			for (int i = 0; i < mip_levels; i++)
			{
				const cAlignedBitmap& mip = mip_bitmaps.at(i);
				//cImageWorsener worsener(image);
				//cBitmap<1> mip = worsener.Resample(mip_hope.W(), mip_hope.H());
				//mip.ExportPNG("mip_worse" + mObjectName +Twine(i)+ ".png");
				//mip_hope.ExportPNG("mip_linear"  + mObjectName + Twine(i)+".png");
				glTexImage2D(GL_TEXTURE_2D, i+1, internal_format,
					mip.Width(), mip.Height(), 0,
					my_format, GL_UNSIGNED_BYTE, mip.Data());
			}
			if (glCheckAllErrors(__FILE__, __LINE__))
			{
				Log::Warn("there were errors when initializing new 2D texture %s",
										mObjectName);
			}
			else
			{
				//Log::Info("done");
			}
		}
		glActiveTexture(GL_TEXTURE0 + tex_unit_index + 1);
	}
}

void cTextureObject::CreateCubeMap(const char* base_name, bool is_srgb /*= false*/)
{
	static const char* prefixes[6] = {
		"posx_",
		"negx_",
		"posy_",
		"negy_",
		"posz_",
		"negz_"
	};
	static const GLint locations[6] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	GLint tex_unit_index;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &tex_unit_index);
	tex_unit_index -= GL_TEXTURE0;
	glBindTexture(GL_TEXTURE_CUBE_MAP, mObjectID);
	if (glCheckAllErrors(__FILE__, __LINE__) > 0)
	{
		return;
	}
	mBoundTextureUnit = tex_unit_index;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (int i = 0; i < 6; i++)
	{
		std::string file_name(prefixes[i]);
		file_name.append(base_name);
		cAlignedBuffer buf;
		cAlignedBitmap image = LoadImageFile(Twine(file_name, base_name), buf);
		Log::Info("loading image %s ", file_name.c_str());
		int bpp = image.BytesPerPixel();
		GLint internal_format = bpp;
		GLint my_format = bpp == 4 ? GL_RGBA : GL_RGB;
		if (is_srgb)
		{
			internal_format = bpp == 4 ? GL_SRGB8_ALPHA8 : GL_SRGB8;
		}
		else
		{
			internal_format = bpp == 4 ? GL_RGBA8 : GL_RGB8;
		}
		const int largest_side = image.Width() > image.Height() ? image.Width() : image.Height();
		const int mip_levels = static_cast<int>(floor(log(largest_side) / kLogOf2));
		Log::Info("Size = %dx%d, channels = %d", image.Width(), image.Height(), image.BytesPerPixel());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mip_levels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(locations[i], 0, internal_format,
			image.Width(), image.Height(), 0,
			my_format, GL_UNSIGNED_BYTE, image.Data());
		if (glCheckAllErrors(__FILE__, __LINE__))
		{
			Log::Warn("there were errors when initializing new 2D texture %s", mObjectName);
		}
		else
		{
			AlignedBuffer<16> mip_buffer;
			size_t buf_size = 0;
			std::vector<cAlignedBitmap> mip_bitmaps;
			mip_bitmaps.reserve(mip_levels);
			for (int i = 1; i <= mip_levels; i++)
			{
				mip_bitmaps.emplace_back(MipMapper::EmptyMipBitmap(image, i));
				buf_size += mip_bitmaps.back().Size();
			}
			mip_buffer.reset(buf_size);
			uint8_t* ptr = mip_buffer.data();
			//set the bitmap data
			for (int i = 0; i < mip_levels; i++)
			{
				mip_bitmaps.at(i).SetData(ptr);
				ptr += mip_bitmaps.at(i).Size();
			}

			MipMapper::GenerateMipLevels(image, mip_bitmaps);

			for (int i = 0; i < mip_levels; i++)
			{
				const cAlignedBitmap& mip = mip_bitmaps.at(i);
				glTexImage2D(GL_TEXTURE_2D, i + 1, internal_format,
					mip.Width(), mip.Height(), 0,
					my_format, GL_UNSIGNED_BYTE, mip.Data());
			}
			if (glCheckAllErrors(__FILE__, __LINE__))
			{
				Log::Warn("there were errors when initializing new 2D texture %s", mObjectName);
			}
			else
			{
				Log::Info("done");
			}
		}
	}
	glActiveTexture(GL_TEXTURE0 + tex_unit_index + 1);
}

void cTextureObject::UnBind()
{
	//TODO: implement this
	AKJ_ASSERT(false);
}

} // namespace akj