#include "akjStaticObjectVBO.hpp"
#include "akjOGL.hpp"

namespace akj
{

	cStaticObjectVBO::cStaticObjectVBO(cHWGraphicsContext* context, const Twine& object_name,
		std::vector<const cVertexArray*> object_list)
	:cHWGraphicsObject(context, object_name)
	,mObjectList(object_list)
	,mVertexBuffer(context, "object buffer: " + object_name, 0, STATIC_VBO)
{
	mIndexList.reserve(object_list.size());
	uint32_t total_triangles = 0;
	uint32_t total_floats = 0;
	for (const cVertexArray* vertex_array: object_list)
	{
		mIndexList.push_back(static_cast<int>(total_triangles));
		total_triangles += vertex_array->NumVerts()/3;
		total_floats += vertex_array->NumVerts()*vertex_array->FloatsPerVert();
	}
	std::vector<float> temp_vector(total_floats);
	uint32_t current_index = 0;
	auto temp_iter = temp_vector.begin();
	for (const cVertexArray* vertex_array: object_list)
	{
		uint32_t num_floats 
			= vertex_array->NumVerts()*vertex_array->FloatsPerVert();

		for (auto ifloat = vertex_array->begin();
				 ifloat != vertex_array->end();
				  ++ifloat)
		{
			*temp_iter = *ifloat;
			++temp_iter;
		}
	}
	mVertexBuffer.InitBuffer(total_floats*sizeof(float), &temp_vector.at(0));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

cStaticObjectVBO::~cStaticObjectVBO()
{

}

void cStaticObjectVBO::Bind()
{
	mVertexBuffer.Bind();
	
}

int cStaticObjectVBO::FindObjectIndex( const char* name )
{
	//TODO: this
	auto index_iter = mIndexList.begin();
	for(const cVertexArray* mesh: mObjectList){
		if(strcmp(mesh->GetName(),name) == 0){
			return *index_iter;
		}
		index_iter++;
	}
	return -1;
}


} // namespace akj