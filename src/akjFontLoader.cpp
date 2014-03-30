#include "akjFontLoader.hpp"
#include "Twine.hpp"
#include "akjFreeTypeFace.hpp"
#include "BinPacker.hpp"
#include "akjDistanceFieldFont.hpp"
#include "akjDistanceTransform.hpp"
#include "akjIOUtil.hpp"
#include "resources/akjStaticResources.hpp"
#include "Path.hpp"
#include "akjWorkerPool.hpp"
#include "akjTexAtlasFont.hpp"

namespace akj
{
	//////////////////////////////////////////////////////////////////////////
	// Simple struct for tying two classes together for serialization
	//////////////////////////////////////////////////////////////////////////
	struct cDFFFile
	{
		cDFFFile(const cDistanceFieldFont& dff_in, size_t face_size_in)
			: face_size(face_size_in)
			,	face(dff_in.Face())
			, dff(dff_in)
		{}

		template <class tWriter>
		void Serialize(tWriter& writer) const
		{
			writer.WriteSize(face_size);
			face.Serialize(writer);
			dff.Serialize(writer);
		}

		size_t face_size;
		const FreeTypeFace& face;
		const cDistanceFieldFont& dff;
	};


	void DrawBlackRect(const cAlignedBitmap& bm, int x, int y, int w, int h)
	{
		uint8_t* ptr = bm.PixelData(x, y);
		for (int i = 0; i < w; ++i)
		{
			*ptr++ = 0;
		}
		ptr = bm.PixelData(x, y + h);
		for (int i = 0; i < w; ++i)
		{
			*ptr++ = 0;
		}
		ptr = bm.PixelData(x, y + 1);
		int stride = static_cast<int>(bm.Stride());
		for (int i = 0; i < h - 1; ++i)
		{
			ptr[0] = 0;
			ptr[w - 1] = 0;
			ptr += stride;
		}
	}

	cFontLoader::cFontLoader(cWorkerPool& worker_pool)
		:mWorkerPool(worker_pool)
		,mFaceHandles(new tFaceMap)
		,mDistanceFaceHandles(new tDFFMap)
		,mTexAtlasHandles(new tTAFMap)
	{
	}

	cFontLoader::~cFontLoader()
	{
	}



	tFaceHandle cFontLoader::LoadFont(const Twine& filename)
	{
		return InsertFace(
			std::unique_ptr<FreeTypeFace>(new FreeTypeFace(mLibrary, filename)));
	}

	cFontLoader::tDistanceFontHandle 
		cFontLoader::CreateDistanceFieldFont(FreeTypeFace& face, uint32_t tex_size)
	{
		std::unique_ptr<cDistanceFieldFont> font;
		try 
		{
			cCreateBitmapFontTask create_task(face, tex_size);
			do 
			{
				Log::Debug("Font creation progress: %d", 
				static_cast<int>(100.0f*create_task.Progress()));
				create_task.DoWork();
			} while (!create_task.IsDone());

			font = create_task.GetCompletedFont();
		}
		catch(const FreeTypeException& fail)
		{
			Log::Error("Failed Distance font creation with freetype error:"
				"\n\t%s: \"%s\"",fail.what(), 
				FreeTypeLibrary::ErrorString(fail.mFreeTypeError));
			return tDistanceFontHandle();
		}
		return InsertDistanceFont(std::move(font));

	}

void cFontLoader
	::CreateDistanceFieldFontAsync(FreeTypeFace& face, uint32_t tex_size,
										 std::function<void(tDistanceFontHandle)> callback)
	{
		cCreateBitmapFontTask* task;
		std::unique_ptr<cCreateBitmapFontTask> 
			new_task( new cCreateBitmapFontTask( face, tex_size, 8, 3));
		task = new_task.get();
		new_task->SetCallback(
		[this, task, callback](tTaskHandle handle){
			tDistanceFontHandle dff = InsertDistanceFont(task->GetCompletedFont());
			callback(dff);
		});
		mWorkerPool.AddTask(std::move(new_task));
	}


	cDistanceFieldFont& cFontLoader::DistanceFont(tDistanceFontHandle handle)
	{
		AKJ_ASSERT(handle < mDistanceFieldFonts.size());
		return *mDistanceFieldFonts.at(handle);
	}

	FreeTypeFace& cFontLoader::DeSerializeFace(cStringRef data)
	{
		tFaceHandle handle = InsertFace( FreeTypeFace::FromData(mLibrary, data));
		return *mFaces[handle];
	}

	void cFontLoader::
		ExportDFFToFile(tDistanceFontHandle font, const Twine& in_path /*= ""*/)
	{
		auto& dff = DistanceFont(font);
		size_t face_size = cSerialization::SerializedSize(dff.Face());
		cDFFFile pair(dff, face_size);
		AlignedBuffer<16> buffer;
		cSerialization::SerializeLZMA(pair,buffer);
		cStringRef file_data(buffer.ptr<char>(), buffer.size());

		try{
			if (sys::path::has_filename(in_path))
			{
				WriteFileToDisk(file_data, in_path);
			}
			else
			{
				cSmallVector<char, 256> path_buf;
				cStringRef temp_path = in_path.toNullTerminatedStringRef(path_buf);
				sys::path::append(path_buf, in_path, dff.FontName() + ".dff");
				cStringRef full_path(path_buf.data(), path_buf.size());
				WriteFileToDisk(file_data, full_path);
			}
		}
		catch (const Exception& e)
		{
			Log::Warn("Swallowed exception while trying to write dff file: %s",
								e.what());
		}
	}

	cFontLoader::tDistanceFontHandle 
	 cFontLoader::LoadDFFFromMemory(cStringRef data)
	{
		AlignedBuffer<16> buffer;
		auto reader = cSerialization::ReaderCompressed(data, buffer);
		size_t face_length;
		reader.ReadSize(face_length);
	
		std::unique_ptr<cDistanceFieldFont> font;
		
		tFaceHandle face_handle = 
			InsertFace(FreeTypeFace::FromReader(mLibrary, reader));
		
		FreeTypeFace& face = TypeFace(face_handle);
		 
		tDistanceFontHandle df_handle = 
		 InsertDistanceFont(cDistanceFieldFont::FromReader(*mFaces.back(), reader));

		Log::Info("successfully loaded font: %s ",
							mDistanceFieldFonts.back()->FontName());

		return df_handle;
	}

	akj::cFontLoader::tDistanceFontHandle cFontLoader::InsertDistanceFont
			(std::unique_ptr<cDistanceFieldFont>&& font)
	{
		return InsertThing(
			std::move(font), mDistanceFieldFonts, *mDistanceFaceHandles);
	}

	tFaceHandle cFontLoader::InsertFace(std::unique_ptr<FreeTypeFace>&& face)
	{
		return InsertThing(std::move(face), mFaces, *mFaceHandles);
	}

	tTAFHandle cFontLoader::InsertTAF(std::unique_ptr<cTexAtlasFont>&& face)
	{
		return InsertThing(std::move(face), mTexAtlasFonts, *mTexAtlasHandles);
	}

	FreeTypeFace& cFontLoader::TypeFace(tFaceHandle handle)
	{
		if(!handle.IsValid())
		{
			AKJ_THROW("Tried to retrieve invalid face handle");
		}
		return *mFaces[handle];
	}

	akj::tTAFHandle cFontLoader::CreateTexAtlasFont(
		tFaceHandle handle, uint32_t initial_size)
	{
		if(!handle.IsValid()) return tTAFHandle();
		FreeTypeFace& face = TypeFace(handle);
		cCreateTexAtlasTask create_task(face, initial_size);
		
		while(!create_task.IsDone()) create_task.DoWork();

		std::unique_ptr<cTexAtlasFont> taf(
			new cTexAtlasFont(face,
												std::unique_ptr<FreeTypeLibrary>(new FreeTypeLibrary),
												std::move(create_task.GetFinishedAtlas())));
		
		cArray<16, uint32_t> sizes = {14, 12, 21};
		for(uint32_t i: sizes)
		{
			if(i == initial_size) continue;
			cCreateTexAtlasTask create_task2(face, i);
			while(!create_task2.IsDone()) create_task2.DoWork();
			taf->AddAtlas(create_task2.GetFinishedAtlas());
		}
		taf->RepackAtlasTexture();

		return InsertTAF(std::move(taf));
	}



	akj::tFaceHandle cFontLoader::FaceHandleFrom(tDFFHandle df_handle)
	{
		if(!df_handle.IsValid()) return tFaceHandle();
		const cDistanceFieldFont& dff = DistanceFont(df_handle);
		tFaceHandle handle = FindHandleIn(dff, *mFaceHandles);
		return handle;
	}

	akj::tTAFHandle cFontLoader::TexAtlasHandleFrom(tDFFHandle df_handle)
	{
		tFaceHandle face_handle = FaceHandleFrom(df_handle);
		return TexAtlasHandleFrom(face_handle);
	}

	akj::tTAFHandle cFontLoader::TexAtlasHandleFrom(tFaceHandle face_handle)
	{
		if(!face_handle.IsValid()) return tTAFHandle();

		FreeTypeFace& face = TypeFace(face_handle);

		tTAFHandle atlas_handle = FindHandleIn(face, *mTexAtlasHandles);
		//create it if needed
		if(!atlas_handle.IsValid())
		{
			//TODO: make this asynchronous
			atlas_handle = CreateTexAtlasFont(face_handle, kDefaultPixSize);
		}

		return atlas_handle;
	}

	cTexAtlasFont& cFontLoader::TexAtlasFont(tTAFHandle handle)
	{
		AKJ_ASSERT_AND_THROW(handle.IsValid() && handle < mTexAtlasFonts.size());
		return *mTexAtlasFonts.at(handle);
	}

	akj::tDFFHandle cFontLoader::DistanceHandleFrom(tFaceHandle face_handle)
	{
		if(!face_handle.IsValid()) return tDFFHandle();

		FreeTypeFace& face = TypeFace(face_handle);

		tDFFHandle distance_handle = FindHandleIn(face, *mDistanceFaceHandles);

		if(!distance_handle.IsValid())
		{
			//TODO: make this asynchronous
			//distance_handle = CreateDistanceFieldFont(face, kDefaultTextureSize);
		}
		return distance_handle;
	}


} // namespace akj