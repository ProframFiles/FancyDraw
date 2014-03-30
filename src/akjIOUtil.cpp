#include "akjIOUtil.hpp"
#include <stdio.h>
#include "stb_image.h"
#include "StringRef.hpp"
#include "OwningPtr.hpp"
#include "MemoryBuffer.hpp"
#include "FileOutputBuffer.hpp"
#include "Twine.hpp"
#include "Path.hpp"
#include <stdlib.h>

namespace akj{

std::string FileToString( cStringRef filename )
{
	AKJ_ASSERT(!filename.empty());
	OwningPtr<MemoryBuffer> mapped_file;
	MemoryBuffer::getFile(filename, mapped_file).assertOK();

	return mapped_file->getBuffer().str();
}

void WriteCompressedFile(cStringRef outgoing_data, const Twine& filepath)
{
	OwningPtr<MemoryBuffer> packed;
	lz4::compress(outgoing_data, packed);

	const size_t packed_size = packed->getBufferSize();

	OwningPtr<FileOutputBuffer> out_buffer;
	FileOutputBuffer::create(filepath.str(), packed_size, out_buffer).assertOK();
	memcpy(out_buffer->getBufferStart(), packed->getBufferStart(), packed_size);
	out_buffer->commit().assertOK();
}

std::unique_ptr<FileOutputBuffer> GetMappedOutFile(size_t size, 
																							const Twine& path)
{
	cSmallVector<char, 256> error_buf;
	cSmallVector<char, 256> temp;
	if(sys::path::is_relative(path))
	{
		sys::fs::current_path(temp).assertOK();
	}
	sys::path::append(temp, path);
	
	cStringRef out_name(temp.data(), temp.size());
	cStringRef parent = sys::path::parent_path(out_name);

	auto err = sys::fs::create_directories(parent);

	if(err)
	{
		cStringRef e = (Twine("Error writing file:") + (err ?err.message() :"" ) +
			"\"" + out_name + "\"").toNullTerminatedStringRef(error_buf);
		Log::Error(e);
		throw std::runtime_error(e.data());
	}
	
	OwningPtr<FileOutputBuffer> fout;
	err = FileOutputBuffer::create(out_name, size, fout);
	
	if(err)
	{
		AKJ_THROW(Twine("Error while creating file \"") + out_name +
			"\": " + err.message());
	}

	return std::move(std::unique_ptr<FileOutputBuffer>(fout.take()));

}

void WriteFileToDisk(cStringRef outgoing_data, const Twine& filepath)
{
	std::unique_ptr<FileOutputBuffer> fout 
		= GetMappedOutFile(outgoing_data.size(), filepath);

	memcpy(fout->getBufferStart(), outgoing_data.data(), outgoing_data.size());

	fout->commit().assertOK();
}

void InterpToArray(float* out, int num_elements, float start_val, float end_val)
{

	for (int i = 0; i < num_elements; ++i)
	{
		const float interp = i*(1.0f/(num_elements-1));
		out[i] = start_val*(1.0f - interp)+ interp*end_val;
	}
}
void GerpToArray(float* out, int num_elements, float start_val, float end_val, float factor)
{
	float weight = 1.0f;
	double sum = 0.0;
	for (int i = 0; i < num_elements; ++i)
	{
		sum += weight;
		weight = weight*factor;
	}
	const float scale = 1.0f/static_cast<float>(sum);
	weight = 1.0f;
	float interp = 0.0f;
	for (int i = 0; i < num_elements; ++i)
	{
		out[i] = start_val*(1.0f - interp)+ interp*end_val;
		interp += weight*scale;
		weight = weight*factor;
	}
}

cAlignedBitmap 
LoadImageFromMemory(cStringRef raw_image_data, cAlignedBuffer& buf)
{
	int w, h, comp;
	auto* ptr = 
	stbi_load_from_memory((const uchar*)raw_image_data.data(),
									(int)raw_image_data.size(), &w, &h, &comp, 0);
	if(!ptr) return cAlignedBitmap();

	cAlignedBitmap ret(buf, w, h, comp, BIT_DEPTH_8);

	auto* row_ptr = ptr;
	for (int row = 0; row < h ; ++row)
	{
		memcpy(ret.RowData(row),row_ptr,w*comp);
		row_ptr += (w*comp);
	}

	stbi_image_free(ptr);

	return ret;
}

cAlignedBitmap 
LoadImageFile(const Twine& file_name, cAlignedBuffer& buf)
{
	OwningPtr<MemoryBuffer> mapped_file;
	MemoryBuffer::getFile(file_name, mapped_file).assertOK();
	if(!mapped_file){
		return cAlignedBitmap();
	}
	return LoadImageFromMemory(mapped_file->getBuffer(), buf);
}




}
