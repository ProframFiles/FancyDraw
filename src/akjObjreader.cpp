#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdexcept>
#include "akjObjreader.hpp"
#include "akjVertexArray.hpp"

namespace akj
{
cObjReader::cObjReader(cStringRef file_data)
	:mFaceGroupIndex(0)
{
	//Initialize Data
	mMaxX = mMaxZ = mMaxY = -2e16f;
	mMinY = mMinX = mMinZ = 2e16f;
	loadObj(file_data);
}

cObjReader::~cObjReader()
{
	
}

void cObjReader::getBoundingVolume(float *minX, float *maxX,
                                  float *minY, float *maxY,
                                  float *minZ, float *maxZ)
{
	*minX = mMinX;
	*minY = mMinY;
	*minZ = mMinZ;
	*maxX = mMaxX;
	*maxY = mMaxY;
	*maxZ = mMaxZ;
}

void cObjReader::loadObj(cStringRef file_buffer)
{
	std::vector<char> face_group_string(1024);
	char* group_name = &face_group_string[0];

	// Preprocess the Text File
	// Get the size number of vertices
	// Get the number of faces
	int vertex_count, normal_count, texcoord_count, current_face_count;
	vertex_count = normal_count = texcoord_count = current_face_count = 0;
	mFaceGroups.reserve(16);
	const char* ptr = file_buffer.data();
	const char* const end = ptr + file_buffer.size();
	while(ptr != end)
	{
		if(ptr[0] == '\n' && ptr+2 < end)
		{
			ptr++;
			if( ptr[0] == 'v' && ptr[1] == ' ' )
			{
				vertex_count++;
			}
			else if( ptr[0] == 'v' && ptr[1] == 'n' )
			{
				normal_count++;
			}
			else if( ptr[0] == 'v' && ptr[1] == 't' )
			{
				texcoord_count++;
			}
			else if( ptr[0] == 'g' && ptr[1] == ' ' )
			{
				if(current_face_count > 0){
					mFaceGroups.emplace_back(group_name, current_face_count);
					current_face_count = 0;
				}
				sscanf(&ptr[2], "%s", group_name);

			}
			else if( ptr[0] == 'f' && ptr[1] == ' ' )
			{
				current_face_count++;
			}
		}
		else ++ptr;
	}
	if(current_face_count > 0){
		mFaceGroups.emplace_back(group_name, current_face_count);
		current_face_count = 0;
	}
	mVertexList.reserve(vertex_count);
	mNormalList.reserve(normal_count);
	mTexcoordList.reserve(texcoord_count);
	int current_face_group = -1;
	// Reset the filestream
	ptr = file_buffer.data();
	// Read in the data
	while( ptr != end )
	{
		if (ptr[0] == '\n' && ptr + 2 < end)
		{
			ptr++;
			if (ptr[0] == 'v' && ptr[1] == ' ')
			{
				cCoord3 new_vertex;
				sscanf(&ptr[2], "%f%f%f", &new_vertex.x, &new_vertex.y, &new_vertex.z);
				if (mVertexList.empty())
				{
					mMaxX = new_vertex.x;
					mMaxY = new_vertex.y;
					mMaxZ = new_vertex.z;
					mMinX = new_vertex.x;
					mMinY = new_vertex.y;
					mMinZ = new_vertex.z;
				}
				else
				{
					if (new_vertex.x > mMaxX) mMaxX = new_vertex.x;
					if (new_vertex.y > mMaxY) mMaxY = new_vertex.y;
					if (new_vertex.z > mMaxZ) mMaxZ = new_vertex.z;
					if (new_vertex.x < mMinX) mMinX = new_vertex.x;
					if (new_vertex.y < mMinY) mMinY = new_vertex.y;
					if (new_vertex.z < mMinZ) mMinZ = new_vertex.z;
				}
				mVertexList.push_back(new_vertex);
			}
			else if (ptr[0] == 'v' && ptr[1] == 'n')
			{
				cCoord3 new_normal;
				sscanf(&ptr[3], "%f%f%f", &new_normal.x, &new_normal.y, &new_normal.z);
				mNormalList.push_back(new_normal.normalized());
			}
			else if (ptr[0] == 'v' && ptr[1] == 't')
			{
				cCoord3 new_texcoord;
				sscanf(&ptr[3], "%f%f", &new_texcoord.x, &new_texcoord.y);

				mTexcoordList.push_back(new_texcoord);
			}
			else if (ptr[0] == 'g' && ptr[1] == ' ')
			{
				current_face_group++;
			}
			else if (ptr[0] == 'f' && ptr[1] == ' ')
			{
				int vnum, nnum, tnum;
				const char *tempptr;
				tempptr = &ptr[2];
				int face_vertex = 0;
				mFaceGroups.at(current_face_group).mFaceList.emplace_back();
				face& new_face = mFaceGroups.at(current_face_group).mFaceList.back();
				const int current_verts = static_cast<int>(mVertexList.size());
				while (tempptr && (3 == sscanf(tempptr, "%i/%i/%i", &vnum, &tnum, &nnum)))
				{
					if (vnum < 0) vnum = current_verts + vnum + 1;
					if (nnum < 0) nnum = current_verts + nnum + 1;
					if (tnum < 0) tnum = current_verts + tnum + 1;
					vnum -= 1;
					nnum -= 1;
					tnum -= 1;
					new_face.v[face_vertex] = vnum;
					new_face.n[face_vertex] = nnum;
					new_face.t[face_vertex] = tnum;
					face_vertex++;
					tempptr = strchr(tempptr, ' ');
					if (tempptr)
					{
						tempptr += 1;
					}
				}
			}
		}
		else ++ptr;
	}
	if( mVertexList.size() != vertex_count || mNormalList.size() != normal_count 
		|| mTexcoordList.size() != texcoord_count)
	{
		throw std::runtime_error("ObjReader Error: object counts are wrong");
	}
	mMaxX += mMaxX * (mMaxX > 0 ? .1f : -0.1f);
	mMaxY += mMaxY * (mMaxY > 0 ? .1f : -0.1f);
	mMaxZ += mMaxZ * (mMaxZ > 0 ? .1f : -0.1f);
	mMinX -= mMinX * (mMinX > 0 ? .1f : -0.1f);
	mMinY -= mMinY * (mMinY > 0 ? .1f : -0.1f);
	mMinZ -= mMinZ * (mMinZ > 0 ? .1f : -0.1f);

};

 cVertexArray* cObjReader::GetNextVertexArray(const char* prefix )
{
	if(mFaceGroupIndex >= static_cast<int>(mFaceGroups.size())){
		mFaceGroupIndex = 0;
		return NULL;
	}
	const auto face_group = mFaceGroups.begin() + mFaceGroupIndex;
	const auto& face_list = face_group->mFaceList;
	cVertexArray* ptr = new cVertexArray(prefix + face_group->mGroupName, face_list.size()*3);
	for( size_t i = 0; i < face_list.size(); i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			const cCoord3 n = mNormalList[face_list.at(i).n[j]];
			const cCoord3 rot_n(n.x,n.y, n.z);
			ptr->PlaceVertex(i*3+j	,  mVertexList[face_list.at(i).v[j]]
									,  rot_n
									,mTexcoordList[face_list.at(i).t[j]], face_list.at(i).v[j] );
		} // END: for j
	} // END: for i
	ptr->GenerateBiTangents();

	mFaceGroupIndex++;
	return ptr;
}


void cObjReader::centerObject()
{
	float cx = mMinX + (mMaxX - mMinX) / 2.0f;
	float cy = mMinY + (mMaxY - mMinY) / 2.0f;
	float cz = mMinZ + (mMaxZ - mMinZ) / 2.0f;

	for(size_t i = 0; i < mVertexList.size(); i++ )
	{
		mVertexList[i].x -= cx;
		mVertexList[i].y -= cy;
		mVertexList[i].z -= cz;
	} // END for i
	mMinX -= cx;
	mMaxX -= cx;
	mMinY -= cy;
	mMaxY -= cy;
	mMinZ -= cz;
	mMaxZ -= cz;
}

void cObjReader::resizeObject()
{
	float s = 0.0;

	if (-mMinX > s)
	{
		s = -mMinX;
	}
	if (-mMinY > s)
	{
		s = -mMinY;
	}
	if (-mMinZ > s)
	{
		s = -mMinZ;
	}
	if (mMaxX > s)
	{
		s = mMaxX;
	}
	if (mMaxY > s)
	{
		s = mMaxY;
	}
	if (mMaxZ > s)
	{
		s = mMaxZ;
	}

	s = 1.0f / s;

	for (size_t i = 0; i < mVertexList.size(); i++)
	{
		mVertexList[i].x *= s;
		mVertexList[i].y *= s;
		mVertexList[i].z *= s;
	}
	mMinX *= s;
	mMaxX *= s;
	mMinY *= s;
	mMaxY *= s;
	mMinZ *= s;
	mMaxZ *= s;
}
}
