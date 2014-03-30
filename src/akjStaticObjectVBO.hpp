#pragma once
#include "akjHWGraphicsObject.hpp"
#include <vector>
#include "akjVertexArray.hpp"
#include "akjArrayBuffer.hpp"

namespace akj
{
	class cStaticObjectVBO : public cHWGraphicsObject
	{
	public:
		cStaticObjectVBO(cHWGraphicsContext* context, const Twine& object_name,
											std::vector<const cVertexArray*> object_list );
		~cStaticObjectVBO();
		void Bind();
		int FindObjectIndex(const char* name);
		const cArrayBuffer* GetArrayBuffer() const {
			return &mVertexBuffer;
		} 
		std::vector<const cVertexArray*>::const_iterator beginVertArrays() const{
			return mObjectList.begin();
		}
		std::vector<int>::const_iterator beginVBOIndices() const{
			return mIndexList.begin();
		}
		std::vector<const cVertexArray*>::const_iterator endVertArrays() const{
			return mObjectList.end();
		}
		std::vector<int>::const_iterator endVBOIndices() const{
			return mIndexList.end();
		}
	private:
		std::vector<const cVertexArray*> mObjectList;
		std::vector<int> mIndexList;
		cArrayBuffer mVertexBuffer;
	};
}
