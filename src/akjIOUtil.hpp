#pragma once
#include <string>
#include "FancyDrawMath.hpp"
#include "StringRef.hpp"
#include "akjLog.hpp"
#include "akjAlignedBuffer.hpp"
#include "Bitmap.hpp"
#include <memory>

namespace akj{

class Twine;
class FileOutputBuffer;

void InterpToArray(float* out, int num_elements, float start_val, float end_val);
void GerpToArray(float* out, int num_elements, float start_val, float end_val, float factor);

void WriteFileToDisk(cStringRef outgoing_data, const Twine& filepath);

std::unique_ptr<FileOutputBuffer> GetMappedOutFile(size_t size, 
																							const Twine& path);

void WriteCompressedFile(cStringRef outgoing_data, const Twine& filepath);
std::string FileToString(cStringRef filename);

cAlignedBitmap LoadImageFile(const Twine& file_name, cAlignedBuffer& buf);
cAlignedBitmap LoadImageFromMemory(cStringRef raw_image_data, 
																		cAlignedBuffer& buf);

}
