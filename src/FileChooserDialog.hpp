#pragma once
#include <memory>
#include <vector>

class Fl_Native_File_Chooser;

namespace akj{
	
	struct cFileSelection{
		std::vector<size_t> mPathIndices;
		std::vector<char> mRawPaths;
	};
	
	class cFileChooserDialog
	{
	public:
		// Filter format:
		//"*" --all files
		//"*.txt" --all files ending in.txt
		//"*.{cxx,cpp}" --all files ending in either.cxx or.cpp
		//"*.[CH]" --all files ending in.C or.H
		cFileChooserDialog(const char* title, const char* starting_dir, const char* filter = "*");
		virtual ~cFileChooserDialog();
		std::unique_ptr<cFileSelection> ShowAndBlock();
	private:
		std::unique_ptr<Fl_Native_File_Chooser> mChooserImpl;
	};
}
