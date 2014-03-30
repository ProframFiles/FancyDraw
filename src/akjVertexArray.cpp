#include "akjVertexArray.hpp"
#include "akjOGL.hpp"
#include "FancyDrawMath.hpp"

void akj::cVertexArray::SetGLAttributes() const
{
	const int byte_stride = FloatsPerVert()*sizeof(float);
	//texCoord
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, kTexCoordSize, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(0) );
	//normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, kNormalSize, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(kTexCoordSize) );
	//vertex
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, kVertSize, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(kTexCoordSize+kNormalSize));

	//binormal
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, kBiNormalSize, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(kTexCoordSize+kNormalSize+kVertSize));
}

void akj::cVertexArray::GetTriangles( std::vector<cTriangle>& tris )
{
	
	tris.reserve(tris.size() + NumVerts()/3);
	int vertex_stride = FloatsPerVert();
	int offset = kNormalSize+kTexCoordSize;
	cCoord3 tri_verts[3];
	cCoord3 vertex_normal(0.0f);
	int vertex_index =0;
	for (size_t i = offset; i < mVertStorage.size(); i+=vertex_stride)
	{
		tri_verts[vertex_index].x = mVertStorage.at(i);
		tri_verts[vertex_index].y = mVertStorage.at(i+1);
		tri_verts[vertex_index].z = mVertStorage.at(i+2);
		vertex_normal.x += mVertStorage.at(i-3);
		vertex_normal.y += mVertStorage.at(i-2);
		vertex_normal.z += mVertStorage.at(i-1);
		vertex_index++;
		if(vertex_index == 3){
			cCoord3 tri_normal = (tri_verts[1]-tri_verts[0]).cross(tri_verts[2]-tri_verts[0]);
			tri_normal *=vertex_normal.dot(tri_normal);
			if(tri_normal.length()> 0.01){
				tris.emplace_back(tri_verts[0], tri_verts[1], tri_verts[2], tri_normal);
			}
			vertex_index =0;
			vertex_normal = cCoord3::Zeros();
		}
	}
}

void akj::cVertexArray::GenerateBiTangents()
{
	int vertex_stride = FloatsPerVert();
	int offset = kNormalSize+kTexCoordSize;
	cCoord3 v[3];
	cCoord3 vn[3];
	cCoord3 tc[3];
	int tri_index =0;
	int vertex_index=0;
	

	for (size_t i = offset; i < mVertStorage.size(); i+=vertex_stride)
	{
		tc[tri_index].x = mVertStorage.at(i-5);
		tc[tri_index].y = mVertStorage.at(i-4);
		vn[tri_index].x = mVertStorage.at(i-3);
		vn[tri_index].y = mVertStorage.at(i-2);
		vn[tri_index].z = mVertStorage.at(i-1);
		v[tri_index].x = mVertStorage.at(i);
		v[tri_index].y = mVertStorage.at(i+1);
		v[tri_index].z = mVertStorage.at(i+2);
		tri_index++;
		if(tri_index == 3){
			
			
			for (int j = 0; j < 3; j++)
			{
				const int ii = j;
				const int ione = ii==0? 1 : (ii==1 ? 2 : 0);
				const int itwo = ii==0? 2 : (ii==1 ? 0 : 1);
				const cCoord4 bt = GenerateBiTangent(v[ione]-v[ii],v[itwo]-v[ii],cCoord3(vn[j].x,vn[j].y,vn[j].z), tc[ione]-tc[ii], tc[itwo]-tc[ii] );
				SetBiTangent(bt, vertex_index);
				vertex_index++;
			}
			tri_index =0;
		}
	}
}

void akj::cVertexArray::MakeAndSetBiTangent( int vert_index, const cCoord3& normal )
{
	int ii = vert_index%3;
	int tri_start = vert_index - ii;
	const int izero = ii+tri_start;
	const int ione = tri_start+ (ii==0? 1 : (ii==1 ? 2 : 0));
	const int itwo = tri_start+ (ii==0? 2 : (ii==1 ? 0 : 1));

	const cCoord3 my_vert=GetVertex(izero);
	const cCoord3 my_tc=GetTexCoord(izero);
	const cCoord3 e1 = GetVertex(ione)-my_vert;
	const cCoord3 e2 = GetVertex(itwo)-my_vert;
	const cCoord3 st1 = GetTexCoord(ione)-my_tc;
	const cCoord3 st2 = GetTexCoord(itwo)-my_tc;
	

	const cCoord4 bt = GenerateBiTangent(e1,e2,normal, st1, st2 );
	SetBiTangent(bt, vert_index);


}

