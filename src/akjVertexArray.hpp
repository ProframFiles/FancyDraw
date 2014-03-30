#pragma once
#include <vector>
#include "FancyDrawMath.hpp"
#include "akjLog.hpp"
#include <unordered_map>
#include <memory>

namespace akj {
	class cVertexArray
	{
	public:
		
		cVertexArray(std::string name, size_t num_verts)
			: mName(name)
			, kVertSize(3)
			, kNormalSize(3)
			, kBiNormalSize(4)
			, kTexCoordSize(2)
		{
			Log::Debug("constructing vertex array %s", mName.c_str());
			mVertStorage.resize(num_verts*FloatsPerVert(), 0.0f);
			//mLinkStructure.reserve(num_verts/2);
		}
		cVertexArray(std::string name, size_t num_verts, int num_position, int num_tex, int num_normal)
			: mName(name)
			, kVertSize(num_position)
			, kNormalSize(num_normal)
			, kBiNormalSize(4)
			, kTexCoordSize(num_tex)
		{
			Log::Debug("constructing vertex array %s", mName.c_str());
			AKJ_WARN_IF( FloatsPerVert()%4 != 0);
			mVertStorage.resize(num_verts*FloatsPerVert(), 0.0f);
		}
		~cVertexArray(){
			Log::Debug("destroying vertex array %s", mName.c_str());
		}
		void GetTriangles(std::vector<cTriangle>& tris);
		void SetGLAttributes() const;
		void PlaceVertex(size_t index, const cCoord3& vert, const cCoord3& normal, const cCoord3& tex_coord, int vertex_id)
		{
			//we're going to overwrite the last texture coordinate here. THis is on purpose
			int vert_number = static_cast<int>(index);
			int tri_vertex = vert_number%3;

			index *= FloatsPerVert();
			tex_coord.CopyToArray(&mVertStorage.at(index));
			normal.CopyToArray(&mVertStorage.at(index + kTexCoordSize));
			vert.CopyToArray(&mVertStorage.at(index+ kTexCoordSize + kNormalSize));
			/*
			auto iter = mLinkStructure.find(vertex_id);
			if (iter != mLinkStructure.end()){
				iter->second.AddPartner(vert_number);
			}
			else{
				iter = mLinkStructure.insert(std::make_pair(vertex_id, cLinkedVertex(vert_number, *this))).first;
			}
			*/
		}
		void MakeAndSetBiTangent(int i, const cCoord3& normal);
		void GenerateBiTangents();
		const std::vector<float>::const_iterator begin() const{
			return mVertStorage.begin();
		}
		const std::vector<float>::const_iterator end() const{
			return mVertStorage.end();
		}
		void SetBiTangent(const cCoord4& bt, int index){
			const int float_index = index*FloatsPerVert() + kTexCoordSize + kNormalSize + kVertSize;
			bt.CopyToArray(&mVertStorage.at(float_index));
		}

		const float* Data() const 
		{
			return mVertStorage.data();
		}
		int GetNormalIndex(int i){
			return i*FloatsPerVert()+kTexCoordSize;
		}
		int GetVertexIndex(int i){
			return i*FloatsPerVert()+kTexCoordSize+kNormalSize;
		}
		cCoord3 GetNormal(int i){
			const int ni = GetNormalIndex(i);
			return cCoord3(mVertStorage.at(ni), mVertStorage.at(ni+1), mVertStorage.at(ni+2));
		}
		cCoord3 GetVertex(int i){
			const int ni = GetVertexIndex(i);
			return cCoord3(mVertStorage.at(ni), mVertStorage.at(ni+1), mVertStorage.at(ni+2));
		}
		cCoord3 GetTexCoord(int i){
			const int ni = i;
			return cCoord3(mVertStorage.at(ni), mVertStorage.at(ni+1), 0.0f);
		}
		const int FloatsPerVert() const
		{
			return kVertSize + kNormalSize + kTexCoordSize + kBiNormalSize;
		}
		const int NumVerts() const
		{
			return static_cast<int>(mVertStorage.size()/FloatsPerVert());
		}
		const char* GetName() const {
			return mName.c_str();
		}
		struct cLinkedVertex{
			cLinkedVertex(int index, cVertexArray& parent)
				:mIndex(index)
				,mNumPartners(0)
				,mParent(parent)
			{
				mNumPartners = 0;
			}
			cLinkedVertex(cLinkedVertex&& other)
				:mIndex(other.mIndex)
				,mNumPartners(other.mNumPartners)
				,mParent(other.mParent)
			{
				for (int i = 0; i < 4; i++)
				{
					mPartners[i] = other.mPartners[i];
				}
				mExtraPartners.swap(other.mExtraPartners);
			}
			~cLinkedVertex(){};
			void MakeBiTangent(){
				const cCoord3 my_normal = GetMeanNormal();
				mParent.MakeAndSetBiTangent(mIndex, my_normal);
				for (int i = 0; i < mNumPartners; i++)
				{
					if(i < 4){
						mParent.MakeAndSetBiTangent(mPartners[i], my_normal);
					}
					else{
						mParent.MakeAndSetBiTangent(mExtraPartners->at(i-4), my_normal);
					}
				}
			}
			cCoord3 GetMeanNormal(){
				cCoord3 n = mParent.GetNormal(mIndex);
				for (int i = 0; i < mNumPartners; i++)
				{
					if(i < 4){
						n += mParent.GetNormal(mPartners[i]);
					}
					else{
						n += mParent.GetNormal(mExtraPartners->at(i-4));
					}
					float f = n.length();
				}
				return n/static_cast<float>(mNumPartners+1);
			}
			void AddPartner(int index){
				if(mNumPartners < 4){
					mPartners[mNumPartners] = index;
				}
				else{
					if(mNumPartners == 4){
						mExtraPartners.reset(new std::vector<int>());
					}
					mExtraPartners->push_back(index);
				}
				mNumPartners++;
			}
			std::unique_ptr<std::vector<int>> mExtraPartners;
			int mIndex;
			int mEdgeMates[2];
			int mPartners[4];
			int mNumPartners;
			cVertexArray& mParent;
		};
	private:
		const int kVertSize;
		const int kNormalSize;
		const int kBiNormalSize;
		const int kTexCoordSize;
		std::string mName;
		std::vector<float> mVertStorage;
		std::unordered_map<int, cLinkedVertex> mLinkStructure;
	};
}

