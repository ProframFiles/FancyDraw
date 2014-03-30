#pragma once
#include "Twine.hpp"
#include "SmallString.hpp"
#include "Bitmap.hpp"
#include "akjTask.hpp"

namespace akj
{
class cSaveImageTask : public cTask
{
public:
	cSaveImageTask(const cBitmap<4> bitmap, const Twine& filename)
		:cTask("Saving "+filename)
		,mBitmap(bitmap)
		,mPercentDone(0)
	{
		filename.toVector(mFilename);
		mBitmap.UseAsStorage(mStorage);
	}
	~cSaveImageTask(){};

	const cBitmap<4>& Bitmap() const {return mBitmap;} 
	virtual void DoWork(uint32_t thread_number = 0)
	{
		mBitmap.ExportPNG(cStringRef(mFilename));
		mStorage.Deallocate();
		mPercentDone = 100;
	};

	virtual float Progress() const {return mPercentDone*0.01f;};

	virtual cStringRef StatusString() const {return mFilename;};

	virtual bool IsDone() const {return mPercentDone == 100;}
private:
	cAlignedBuffer mStorage;
	cBitmap<4> mBitmap;
	SmallString<256> mFilename;
	int mPercentDone;
};


}
