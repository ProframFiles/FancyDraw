#ifndef OBJREADER
#define OBJREADER
#include <memory>
#include "FancyDrawMath.hpp"
#include "StringRef.hpp"

namespace akj
{
class cVertexArray;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   class objReader                               %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

class cObjReader
{
public:
	cObjReader(cStringRef file_data);
	~cObjReader();

	void loadObj(cStringRef file_data);

	void getBoundingVolume(float *minX, float *maxX,
	                       float *minY, float *maxY,
	                       float *minZ, float *maxZ);

	int getVertexCount();
	int getTriangleCount();

	void centerObject();
	void resizeObject();
	akj::cVertexArray* GetNextVertexArray(const char* prefix = "" );
private:

	struct face
	{
		int v[3]; // vertices
		int n[3]; // normals
		int t[3]; // texture coords
	};
	struct cFaceGroup
	{
		cFaceGroup(){}
		~cFaceGroup(){}
		cFaceGroup(const char* name, int reserved_size)
			: mGroupName(name)
		{
			mFaceList.reserve(reserved_size);
		}
		std::string mGroupName;
		std::vector<face> mFaceList;
	};
	std::vector<cCoord3> mVertexList;         // loaded vertex data
	std::vector<cCoord3> mNormalList;         // loaded normal data
	std::vector<cCoord3> mTexcoordList;       // loaded texture coordinate data
	std::vector<cFaceGroup> mFaceGroups;
	int mFaceGroupIndex;

	float     mMaxX;             // maximum and mininum x, y and z values
	float     mMinX;             // for loaded object
	float     mMaxY;             //
	float     mMinY;             //
	float     mMaxZ;             //
	float     mMinZ;             //

};
}
#endif
