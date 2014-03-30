#include "FileChooserDialog.hpp"
#include "akjLog.hpp"
#include "FL/Fl_Native_File_Chooser.H"


namespace akj{
	typedef Fl_Native_File_Chooser tChooserImpl;


	cFileChooserDialog::cFileChooserDialog(const char* title, const char* starting_dir, const char* filter /*= "*"*/)
	{
		mChooserImpl.reset(new tChooserImpl);
		mChooserImpl->title(title);
		mChooserImpl->directory(starting_dir);
		mChooserImpl->filter(filter);
		mChooserImpl->type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
	}

	cFileChooserDialog::~cFileChooserDialog()
	{

	}

	std::unique_ptr<cFileSelection> cFileChooserDialog::ShowAndBlock()
	{
		int chooser_result = mChooserImpl->show();
		

		std::unique_ptr<cFileSelection> selection(new cFileSelection);
		if (chooser_result == 1)
		{
			Log::Info("File selection canceled");
		}
		else if (chooser_result < 0)
		{
			Log::Info("Error in file selection dialog: %s", mChooserImpl->errmsg());
		}
		else
		{
			std::vector<char>& path_data = selection->mRawPaths;

			for (int i = 0; i < mChooserImpl->count(); ++i)
			{
				const char* values = mChooserImpl->filename(i);
				selection->mPathIndices.push_back(path_data.size());
				while (char single_char = *values++)
				{
					path_data.push_back(single_char);
				}
				path_data.push_back('\0');
			}
		}
		
		return selection;
	}



} // namespace akj

/*
see http://extinguishedscholar.com/soft/Fl_Native_File_Chooser.html
Fl_Native_File_Chooser fnfc;
fnfc.title("Pick a file");
fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
fnfc.filter("Text\t*.txt\n"
"C Files\t*.{cxx,h,c}");
fnfc.directory("E:\\");           // default directory to use
*/