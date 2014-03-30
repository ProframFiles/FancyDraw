#pragma once
#include "OwningPtr.hpp"
#include "akjFreeTypeLibrary.hpp"
#include "SystemError.hpp"
#include <vector>
#include <memory>
#include "akjUintHandle.hpp"
#include <map>
#include <unordered_map>
#include "Twine.hpp"
#include <functional>
#include "akjUintHandle.hpp"
#include "akjExceptional.hpp"


namespace akj
{
	class Twine;

	class FreeTypeFace;
	class cDistanceFieldFont;
	class cTexAtlasFont;
	class cWorkerPool;
}

namespace akj{
	typedef cHandle<cDistanceFieldFont> tDFFHandle;
	typedef cHandle<cTexAtlasFont> tTAFHandle;
	typedef cHandle<FreeTypeFace> tFaceHandle;

	class cFontLoader
	{
	public:

		// default size for a pixmap font
		enum { 
			kDefaultPixSize = 14,
			kDefaultTextureSize = 1024
		};

		typedef tDFFHandle tDistanceFontHandle;
		

		cFontLoader(cWorkerPool& worker_pool);
		~cFontLoader();
		tFaceHandle LoadFont(const Twine& filename);
		tFaceHandle FaceHandleFrom(tDFFHandle df_handle);
		tTAFHandle TexAtlasHandleFrom(tDFFHandle df_handle);
		tTAFHandle TexAtlasHandleFrom(tFaceHandle face_handle);
		tDFFHandle DistanceHandleFrom(tFaceHandle face_handle);


		tDistanceFontHandle CreateDistanceFieldFont(
			FreeTypeFace& face, uint32_t tex_size);
		tDistanceFontHandle LoadDFFFromMemory(cStringRef data);

		tTAFHandle CreateTexAtlasFont(tFaceHandle face, uint32_t initial_size);


		void CreateDistanceFieldFontAsync(
			FreeTypeFace& face, uint32_t tex_size,
			std::function<void(tDistanceFontHandle)> callback);
		void CreateTexAtlasFontAsync(
			FreeTypeFace& face, uint32_t tex_size,
			std::function<void(tTAFHandle)> callback);


		//////////////////////////////////////////////////////////////////////////
		// Once you have a handle, the actual classes can be accessed here
		//////////////////////////////////////////////////////////////////////////
		FreeTypeFace& TypeFace(tFaceHandle handle);
		cDistanceFieldFont& DistanceFont(tDistanceFontHandle handle);
		cTexAtlasFont& TexAtlasFont(tTAFHandle handle);
		
		

		void ExportDFFToFile(tDistanceFontHandle font, const Twine& path = "");
		


		

	private:
		template <typename tFont>
	static cHandle<tFont> InsertThing(
		std::unique_ptr<tFont>&& font,
		std::vector<std::unique_ptr<tFont>>& container,
		std::unordered_map<const FreeTypeFace*, cHandle<tFont>>& table)
	{
		if (font)
		{
			//uint32_t hash = font->HashVal();
			auto found = table.find(&font->Face());
			if (found == table.end())
			{
				const cHandle<tFont> handle(static_cast<uint32_t>(container.size()));
				container.emplace_back(std::move(font));
				table[&container.back()->Face()] = handle;
				return handle;
			}
			else
			{
				return found->second;
			}
		}
		AKJ_THROW("Failed to deserialize something!");
	}

	template <typename tFont, typename tHandle>
	static cHandle<tHandle> FindHandleIn(const tFont& font,
		std::unordered_map<const FreeTypeFace*, cHandle<tHandle>>& table)
	{
		//uint32_t hash = font->HashVal();
		auto found = table.find(&font.Face());
		if (found == table.end())
		{
			return cHandle<tHandle>();
		}
		else
		{
			return found->second;
		}
	}



		FreeTypeFace& DeSerializeFace(cStringRef data);
		tFaceHandle InsertFace(std::unique_ptr<FreeTypeFace>&& face);
		tDistanceFontHandle 
			InsertDistanceFont( std::unique_ptr<cDistanceFieldFont>&& font);
		tTAFHandle InsertTAF(std::unique_ptr<cTexAtlasFont>&& font);
		FreeTypeLibrary mLibrary;

		cWorkerPool& mWorkerPool;

		std::vector<std::unique_ptr<FreeTypeFace> > mFaces;
		std::vector<std::unique_ptr<cDistanceFieldFont> > mDistanceFieldFonts;
		std::vector<std::unique_ptr<cTexAtlasFont> > mTexAtlasFonts;

		typedef std::unordered_map
			<const FreeTypeFace*, tDistanceFontHandle> tDFFMap;
		typedef std::unordered_map
			<const FreeTypeFace*, tFaceHandle> tFaceMap;
		typedef std::unordered_map
			<const FreeTypeFace*, tTAFHandle> tTAFMap;
		std::unique_ptr< tDFFMap > mDistanceFaceHandles;
		std::unique_ptr< tFaceMap > mFaceHandles;
		std::unique_ptr< tTAFMap > mTexAtlasHandles;
	};



}
