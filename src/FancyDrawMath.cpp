#include "FancyDrawMath.hpp"
#include <Eigen/Core>
#include <Eigen/LU>

namespace akj{

void InverseTranspose4f(float* ptr)
{
	Eigen::Map<Eigen::Matrix4f> mat(ptr);
	mat = mat.inverse().eval();
	mat.transposeInPlace();
}

cCoord4 GenerateBiTangent( const cCoord3& edge1,const cCoord3& edge2, const cCoord3& normal, const cCoord3& st1, const cCoord3& st2 )
{
	const float st_scale = (st1.x*st2.y-st1.y*st2.x);
	const float inv_scale = st_scale == 0.0f ? 1.0f : 1.0f/st_scale;
	cCoord3 u;
	cCoord3 v;
	float el1 = edge1.length();
	float el2 = edge2.length();
	AKJ_ASSERT(el1>0.0f && el2>0.0f);

	const cCoord3 tangent = inv_scale*(st2.y*edge1 - st1.y*edge2);
	const cCoord3 bitangent = inv_scale*(st1.x*edge2 - st2.x*edge1);
	const cCoord3 tn = (tangent - normal.dot(tangent)*normal).normalized();
	const cCoord3 btn = (bitangent - normal.dot(bitangent)*normal-tn.dot(bitangent)*tn).normalized();
	const float determinant = tn.x*btn.y*normal.z + tn.y*btn.z*normal.x + tn.z*btn.x*normal.y
		-(tn.z*btn.y*normal.x + tn.y*btn.x*normal.z + tn.x*btn.z*normal.y);

	return cCoord4(tn, st_scale);
}

void GenerateNGon( int faces, std::vector<cCoord3>& vertices )
{
	//assert(faces > 2 && faces < 254);
	vertices.clear();
	vertices.reserve(faces);
	//want: the closest vertex to be as far as possible
	//find the closest vertex, then find the next closest vertex
	// rotate half way between them
	const double center_arc = 1.0/faces;
	const float cos_factor = static_cast<float>(cos(center_arc * 2.0 * AKJ_PIf));
	const float rotate_arc = static_cast<float>(1.0/(2.0*LeastCommonMultiple(4,faces)));
	const cAngleAxis rotation(rotate_arc*2.0f*AKJ_PIf, 0.0f, 0.0f, 1.0f);
	const float length_factor = 1.0f/cosf(rotate_arc * 2.0f * AKJ_PIf);
	const float sin_factor = static_cast<float>(sin(center_arc * 2.0 * AKJ_PIf));
	cCoord3 out_vector(1.0f, 0.0f, 0.0f);

	out_vector = rotation.rotate(out_vector);

	vertices.push_back(length_factor*out_vector);
	for (int i = 2 ; i <= faces; ++i)
	{
		const cCoord3 perp_vec(-out_vector.y, out_vector.x, 0.0f);
		out_vector = cos_factor*out_vector;
		out_vector = out_vector + sin_factor*perp_vec;
		vertices.push_back(length_factor*out_vector);
	}
}

cCoord3 cTriangle::ClosestPoint( const cCoord3& point ) const
{
	cCoord3 r0 = mVertices[0] - point;
	cCoord3 r1 = mVertices[1] - mVertices[0];
	cCoord3 r2 = mVertices[2] - mVertices[0];
	const float len0sqr = r1.lengthSquared();
	const float r1dotr2 = r1.dot(r2);
	const float len1sqr = r2.lengthSquared();
	const float p0 = r0.dot(r1);
	const float p1 = r0.dot(r2);
	const float c = r0.lengthSquared();
	const float det = std::abs(len0sqr * len1sqr - r1dotr2 * r1dotr2);
	float s = r1dotr2 * p1 - len1sqr * p0;
	float t = r1dotr2 * p0 - len0sqr * p1;

	if(s + t <= det) {
		if(s < 0.0f) {
			if(t < 0.0f) { // region 4
				if(p0 < 0.0f) {
					t = 0.0f;
					if(-p0 >= len0sqr) {
						s = 1.0f;
					} else {
						s = -p0 / len0sqr;
					}
				} else {
					s = 0.0f;
					if(p1 >= 0.0f) {
						t = 0.0f;
					} else if(-p1 >= len1sqr) {
						t = 1.0f;
					} else {
						t = -p1 / len1sqr;
					}
				}
			} else { // region 3
				s = 0.0f;
				if(p1 >= 0.0f) {
					t = 0.0f;
				} else if(-p1 >= len1sqr) {
					t = 1.0f;
				} else {
					t = -p1 / len1sqr;
				}
			}
		} else if(t < 0.0f) { // region 5
			t = 0.0f;
			if(p0 >= 0.0f) {
				s = 0.0f;
			} else if(-p0 >= len0sqr) {
				s = 1.0f;
			} else {
				s = -p0 / len0sqr;
			}
		} else { // region 0
			// minimum at interior point
			const float invDet = (1.0f) / det;
			s *= invDet;
			t *= invDet;
		}
	} else {

		if(s < 0.0f) { // region 2
			const float tmp0 = r1dotr2 + p0;
			const float tmp1 = len1sqr + p1;
			if(tmp1 > tmp0) {
				const float numer = tmp1 - tmp0;
				const float denom = len0sqr - (2.0f) * r1dotr2 + len1sqr;
				if(numer >= denom) {
					s = 1.0f;
					t = 0.0f;
				} else {
					s = numer / denom;
					t = 1.0f - s;
				}
			} else {
				s = 0.0f;
				if(tmp1 <= 0.0f) {
					t = 1.0f;
				} else if(p1 >= 0.0f) {
					t = 0.0f;
				} else {
					t = -p1 / len1sqr;
				}
			}
		} 
		else if(t < 0.0f){ // region 6
			const float tmp0 = r1dotr2 + p1;
			const float tmp1 = len0sqr + p0;
			if(tmp1 > tmp0) {
				const float numer = tmp1 - tmp0;
				const float denom = len0sqr - (2.0f) * r1dotr2 + len1sqr;
				if(numer >= denom) {
					t = 1.0f;
					s = 0.0f;

				} else {
					t = numer / denom;
					s = 1.0f - t;
				}
			} else {
				t = 0.0f;
				if(tmp1 <= 0.0f) {
					s = 1.0f;
				} else if(p0 >= 0.0f) {
					s = 0.0f;
				} else {
					s = -p0 / len0sqr;
				}
			}
		} 
		else{ // region 1
			const float numer = len1sqr + p1 - r1dotr2 - p0;
			if(numer <= 0.0f) {
				s = 0.0f;
				t = 1.0f;
			} else {
				const float denom = len0sqr - (2.0f) * r1dotr2 + len1sqr;
				if(numer >= denom) {
					s = 1.0f;
					t = 0.0f;
				} else {
					s = numer / denom;
					t = 1.0f - s;
				}
			}
		}

	}
	return mVertices[0] + s * r1 + t * r2;
}

bool cAABB::RayIntersection( cRay& result, const cCoord3 origin, const cCoord3 direction )
{
	if(CheckWithin(origin)){
		cCoord3 bounding_planes = mUpperCorner;
		if(direction.x < 0.0f){
			bounding_planes.x = mLowerCorner.x;
		}
		if(direction.y < 0.0f){
			bounding_planes.y = mLowerCorner.y;
		}
		if(direction.z < 0.0f){
			bounding_planes.z = mLowerCorner.z;
		}

		const cCoord3 max_ray = origin - bounding_planes;
		float ray_d = std::numeric_limits<float>::max();
		if(direction.x != 0.0 ){
			const float dist = std::abs(max_ray.x/direction.x);
			if( dist < ray_d ){
				ray_d = dist;
				result.mDirection = direction.x<0.0f ? -cCoord3::XAxis() : cCoord3::XAxis();

			}
		}
		if(direction.y != 0.0 ){
			const float dist = std::abs(max_ray.y/direction.y);
			if( dist < ray_d ){
				ray_d = dist;
				result.mDirection = direction.y<0.0f ? -cCoord3::YAxis() : cCoord3::YAxis();
			}
		}
		if(direction.z != 0.0 ){
			const float dist = std::abs(max_ray.z/direction.z);
			if( dist < ray_d ){
				ray_d = dist;
				result.mDirection = direction.z<0.0f ? -cCoord3::ZAxis() : cCoord3::ZAxis();
			}
		}
		AKJ_ASSERT(!(ray_d<0.0f));
		result.mLength = ray_d;
		return true;
	}
	// now the outside case
	const float dot_result = direction.dot(0.5*(mLowerCorner+mUpperCorner)-origin);
	if(dot_result < 0.0f){
		return false;
	}
	float ray_d = 0.0;
	cCoord3 bounding_planes = mLowerCorner;
	if(direction.x!= 0.0f && !CheckWithinX(origin)){
		bounding_planes.x = (direction.x > 0.0f) ? mLowerCorner.x : mUpperCorner.x;
		float dist_x = bounding_planes.x - origin.x;
		const float dist = std::abs(dist_x/direction.x);
		if( dist > ray_d ){
			ray_d = dist;
			result.mDirection = direction.x>0.0f ? -cCoord3::XAxis() : cCoord3::XAxis();
		}
	}
	if(direction.y!= 0.0f && !CheckWithinY(origin)){
		bounding_planes.y = (direction.y > 0.0f) ? mLowerCorner.y : mUpperCorner.y;
		float dist_y = bounding_planes.y - origin.y;
		const float dist = std::abs(dist_y/direction.y);
		if( dist > ray_d ){
			ray_d = dist;
			result.mDirection = direction.y>0.0f ? -cCoord3::YAxis() : cCoord3::YAxis();
		}
	}
	if(direction.z!= 0.0f && !CheckWithinZ(origin)){
		bounding_planes.z = (direction.z > 0.0f) ? mLowerCorner.z : mUpperCorner.z;
		float dist_z = bounding_planes.z - origin.z;
		const float dist = std::abs(dist_z/direction.z);
		if( dist > ray_d ){
			ray_d = dist;
			result.mDirection = direction.z>0.0f ? -cCoord3::ZAxis() : cCoord3::ZAxis();
		}
	}
	const cCoord3 max_ray = bounding_planes-origin;
	AKJ_ASSERT(!(ray_d<0.0f));
	//TODO: better way to do this than the fudge
	if(CheckWithin(direction*(ray_d+0.0001f)+origin)){
		result.mLength = ray_d;
		return true;
	}
	return false;
}
}
